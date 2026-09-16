from app import mcp
from core.runner import BenchctlError, conn_args, run_benchctl
from tools.common import CONFIRM_MESSAGE, err, fmt


@mcp.tool()
def sequence_run(
    path: str,
    confirm: bool = False,
    host: str = "",
    port: int = 0,
    bench_config: str = "",
) -> str:
    """Run a JSON step-list file against the PSU. State-changing — requires confirm=True.

    A good fit for a single voice command that triggers a pre-defined multi-step sequence
    (e.g. "run the step-up sequence"). See docs/CLI.md in the benchctl repo for the file format.

    Args:
        path: Path to the sequence JSON file.
        confirm: Must be True — set only after the user has explicitly confirmed.
    """
    if not confirm:
        return CONFIRM_MESSAGE
    args = ["sequence", "run", path, *conn_args(host, port, bench_config)]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)
