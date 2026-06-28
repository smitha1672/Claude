import re

from app import mcp
from config import POOL_DIR
from core.store import clear_all_parts, clear_history, delete_part, get_history, get_last_result, list_parts


@mcp.tool()
def ds_list() -> str:
    """Show all indexed datasheets: part number, source, date added, page count."""
    rows = list_parts()
    if not rows:
        return "Pool is empty. Use ds_add_pdf or ds_add_text to add a datasheet."

    header = f"{'Part':<20} {'Pages':>5}  {'Chunks':>6}  {'Added':<20}  Source"
    sep = "─" * 80
    lines = [header, sep]
    for r in rows:
        added = r["date_added"][:16].replace("T", " ")
        source = r["source"] or ""
        if len(source) > 40:
            source = "..." + source[-37:]
        lines.append(f"{r['part']:<20} {r['page_count'] or 0:>5}  {r['chunk_count'] or 0:>6}  {added:<20}  {source}")
    return "\n".join(lines)


@mcp.tool()
def ds_remove(part: str) -> str:
    """Remove a single datasheet from the pool.

    Args:
        part: The part name as shown in ds_list.
    """
    if delete_part(part):
        return f"✓ Removed {part} from the pool."
    return f"Error: part '{part}' not found. Run ds_list to see available parts."


@mcp.tool()
def ds_clear() -> str:
    """Wipe the entire datasheet pool."""
    count = clear_all_parts()
    return f"✓ Pool cleared. Removed {count} part(s)."


@mcp.tool()
def ds_export(target: str = "last") -> str:
    """Save a query result to a .md file in pool/exports/.

    Args:
        target: 'last' to export the most recent result (default).
    """
    row = get_last_result()
    if not row:
        return "No results in query log to export."

    exports_dir = POOL_DIR / "exports"
    exports_dir.mkdir(parents=True, exist_ok=True)

    date_str = row["timestamp"][:10]
    slug = re.sub(r"[^a-z0-9]+", "-", row["query"].lower())[:40].strip("-")
    filename = f"{date_str}-{slug}.md"
    out_path = exports_dir / filename

    content = f"""# Export: {row['tool']} — {row['part'] or 'N/A'}
Date: {row['timestamp'][:16].replace('T', ' ')}
Query: {row['query']}

## Result
{row['result']}
"""
    out_path.write_text(content, encoding="utf-8")
    return f"✓ Saved to {out_path}"


@mcp.tool()
def ds_history(part: str = "", tool: str = "", limit: int = 50) -> str:
    """Show past queries — filter by part, tool, or date.

    Args:
        part:  Filter by part name (optional).
        tool:  Filter by tool name, e.g. ds_code (optional).
        limit: Maximum number of rows to show (default 50).
    """
    rows = get_history(part=part or None, tool=tool or None, limit=limit)
    if not rows:
        return "No query history found."

    header = f"{'Timestamp':<20}  {'Tool':<12}  {'Part':<20}  Query"
    sep = "─" * 80
    lines = [header, sep]
    for r in rows:
        ts = r["timestamp"][:16].replace("T", " ")
        q = r["query"][:40]
        lines.append(f"{ts:<20}  {r['tool']:<12}  {(r['part'] or ''):<20}  {q}")
    return "\n".join(lines)


@mcp.tool()
def ds_history_clear() -> str:
    """Wipe the query log."""
    count = clear_history()
    return f"✓ Cleared {count} query log entries."
