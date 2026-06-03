from app import mcp


@mcp.tool()
def ds_compare(part_a: str, part_b: str, specs: str = "") -> str:
    """Compare two indexed parts on key specs.

    Args:
        part_a: First part name.
        part_b: Second part name.
        specs:  Comma-separated spec names to compare (optional — compares all).
    """
    return "ds_compare: not yet implemented."
