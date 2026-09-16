# Jarvis — Voice/Vision-Controlled PSU Assistant (Design Review)

## Overview
- **Goal:** Trigger `/Jarvis` in Claude Code to open a webcam and mic, listen for a spoken command (e.g. "power on PSU channel 1"), and execute the corresponding action on the bench — hands-free operation while the user's hands are occupied.
- **Host:** Ubuntu 22.04
- **Webcam:** Logitech Brio 500 (on hand, standard UVC device)
- **Command surface:** existing `benchctl` CLI (see below) — already implements validated, hardware-safe PSU and JS220 control.
- **No wake-word detection required** — `/Jarvis` is invoked manually to start the session.

## Command Surface — `benchctl`

Voice commands map onto the existing `benchctl` CLI rather than a bespoke PSU script — it already has validated safety behavior built in, so Jarvis should call *into* it (subprocess or shared Python import), not reimplement it.

| Group | Commands | Notes |
|---|---|---|
| `benchctl psu` | `info`, `measure`, `set`, `power`, `status`, `protect`, `profile` | GW Instek GPP-4323 over SCPI-over-LAN. `set` rejects out-of-range values before sending (`LimitError`). `power off` / `power --all off` are the main "hands busy" targets. |
| `benchctl js220` | `list`, `info`, `power`, `measure`, `capture`, `config`, `profile` | Joulescope JS220 meter. `power off` disconnects the DUT — gated behind `--force`. |
| `benchctl sequence run` | Runs a JSON step-list against the PSU | Good fit for a single voice command that triggers a pre-defined multi-step sequence (e.g. "run the step-up sequence"). |

**Safety carries over from `benchctl` itself** — no need to reinvent it in Jarvis:
- Value validation happens before any hardware I/O (`LimitError`, profile validation at load time).
- Destructive/DUT-disconnecting actions (`psu power off`, `js220 power off`) already require `--force` at the CLI level — Jarvis's dispatcher must pass `--force` deliberately, not by default, so a misheard command can't silently disconnect the DUT.
- `status`/`measure` calls are read-only and safe to run without confirmation.
- Every command supports `--json`, which is what the MCP tool wrapper should parse (skip `--no-json`).

**Voice → CLI mapping (examples):**
- "power on PSU channel 1" → `benchctl psu power --channel 1 on --json`
- "what's the voltage on channel 2" → `benchctl psu measure --channel 2 --json`
- "turn off channel 1" → `benchctl psu power --channel 1 off --json` (destructive on JS220 side if in series; PSU-side `power off` is not `--force`-gated per the CLI doc — still worth a confirmation step, see Open Questions)
- "run the ble dut profile on channel 1" → `benchctl psu profile apply ble_dut_3v3 --channel 1 --json`
- "check channel 1 status" → `benchctl psu status --channel 1 --json`

## Architecture

1. **Trigger — `/Jarvis` slash command**
   - Starts a script that opens the webcam and begins mic capture (continuous or push-to-talk).

2. **Webcam** *(optional — use case not yet decided)*
   - Standard UVC/V4L2 device, accessible via OpenCV or `ffmpeg`/`v4l2`.
   - Camera is installed and available, but no concrete use case defined yet (e.g. presence check, visual context, face-recognition gate). Can be deferred — not required for the core voice → PSU action flow.

3. **Speech-to-Text (STT)**
   - Recommend local Whisper.cpp or faster-whisper (no internet dependency, low latency, good fit for a lab bench).
   - Converts spoken command to text.

4. **Command filter — "Hey Jarvis" preamble**
   - Not a hardware wake-word (no wake-word engine used). It's a software-level filter phrase checked against the transcribed text.
   - STT runs continuously on all captured audio; the parser checks if the transcript starts with (or contains) "Hey Jarvis" / "Jarvis."
   - If the phrase is absent → discard as ambient noise/conversation, no action taken.
   - If present → strip the preamble, treat the remainder as the actual instruction (e.g. "power on PSU channel 1").
   - Tradeoffs: continuous STT runs even when not being addressed (CPU cost); string-matching on transcribed text can miss commands if STT mishears "Jarvis."

5. **Command dispatch + confirmation gate**
   - Filtered instruction text passed to Claude Code as a prompt, or parsed directly for a fast-path on simple, well-defined commands.
   - Before calling any state-changing `benchctl` command (power on/off, `set`, `protect`, `profile apply`, `sequence run`), Jarvis speaks back what it's about to do (e.g. "Turn on PSU channel 1 — confirm?") and waits for a verbal "yes"/"confirm" before dispatching.
   - Read-only commands (`measure`, `status`, `info`, `js220 list`) skip confirmation and execute immediately.
   - Claude Code calls an MCP tool wrapping the corresponding `benchctl` CLI invocation once confirmed.

6. **MCP server**
   - Wraps `benchctl` subcommands as callable tools (`psu_power`, `psu_measure`, `psu_status`, `psu_profile_apply`, `js220_power`, `js220_measure`, `sequence_run`, etc.) — either by shelling out to the CLI with `--json` or importing `benchctl`'s underlying Python modules directly.
   - This is the integration point between Claude Code and the hardware; `benchctl` itself remains the single source of truth for validation and safety gating (see Command Surface section above).

7. **Session behavior**
   - Continuous listening loop, started by `/Jarvis`.

8. **Stop/Disable — `/Jarvis stop` (or `/Jarvis off`)**
   - Slash command signals the running background process to shut down cleanly.
   - Implementation: background listener writes a PID file (or exposes a simple IPC socket/file flag) on start; `/Jarvis stop` reads the PID and sends a termination signal (or writes the stop flag the loop checks each cycle).
   - On shutdown: release webcam handle, close mic stream, remove PID file, exit process cleanly (no orphaned processes holding the camera/mic).

## Open Questions / Decisions Needed
- [x] Continuous listening vs. push-to-talk for mic capture? → **Continuous**, stopped via `/Jarvis stop`.
- [x] Single-shot command per `/Jarvis` call, or multi-turn session? → **Continuous/multi-turn**, until stopped.
- [ ] Webcam use case not yet defined — deferred for now, not blocking core flow. **Hardware confirmed: Logitech Brio 500 on hand.**
- [x] Local STT (Whisper.cpp/faster-whisper) vs. cloud STT → **Local confirmed.**
- [x] Confirmation step before executing power actions → **Yes, required.** Any command that changes hardware state (power on/off, set voltage/current, protect, profile apply, sequence run) must be spoken back for confirmation before dispatch; read-only commands (`measure`, `status`, `info`) execute immediately without confirmation.
- [x] Channel/voltage safety limits enforced in the Python layer, independent of voice input → **Yes.** Already covered by `benchctl` itself (`LimitError` on out-of-range `set`, profile validation at load time, per-channel hardware caps in `CHANNEL_LIMITS`) — Jarvis relies on these rather than duplicating limit checks; voice/STT layer never bypasses `benchctl`'s validation.

## Risks / Notes
- **Latency:** wake detection (n/a) → STT → Claude Code invocation may take a few seconds; acceptable for bench control, not real-time.
- **Safety:** PSU channels control real hardware — misheard commands could cause unintended power state changes. Recommend a confirmation step or hard-coded voltage/channel limits validated before execution.
- **Reusability:** Wrapping `benchctl` as an MCP server keeps it reusable beyond this voice-control use case.

## Next Steps
1. Build MCP server exposing `benchctl` commands as tools.
2. Build webcam + mic trigger script for `/Jarvis`.
3. Wire STT output into Claude Code prompt / MCP tool calls.
4. Test end-to-end with a single command before adding multi-turn/session support.
