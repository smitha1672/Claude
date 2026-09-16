from app import mcp
from core.runner import BenchctlError, conn_args, run_benchctl
from tools.common import CONFIRM_MESSAGE, err, fmt


@mcp.tool()
def psu_info(host: str = "", port: int = 0, bench_config: str = "") -> str:
    """Show GW Instek GPP-4323 PSU identity. Read-only."""
    try:
        result = run_benchctl(["psu", "info", *conn_args(host, port, bench_config)])
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_measure(channel: int, host: str = "", port: int = 0, bench_config: str = "") -> str:
    """One-shot voltage/current/power readback for a PSU channel. Read-only.

    Args:
        channel: PSU output channel, 1-4.
    """
    try:
        result = run_benchctl(
            ["psu", "measure", "--channel", str(channel), *conn_args(host, port, bench_config)]
        )
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_status(channel: int, host: str = "", port: int = 0, bench_config: str = "") -> str:
    """Output/CV-CC/OVP/OCP status for a PSU channel — hardware-safety check. Read-only.

    Args:
        channel: PSU output channel, 1-4.
    """
    try:
        result = run_benchctl(
            ["psu", "status", "--channel", str(channel), *conn_args(host, port, bench_config)]
        )
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_set(
    channel: int,
    voltage: float | None = None,
    current: float | None = None,
    confirm: bool = False,
    host: str = "",
    port: int = 0,
    bench_config: str = "",
) -> str:
    """Set a PSU channel's voltage and/or current limit. State-changing — requires confirm=True.

    benchctl validates values against the channel's hardware limits before sending
    (LimitError on out-of-range) — this tool does not duplicate that check.

    Args:
        channel: PSU output channel, 1-4.
        voltage: Target voltage limit in volts (optional).
        current: Target current limit in amps (optional).
        confirm: Must be True — set only after the user has explicitly confirmed.
    """
    if not confirm:
        return CONFIRM_MESSAGE
    args = ["psu", "set", "--channel", str(channel)]
    if voltage is not None:
        args += ["--voltage", str(voltage)]
    if current is not None:
        args += ["--current", str(current)]
    args += conn_args(host, port, bench_config)
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_power(
    state: str = "",
    channel: int = 0,
    all: str = "",
    confirm: bool = False,
    host: str = "",
    port: int = 0,
    bench_config: str = "",
) -> str:
    """Get or set PSU channel output state. Setting state is destructive-adjacent (drives real
    hardware) and requires confirm=True; querying (state and all both omitted) does not.

    Args:
        state: 'on' or 'off' to set a single channel's output; omit to query current state.
        channel: PSU output channel, 1-4 — required when setting a single channel.
        all: 'on' or 'off' to set every channel at once, instead of channel/state.
        confirm: Must be True to actually change power state.
    """
    setting = bool(state) or bool(all)
    if setting and not confirm:
        return CONFIRM_MESSAGE
    args = ["psu", "power"]
    if state:
        args.append(state)
    if channel:
        args += ["--channel", str(channel)]
    if all:
        args += ["--all", all]
    args += conn_args(host, port, bench_config)
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_protect(
    channel: int,
    ovp: float | None = None,
    ocp: float | None = None,
    ovp_enable: bool | None = None,
    ocp_enable: bool | None = None,
    confirm: bool = False,
    host: str = "",
    port: int = 0,
    bench_config: str = "",
) -> str:
    """Get or set OVP/OCP trip level and enable state for a PSU channel. Setting any value
    is state-changing and requires confirm=True; querying (all optional args omitted) does not.

    Args:
        channel: PSU output channel, 1-4.
        ovp: OVP trip voltage (optional).
        ocp: OCP trip current (optional).
        ovp_enable: Enable/disable OVP (optional).
        ocp_enable: Enable/disable OCP (optional).
        confirm: Must be True to actually change protection settings.
    """
    setting = any(v is not None for v in (ovp, ocp, ovp_enable, ocp_enable))
    if setting and not confirm:
        return CONFIRM_MESSAGE
    args = ["psu", "protect", "--channel", str(channel)]
    if ovp is not None:
        args += ["--ovp", str(ovp)]
    if ocp is not None:
        args += ["--ocp", str(ocp)]
    if ovp_enable is not None:
        args.append("--ovp-enable" if ovp_enable else "--no-ovp-enable")
    if ocp_enable is not None:
        args.append("--ocp-enable" if ocp_enable else "--no-ocp-enable")
    args += conn_args(host, port, bench_config)
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_profile_list(profiles_file: str = "") -> str:
    """List available named per-channel PSU config profiles. Read-only."""
    args = ["psu", "profile", "list"]
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_profile_show(name: str, profiles_file: str = "") -> str:
    """Show a named PSU profile's per-channel settings. Read-only.

    Args:
        name: Profile name, as shown by psu_profile_list.
    """
    args = ["psu", "profile", "show", name]
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def psu_profile_apply(
    name: str,
    channel: int = 0,
    dry_run: bool = False,
    confirm: bool = False,
    profiles_file: str = "",
    host: str = "",
    port: int = 0,
    bench_config: str = "",
) -> str:
    """Push a named profile's setpoints to the PSU. State-changing unless dry_run=True —
    requires confirm=True otherwise.

    Args:
        name: Profile name, as shown by psu_profile_list.
        channel: Restrict to a single channel, 1-4 (optional; profile default otherwise).
        dry_run: Preview the setpoints without sending them — does not require confirm.
        confirm: Must be True to actually apply the profile (ignored if dry_run=True).
    """
    if not dry_run and not confirm:
        return CONFIRM_MESSAGE
    args = ["psu", "profile", "apply", name]
    if channel:
        args += ["--channel", str(channel)]
    if dry_run:
        args.append("--dry-run")
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    args += conn_args(host, port, bench_config)
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)
