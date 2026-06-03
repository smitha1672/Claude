from app import mcp
from core.embedder import embed
from core.store import get_part, log_query, query_chunks


@mcp.tool()
def ds_search(query: str, part: str = "", n: int = 5) -> str:
    """Semantic search across all indexed datasheets.

    Args:
        query: Natural language query, e.g. 'how to enter deep sleep'.
        part:  Limit search to a specific part (optional).
        n:     Number of results to return (default 5).
    """
    if not query.strip():
        return "Error: query is empty."
    if part and not get_part(part):
        return f"Error: part '{part}' not found. Run ds_list to see available parts."

    embedding = embed([query])[0]
    hits = query_chunks(embedding, n_results=n, part=part or None)

    if not hits:
        return "No results found."

    lines = []
    for i, h in enumerate(hits, 1):
        page_info = f" (p.{h['page']})" if h.get("page") else ""
        score_pct = f"{h['score'] * 100:.0f}%"
        lines.append(f"[{i}] {h['part']}{page_info}  score={score_pct}")
        lines.append(h["text"].strip()[:300])
        lines.append("")

    result = "\n".join(lines)
    log_query("ds_search", query, result, part=part or None)
    return result


@mcp.tool()
def ds_find(part: str, parameter: str) -> str:
    """Extract a specific parameter value from a datasheet (e.g. voltage, address).

    Args:
        part:      Part name as shown in ds_list.
        parameter: Parameter to look up, e.g. 'operating voltage', 'I2C address'.
    """
    return "ds_find: not yet implemented. Use ds_search to locate parameters manually."
