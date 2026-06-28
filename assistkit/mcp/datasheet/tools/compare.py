from app import mcp
from core.embedder import embed
from core.store import get_part, log_query, query_chunks


@mcp.tool()
def ds_compare(part_a: str, part_b: str, specs: str = "") -> str:
    """Compare two indexed parts on key specs.

    Args:
        part_a: First part name.
        part_b: Second part name.
        specs:  Comma-separated spec names to focus on (optional).
    """
    missing = [p for p in [part_a, part_b] if not get_part(p)]
    if missing:
        return f"Error: part(s) not found: {', '.join(missing)}. Run ds_list to see available parts."

    query = specs if specs.strip() else "voltage current power consumption temperature package interface flash ram"

    lines = [f"Comparing: {part_a} vs {part_b}"]
    if specs:
        lines.append(f"Focus: {specs}")
    lines.append("")

    for part in [part_a, part_b]:
        embedding = embed([query])[0]
        hits = query_chunks(embedding, n_results=4, part=part)
        lines.append(f"## {part}")
        if not hits:
            lines.append("No indexed content found.")
        else:
            for h in hits:
                page = f" (p.{h['page']})" if h.get("page") else ""
                lines.append(f"--- Datasheet{page} ---")
                lines.append(h["text"].strip()[:300])
                lines.append("")

    lines.append("Using the datasheet sections above, produce a comparison table of the two parts on their key specs.")

    result = "\n".join(lines)
    log_query("ds_compare", f"{part_a} vs {part_b}", result)
    return result
