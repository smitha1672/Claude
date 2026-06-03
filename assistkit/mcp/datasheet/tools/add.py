from pathlib import Path

from app import mcp
from core.embedder import embed
from core.parser import parse_pdf, parse_text
from core.store import register_part, upsert_chunks


@mcp.tool()
def ds_add_pdf(path: str, part: str = "") -> str:
    """Parse and index a PDF datasheet into the pool.

    Args:
        path: Absolute path to the PDF file.
        part: Part name/label. Defaults to the PDF filename stem.
    """
    p = Path(path)
    if not p.exists():
        return f"Error: file not found: {path}"
    if not p.suffix.lower() == ".pdf":
        return f"Error: expected a .pdf file, got: {p.suffix}"

    label = part.strip() or p.stem
    chunks, page_count = parse_pdf(str(p))
    texts = [c["text"] for c in chunks]
    embeddings = embed(texts)
    part_id = register_part(label, str(p), page_count, len(chunks))
    upsert_chunks(part_id, label, chunks, embeddings)
    return f"✓ Indexed {label}: {page_count} pages, {len(chunks)} chunks."


@mcp.tool()
def ds_add_text(text: str, part: str) -> str:
    """Add a raw text snippet with a part label.

    Args:
        text: The text content to index.
        part: Part name/label for this snippet.
    """
    if not part.strip():
        return "Error: part label is required."
    if not text.strip():
        return "Error: text is empty."

    chunks = parse_text(text)
    texts = [c["text"] for c in chunks]
    embeddings = embed(texts)
    part_id = register_part(part.strip(), "text", 0, len(chunks))
    upsert_chunks(part_id, part.strip(), chunks, embeddings)
    return f"✓ Indexed {part}: {len(chunks)} chunks."


@mcp.tool()
def ds_add_part(part_number: str) -> str:
    """Fetch a datasheet by part number from the web, then index it.

    Args:
        part_number: The component part number (e.g. MSPM0C1104, BME280).
    """
    import os
    import tempfile
    import httpx

    part = part_number.strip()
    p_lower = part.lower()

    candidates = [
        f"https://www.ti.com/lit/ds/symlink/{p_lower}.pdf",
        f"https://www.ti.com/lit/ds/symlink/{p_lower}a.pdf",
        f"https://www.st.com/resource/en/datasheet/{part}.pdf",
        f"https://www.st.com/resource/en/datasheet/{p_lower}.pdf",
        f"https://www.nxp.com/docs/en/data-sheet/{part}.pdf",
        f"https://www.nxp.com/docs/en/data-sheet/{p_lower}.pdf",
        f"https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-{p_lower}-ds000.pdf",
    ]

    headers = {"User-Agent": "Mozilla/5.0 (compatible; datasheet-fetcher/1.0)"}

    with httpx.Client(timeout=30, follow_redirects=True) as client:
        for url in candidates:
            try:
                head = client.head(url, headers=headers)
                if head.status_code != 200:
                    continue
                content_type = head.headers.get("content-type", "")
                if "pdf" not in content_type and not url.endswith(".pdf"):
                    continue

                resp = client.get(url, headers=headers)
                if resp.status_code != 200:
                    continue

                with tempfile.NamedTemporaryFile(suffix=".pdf", delete=False) as f:
                    f.write(resp.content)
                    tmp_path = f.name

                try:
                    chunks, page_count = parse_pdf(tmp_path)
                    texts = [c["text"] for c in chunks]
                    embeddings = embed(texts)
                    part_id = register_part(part, url, page_count, len(chunks))
                    upsert_chunks(part_id, part, chunks, embeddings)
                    return f"✓ Fetched from {url}\n✓ Indexed {part}: {page_count} pages, {len(chunks)} chunks."
                finally:
                    os.unlink(tmp_path)

            except Exception:
                continue

    tried = "\n".join(f"  {u}" for u in candidates)
    return (
        f"Could not auto-fetch datasheet for '{part}'.\n"
        f"Tried:\n{tried}\n\n"
        f"Use ds_add_pdf with a local file instead."
    )
