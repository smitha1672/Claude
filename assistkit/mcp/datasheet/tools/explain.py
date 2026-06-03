from app import mcp


@mcp.tool()
def ds_explain(part: str, topic: str) -> str:
    """Explain a spec, timing diagram, or section in plain English.

    Args:
        part:  Part name as shown in ds_list.
        topic: What to explain, e.g. 'startup timing', 'power modes'.
    """
    return "ds_explain: not yet implemented."


@mcp.tool()
def ds_pins(part: str, pin: str = "") -> str:
    """Look up pin functions and recommended connections.

    Args:
        part: Part name as shown in ds_list.
        pin:  Specific pin name or number (optional — omit to list all).
    """
    return "ds_pins: not yet implemented."


@mcp.tool()
def ds_register(part: str, register: str, value: str = "") -> str:
    """Decode a register value using the datasheet's register map.

    Args:
        part:     Part name as shown in ds_list.
        register: Register name or address, e.g. 'PMCTL' or '0x400AF000'.
        value:    Hex value to decode (optional).
    """
    return "ds_register: not yet implemented."
