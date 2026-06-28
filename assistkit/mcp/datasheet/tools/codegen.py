import re
from pathlib import Path

from app import mcp
from core.embedder import embed
from core.store import query_chunks, log_query, get_part

_STOP_WORDS = {
    "a", "an", "the", "to", "for", "of", "in", "with", "and", "or",
    "how", "write", "generate", "create", "make", "using", "use",
    "function", "init", "initialize", "code", "driver", "example",
}
_MAX_FILE_SIZE = 50_000
_MAX_FILES = 5
_MAX_FILE_CONTENT = 8_000
_SKIP_DIRS = {"build", "obj", "Debug", "Release", "third-party", "thirdparty", ".git", "doc", "docs"}


def _keywords(text: str) -> list[str]:
    words = re.findall(r"[a-zA-Z0-9_]+", text.lower())
    return [w for w in words if w not in _STOP_WORDS and len(w) > 2]


def _part_family_variants(part: str) -> list[str]:
    """Return progressively shorter prefix variants of a part number for path matching."""
    p = part.lower()
    variants = [p]
    # e.g. mspm0c1105 → mspm0c110, mspm0c11, mspm0c, mspm0
    for length in (len(p) - 1, len(p) - 2, len(p) - 3, len(p) - 4):
        if length > 3:
            variants.append(p[:length])
    return variants


def _score_file(path: Path, keywords: list[str], part: str) -> int:
    parts_lower = [p.lower() for p in path.parts]
    name = path.stem.lower()
    path_str = str(path).lower()
    score = 0

    # Keyword match in filename (strong signal)
    score += sum(3 for kw in keywords if kw in name)

    # Part family match anywhere in path
    for variant in _part_family_variants(part):
        if any(variant in p for p in parts_lower):
            score += 4
            break

    # Prefer nortos (bare metal) over rtos
    if "nortos" in parts_lower:
        score += 3
    elif "rtos" in parts_lower:
        score -= 2

    # Prefer driverlib headers/examples over BSL or third-party
    if "driverlib" in parts_lower:
        score += 2
    if "bsl" in path_str:
        score -= 1

    return score


def _scan_sdk(sdk_root: Path, keywords: list[str], part: str) -> list[dict]:
    candidates = []
    for p in sdk_root.rglob("*"):
        if any(d in _SKIP_DIRS for d in p.parts):
            continue
        if p.suffix.lower() not in (".h", ".c"):
            continue
        try:
            if p.stat().st_size > _MAX_FILE_SIZE:
                continue
        except OSError:
            continue
        score = _score_file(p, keywords, part)
        if score > 0:
            candidates.append((score, p))

    candidates.sort(key=lambda x: -x[0])

    results = []
    for _, p in candidates[:_MAX_FILES]:
        try:
            content = p.read_text(encoding="utf-8", errors="ignore")
            results.append({
                "path": str(p.relative_to(sdk_root)),
                "content": content[:_MAX_FILE_CONTENT],
                "truncated": len(content) > _MAX_FILE_CONTENT,
            })
        except OSError:
            pass
    return results


@mcp.tool()
def ds_code(part: str, request: str, sdk_path: str = "") -> str:
    """Generate C driver or init code using datasheet context and SDK sources.

    Args:
        part:     Part name as shown in ds_list.
        request:  What to generate, e.g. 'I2C init function', 'deep sleep entry'.
        sdk_path: Path to the SDK root folder (optional but strongly recommended).
    """
    if not get_part(part):
        return f"Error: part '{part}' not found. Run ds_list to see available parts."

    # Retrieve relevant datasheet sections
    embedding = embed([request])[0]
    hits = query_chunks(embedding, n_results=5, part=part)
    if not hits:
        return f"Error: no indexed content found for '{part}'."

    ds_blocks = []
    for h in hits:
        page = f" (p.{h['page']})" if h.get("page") else ""
        ds_blocks.append(f"--- Datasheet{page} ---\n{h['text'].strip()}")

    # Scan SDK if provided
    sdk_blocks = []
    sdk_note = ""
    if sdk_path:
        root = Path(sdk_path)
        if not root.exists():
            sdk_note = f"Warning: SDK path not found: {sdk_path}"
        else:
            kws = _keywords(request)  # functional keywords only; part handled by family matching
            files = _scan_sdk(root, kws, part)
            if files:
                for f in files:
                    tail = " (truncated)" if f["truncated"] else ""
                    sdk_blocks.append(f"--- SDK: {f['path']}{tail} ---\n{f['content']}")
                sdk_note = f"SDK: {sdk_path} — {len(files)} file(s) found (keywords: {', '.join(kws)})"
            else:
                sdk_note = f"SDK: {sdk_path} — no files matched keywords: {', '.join(kws)}"

    # Assemble context for Claude
    out = [f"Part: {part}", f"Request: {request}", "Language: C"]
    if sdk_note:
        out.append(sdk_note)
    out += ["", "## Datasheet Context"] + ds_blocks
    if sdk_blocks:
        out += ["", "## SDK Context"] + sdk_blocks
    out += ["", "Using the context above, generate the requested C code."]

    result = "\n\n".join(out)
    log_query("ds_code", request, result, part=part)
    return result
