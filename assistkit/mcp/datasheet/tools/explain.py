from app import mcp
from core.embedder import embed
from core.store import get_part, log_query, query_chunks


@mcp.tool()
def ds_explain(part: str, topic: str) -> str:
    """Explain a spec, timing diagram, or section in plain English.

    Args:
        part:  Part name as shown in ds_list.
        topic: What to explain, e.g. 'startup timing', 'power modes'.
    """
    if not get_part(part):
        return f"Error: part '{part}' not found. Run ds_list to see available parts."

    embedding = embed([topic])[0]
    hits = query_chunks(embedding, n_results=4, part=part)

    if not hits:
        return f"No content found for '{topic}' in {part}."

    lines = [f"Part: {part}", f"Topic: {topic}", ""]
    for h in hits:
        page = f" (p.{h['page']})" if h.get("page") else ""
        lines.append(f"--- Datasheet{page} ---")
        lines.append(h["text"].strip()[:400])
        lines.append("")
    lines.append("Explain the above in plain English, suitable for a hardware engineer implementing this feature.")

    result = "\n".join(lines)
    log_query("ds_explain", topic, result, part=part)
    return result


@mcp.tool()
def ds_pins(part: str, pin: str = "") -> str:
    """Look up pin functions and recommended connections.

    Args:
        part: Part name as shown in ds_list.
        pin:  Specific pin name or number (optional — omit to list all).
    """
    if not get_part(part):
        return f"Error: part '{part}' not found. Run ds_list to see available parts."

    query = f"pin {pin} function signal description" if pin else "pin functions signal descriptions IO table pinout"
    embedding = embed([query])[0]
    hits = query_chunks(embedding, n_results=5, part=part)

    if not hits:
        return f"No pin information found in {part}."

    lines = [f"Part: {part}", f"Pin: {pin or 'all'}", ""]
    for h in hits:
        page = f" (p.{h['page']})" if h.get("page") else ""
        lines.append(f"--- Datasheet{page} ---")
        lines.append(h["text"].strip()[:400])
        lines.append("")

    target = f"pin {pin}" if pin else "all pins"
    lines.append(f"List the function and recommended connections for {target} from the tables above.")

    result = "\n".join(lines)
    log_query("ds_pins", pin or "all", result, part=part)
    return result


@mcp.tool()
def ds_register(part: str, register: str, value: str = "") -> str:
    """Decode a register value using the datasheet's register map.

    Args:
        part:     Part name as shown in ds_list.
        register: Register name or address, e.g. 'PMCTL' or '0x400AF000'.
        value:    Hex value to decode (optional).
    """
    if not get_part(part):
        return f"Error: part '{part}' not found. Run ds_list to see available parts."

    query = f"register {register} bit field address map description"
    embedding = embed([query])[0]
    hits = query_chunks(embedding, n_results=4, part=part)

    if not hits:
        return f"No register information found for '{register}' in {part}."

    lines = [f"Part: {part}", f"Register: {register}"]
    if value:
        lines.append(f"Value to decode: {value}")
    lines.append("")
    for h in hits:
        page = f" (p.{h['page']})" if h.get("page") else ""
        lines.append(f"--- Datasheet{page} ---")
        lines.append(h["text"].strip()[:400])
        lines.append("")

    if value:
        lines.append(f"Using the bit field definitions above, decode the value {value} for register {register}.")
    else:
        lines.append(f"Describe the bit fields and their meanings for register {register}.")

    result = "\n".join(lines)
    log_query("ds_register", f"{register} {value}".strip(), result, part=part)
    return result
