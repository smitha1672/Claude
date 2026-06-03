import fitz  # pymupdf
from config import CHUNK_SIZE, CHUNK_OVERLAP


def parse_pdf(path: str) -> tuple[list[dict], int]:
    """Return (chunks, page_count). Each chunk: {text, page, chunk_index}."""
    doc = fitz.open(path)
    page_count = len(doc)

    # Build full text with page boundary tracking
    segments: list[tuple[int, int, int]] = []  # (char_start, char_end, page_num)
    full_text = ""
    for page_num, page in enumerate(doc, start=1):
        start = len(full_text)
        full_text += page.get_text() + "\n"
        segments.append((start, len(full_text), page_num))
    doc.close()

    def page_for_offset(offset: int) -> int:
        for seg_start, seg_end, pnum in segments:
            if seg_start <= offset < seg_end:
                return pnum
        return page_count

    chunks = []
    pos = 0
    idx = 0
    while pos < len(full_text):
        text = full_text[pos : pos + CHUNK_SIZE]
        chunks.append({"text": text, "page": page_for_offset(pos), "chunk_index": idx})
        pos += CHUNK_SIZE - CHUNK_OVERLAP
        idx += 1

    return chunks, page_count


def parse_text(text: str) -> list[dict]:
    """Chunk a raw text string the same way as a PDF."""
    chunks = []
    pos = 0
    idx = 0
    while pos < len(text):
        chunk = text[pos : pos + CHUNK_SIZE]
        chunks.append({"text": chunk, "page": None, "chunk_index": idx})
        pos += CHUNK_SIZE - CHUNK_OVERLAP
        idx += 1
    return chunks
