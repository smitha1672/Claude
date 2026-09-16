import json

from core.runner import BenchctlError

CONFIRM_MESSAGE = (
    "Confirmation required: this changes hardware state. Re-call with confirm=True "
    "only after the user has explicitly confirmed the action."
)


def fmt(result) -> str:
    return json.dumps(result, indent=2)


def err(exc: BenchctlError) -> str:
    return f"Error: {exc}"
