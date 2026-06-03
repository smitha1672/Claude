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
    return "ds_add_part: not yet implemented. Use ds_add_pdf to index a local PDF."
