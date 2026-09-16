from app import mcp
from core.runner import BenchctlError, run_benchctl
from tools.common import CONFIRM_MESSAGE, err, fmt


@mcp.tool()
def js220_list() -> str:
    """Enumerate connected JS220 devices. Read-only."""
    try:
        result = run_benchctl(["js220", "list"])
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_info(device: str = "") -> str:
    """Show JS220 identity and current settings. Read-only.

    Args:
        device: Device path/serial, as shown by js220_list (optional; default device otherwise).
    """
    args = ["js220", "info"]
    if device:
        args += ["--device", device]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_measure(device: str = "", duration_s: float = 0, raw: bool = False) -> str:
    """Collect averaged voltage/current/power stats from a JS220. Read-only.

    Args:
        device: Device path/serial, as shown by js220_list (optional).
        duration_s: Collection window in seconds (optional; benchctl default otherwise).
        raw: Include raw sample data alongside the averaged stats.
    """
    args = ["js220", "measure"]
    if device:
        args += ["--device", device]
    if duration_s:
        args += ["--duration-s", str(duration_s)]
    if raw:
        args.append("--raw")
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_power(state: str = "", device: str = "", force: bool = False, confirm: bool = False) -> str:
    """Get or set JS220 target power (mirrors the desktop app's power button). Setting state
    is state-changing and requires confirm=True; querying (state omitted) does not.

    Turning power off disconnects the DUT — benchctl itself gates that behind --force, and
    this tool requires confirm=True whenever force=True so a misheard command can't silently
    disconnect the DUT.

    Args:
        state: 'on' or 'off' to set target power; omit to query current state.
        device: Device path/serial, as shown by js220_list (optional).
        force: Pass --force through to benchctl to allow a DUT-disconnecting 'off'. Never
            default this to True — only set it when the user explicitly asked to disconnect.
        confirm: Must be True to actually change power state (also required whenever force=True).
    """
    setting = bool(state)
    if setting and not confirm:
        return CONFIRM_MESSAGE
    if force and not confirm:
        return CONFIRM_MESSAGE
    args = ["js220", "power"]
    if state:
        args.append(state)
    if device:
        args += ["--device", device]
    if force:
        args.append("--force")
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_capture(
    device: str = "",
    out: str = "",
    duration_s: float = 0,
    signals: str = "",
    signal_freq: str = "",
    config_file: str = "",
    note: str = "",
    power_on_delay_s: float | None = None,
    power_off_delay_s: float | None = None,
    max_duration_s: float = 0,
    force: bool = False,
    confirm: bool = False,
) -> str:
    """Record a raw .jls capture from the JS220. State-changing (writes a file, and may toggle
    target power via the delay args) — requires confirm=True.

    Args:
        device: Device path/serial, as shown by js220_list (optional).
        out: Output .jls path (default: jls/<timestamp>.jls).
        duration_s: Capture duration in seconds (default: 30.0).
        signals: Comma-separated signal list, e.g. 'i,v' (optional).
        signal_freq: Sample rate to set before capturing, e.g. '100k' (optional).
        config_file: Config history file used to default signal_freq (optional).
        note: Free-text note to attach to the capture (optional).
        power_on_delay_s: Baseline delay with power off before turning on (-1 = leave untouched).
        power_off_delay_s: Delay after turning power back off before closing the file
            (-1 = leave untouched).
        max_duration_s: Safety cap on capture duration (optional; benchctl default otherwise).
        force: Bypass max_duration_s.
        confirm: Must be True — set only after the user has explicitly confirmed.
    """
    if not confirm:
        return CONFIRM_MESSAGE
    args = ["js220", "capture"]
    if device:
        args += ["--device", device]
    if out:
        args += ["--out", out]
    if duration_s:
        args += ["--duration-s", str(duration_s)]
    if signals:
        args += ["--signals", signals]
    if signal_freq:
        args += ["--signal-freq", signal_freq]
    if config_file:
        args += ["--config-file", config_file]
    if note:
        args += ["--note", note]
    if power_on_delay_s is not None:
        args += ["--power-on-delay-s", str(power_on_delay_s)]
    if power_off_delay_s is not None:
        args += ["--power-off-delay-s", str(power_off_delay_s)]
    if max_duration_s:
        args += ["--max-duration-s", str(max_duration_s)]
    args.append("--force" if force else "--no-force")
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_config(
    device: str = "",
    signal_freq: str = "",
    current_range: int | None = None,
    range_limit_min: int | None = None,
    range_limit_max: int | None = None,
    voltage_range: int | None = None,
    show: bool = False,
    config_file: str = "",
    load: bool = False,
    confirm: bool = False,
) -> str:
    """Read (show=True) or write JS220 acquisition settings. Writing is state-changing —
    requires confirm=True; show=True or load=True alone does not.

    Args:
        device: Device path/serial, as shown by js220_list (optional).
        signal_freq: Sample rate, e.g. '100k' (optional).
        current_range: -1=auto, or manual index 0-5 (10A, 180mA, 18mA, 1.8mA, 180uA, 18uA).
        range_limit_min: Smallest-current index auto-ranging should still cover (optional).
        range_limit_max: Largest-current index auto-ranging should still cover (optional).
        voltage_range: -1=auto, 0=15V, 1=2V (optional).
        show: Show current settings instead of writing.
        config_file: History file for config writes (optional).
        load: Apply the last-written settings from config_file first; other flags override.
        confirm: Must be True to actually write settings (not required for show=True alone).
    """
    writing = any(
        v is not None
        for v in (current_range, range_limit_min, range_limit_max, voltage_range)
    ) or bool(signal_freq) or load
    if writing and not show and not confirm:
        return CONFIRM_MESSAGE
    args = ["js220", "config"]
    if device:
        args += ["--device", device]
    if signal_freq:
        args += ["--signal-freq", signal_freq]
    if current_range is not None:
        args += ["--current-range", str(current_range)]
    if range_limit_min is not None:
        args += ["--range-limit-min", str(range_limit_min)]
    if range_limit_max is not None:
        args += ["--range-limit-max", str(range_limit_max)]
    if voltage_range is not None:
        args += ["--voltage-range", str(voltage_range)]
    if show:
        args.append("--show")
    if config_file:
        args += ["--config-file", config_file]
    if load:
        args.append("--load")
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_profile_list(profiles_file: str = "") -> str:
    """List available named JS220 acquisition-setting profiles. Read-only."""
    args = ["js220", "profile", "list"]
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_profile_show(name: str, profiles_file: str = "") -> str:
    """Show a named JS220 profile's acquisition settings. Read-only.

    Args:
        name: Profile name, as shown by js220_profile_list.
    """
    args = ["js220", "profile", "show", name]
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)


@mcp.tool()
def js220_profile_apply(
    name: str,
    device: str = "",
    dry_run: bool = False,
    confirm: bool = False,
    profiles_file: str = "",
    config_file: str = "",
) -> str:
    """Push a named profile's acquisition settings to the JS220. State-changing unless
    dry_run=True — requires confirm=True otherwise.

    Args:
        name: Profile name, as shown by js220_profile_list.
        device: Device path/serial, as shown by js220_list (optional).
        dry_run: Preview the settings without sending them — does not require confirm.
        confirm: Must be True to actually apply the profile (ignored if dry_run=True).
        profiles_file: Profiles file path (optional).
        config_file: Config history file to record the applied settings in (optional).
    """
    if not dry_run and not confirm:
        return CONFIRM_MESSAGE
    args = ["js220", "profile", "apply", name]
    if device:
        args += ["--device", device]
    if dry_run:
        args.append("--dry-run")
    if profiles_file:
        args += ["--profiles-file", profiles_file]
    if config_file:
        args += ["--config-file", config_file]
    try:
        result = run_benchctl(args)
    except BenchctlError as exc:
        return err(exc)
    return fmt(result)
