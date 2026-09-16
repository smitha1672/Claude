# assistkit-bench-jarvis-mcp

MCP server wrapping the `benchctl` CLI (GW Instek GPP-4323 PSU + Joulescope JS220) as
callable tools. This is the integration point described in `scratch/Jarvis_MCP_Project_Review.md`
between Claude Code and the bench hardware. `jarvis_trigger.py` (invoked via the `/Jarvis`
slash command in `assistkit/claude/commands/Jarvis.md`) manages the mic-capture process
lifecycle, transcribes locally with faster-whisper, filters for a "Hey Jarvis" wake phrase,
and dispatches the result to these MCP tools directly (same process, no MCP transport) —
with a printed spoken-confirmation prompt before any state-changing call.

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
doc's confirmation-gate requirement — the caller (`jarvis_trigger.py`'s dispatch loop, or a
human) is responsible for only passing `confirm=True` after the user has verbally
confirmed the action.

## Milestones
- [x] Scaffold — app/config/server, `core/runner.py` subprocess wrapper, all tools registered
- [x] Manual end-to-end test against real PSU/JS220 hardware — read-only tools, confirm-gate,
  PSU power on/off round-trip, and JS220 power/capture + profile-apply flows all verified
- [x] Mic trigger script for `/Jarvis` — `jarvis_trigger.py` start/stop/status, PID-file +
  SIGTERM lifecycle, continuous `sounddevice` capture, degrades to lifecycle-only when no
  audio device is present. Webcam capture intentionally deferred (no use case defined yet).
- [x] Local STT wired into the trigger script — faster-whisper (`JARVIS_STT_MODEL`, default
  `base.en`) runs on fixed `JARVIS_STT_CHUNK_SECONDS`-second chunks (default 4s) off a
  background thread. Chunk-based, not VAD-segmented — a command can still get split
  across a chunk boundary. Degrades to capture-only (no transcription) when
  `faster-whisper` isn't installed.
- [x] "Hey Jarvis" phrase filter — `_extract_command` (regex `WAKE_PATTERN`) matches
  "jarvis"/"hey jarvis" anywhere in a chunk's transcript (not just as a prefix — filler
  and STT artifacts commonly precede it), uses the *last* match so a stray earlier mention
  doesn't eat the real command. No match → discarded to stderr as `[jarvis] discarded ...`
  (not treated as a command); match found → the text after it is printed to stdout as
  `[jarvis heard] ...`, which is what the dispatch milestone below will consume.
- [x] Command dispatch — `COMMAND_PATTERNS`/`_parse_command` is a fast-path regex parser
  for the small set of commands from the design review's "Voice → CLI mapping" examples
  (not general NLU), calling the `tools/psu.py`/`tools/js220.py` functions directly.
  Read-only commands dispatch immediately; state-changing ones print
  `[jarvis] about to ...` and wait up to `JARVIS_CONFIRM_TIMEOUT_CHUNKS` chunks (default
  3) for a yes/no reply on a later chunk (no wake phrase needed for the reply itself) —
  no reply times out to cancelled. JS220 `power off` is never dispatched with `force=True`
  from voice, so a misheard command can't disconnect the DUT; that stays a text/CLI-only
  action. There's no TTS yet, so "spoken back" is a printed line and "verbal yes" is
  whatever the next transcribed chunk says — both need a real voice loop to validate,
  see below.
