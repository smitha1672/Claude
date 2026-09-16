# assistkit-bench-jarvis-mcp

MCP server wrapping the `benchctl` CLI (GW Instek GPP-4323 PSU + Joulescope JS220) as
callable tools. This is the integration point described in `scratch/Jarvis_MCP_Project_Review.md`
between Claude Code and the bench hardware — the voice/webcam trigger layer (`/Jarvis`) is a
separate, not-yet-built piece that will call into these tools once a command is confirmed.

## Stack
- Language: Python 3.10+
- MCP SDK: mcp[cli] (FastMCP)
- Integration: subprocess shells out to the `benchctl` CLI with `--json`; benchctl itself
  (a sibling project, not a dependency of this one) owns all validation and hardware-safety
  gating (`LimitError`, `--force` checks, profile validation). This server never reimplements
  that logic — see `core/runner.py`.

## Commands
- Start server: `python server.py`
- Install deps: `pip install -e .`
- `benchctl` must be on `PATH`, or set `BENCHCTL_BIN` in `.env` to its full path.
- Set `BENCHCTL_CWD` in `.env` to the `benchctl` repo root (or wherever its `config/` dir
  lives) — benchctl resolves relative defaults like `config/psu_profiles.toml` against the
  process's working directory, not its install location, so without this the profile/config
  tools fail unless the MCP server happens to be launched from that directory.

## Conventions
- Commit format: `feat(JIRA-ID): description`
- Branch format: `JIRA-ID-short-description`
- No hardcoded paths — use config.py or .env
- Tool implementations live in `tools/`, subprocess/runner logic in `core/`

## Safety gate
Every state-changing tool (PSU `set`/`power`/`protect`/`profile_apply`, JS220
`power`/`capture`/`config`/`profile_apply`, `sequence_run`) takes a `confirm: bool = False`
parameter and refuses to run unless it's explicitly `True`. Read-only tools (`measure`,
`status`, `info`, `list`, profile `list`/`show`) execute immediately. This mirrors the design
doc's confirmation-gate requirement — the caller (eventually the Jarvis voice dispatcher) is
responsible for only passing `confirm=True` after the user has verbally confirmed the action.

## Milestones
- [x] Scaffold — app/config/server, `core/runner.py` subprocess wrapper, all tools registered
- [ ] Manual end-to-end test against real PSU/JS220 hardware
- [ ] Webcam + mic trigger script for `/Jarvis`
- [ ] Wire local STT output into Claude Code prompt / these MCP tool calls
- [ ] `/Jarvis stop` shutdown path
