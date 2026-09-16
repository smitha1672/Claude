import json
import subprocess

from config import BENCHCTL_BIN, BENCHCTL_CWD


class BenchctlError(Exception):
    """Raised when a `benchctl` invocation exits non-zero."""

    def __init__(self, message: str, returncode: int):
        super().__init__(message)
        self.returncode = returncode


def run_benchctl(args: list[str]):
    """Shell out to `benchctl <args> --json` and return the parsed JSON payload.

    benchctl itself owns all validation and hardware-safety gating (LimitError,
    --force checks, etc.) — this just invokes it and surfaces stderr on failure.
    """
    proc = subprocess.run(
        [BENCHCTL_BIN, *args, "--json"],
        capture_output=True,
        text=True,
        cwd=BENCHCTL_CWD,
    )
    if proc.returncode != 0:
        message = proc.stderr.strip() or f"benchctl exited with code {proc.returncode}"
        raise BenchctlError(message, proc.returncode)

    stdout = proc.stdout.strip()
    return json.loads(stdout) if stdout else None


def conn_args(host: str = "", port: int = 0, bench_config: str = "") -> list[str]:
    """Shared --host/--port/--bench-config flags for PSU-connected commands.

    Omit these entirely to fall back to bench.toml / BENCHCTL_CONFIG env / built-in
    defaults, which benchctl already resolves on its own.
    """
    args = []
    if host:
        args += ["--host", host]
    if port:
        args += ["--port", str(port)]
    if bench_config:
        args += ["--bench-config", bench_config]
    return args
