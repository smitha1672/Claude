#!/usr/bin/env python3
"""Process lifecycle for the /Jarvis trigger: opens continuous mic capture on
start, transcribes it locally with faster-whisper in fixed-size chunks, filters
for a "Hey Jarvis"/"Jarvis" wake phrase, parses the result into a bench-jarvis
MCP tool call, speaks back (prints) a confirmation prompt before any
state-changing call, and releases everything cleanly on stop. Webcam capture
is intentionally deferred (no use case defined yet, per
scratch/Jarvis_MCP_Project_Review.md).
"""
import os
import queue
import re
import signal
import sys
import threading
import time
from pathlib import Path

BASE_DIR = Path(__file__).parent
RUN_DIR = BASE_DIR / "run"
PID_FILE = RUN_DIR / "jarvis.pid"

SAMPLE_RATE = 16000
CHUNK_SECONDS = float(os.getenv("JARVIS_STT_CHUNK_SECONDS", "4"))
STT_MODEL_SIZE = os.getenv("JARVIS_STT_MODEL", "base.en")
# How many transcribed chunks to wait for a yes/no reply before cancelling a
# pending state-changing command.
CONFIRM_TIMEOUT_CHUNKS = int(os.getenv("JARVIS_CONFIRM_TIMEOUT_CHUNKS", "3"))

# Matches "jarvis" optionally preceded by "hey" (e.g. "hey jarvis", "okay
# jarvis," "Jarvis,"). Not anchored to the start — filler words or STT
# artifacts often precede it in a natural utterance.
WAKE_PATTERN = re.compile(r"\b(?:hey\s+)?jarvis\b[,.]?\s*", re.IGNORECASE)

AFFIRM_WORDS = ("yes", "yeah", "yep", "confirm", "confirmed", "do it", "go ahead")
DENY_WORDS = ("no", "nope", "cancel", "stop", "never mind", "nevermind")

try:
    import sounddevice as sd
except (ImportError, OSError):
    # OSError: the pip package is installed but libportaudio isn't (e.g. apt
    # install portaudio19-dev/libportaudio2 wasn't run).
    sd = None

try:
    import numpy as np
    from faster_whisper import WhisperModel
except ImportError:
    np = None
    WhisperModel = None

from tools.js220 import js220_info, js220_list, js220_power
from tools.psu import psu_measure, psu_power, psu_profile_apply, psu_status


def _is_running(pid: int) -> bool:
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    return True


def _read_pid() -> int | None:
    if not PID_FILE.exists():
        return None
    try:
        return int(PID_FILE.read_text().strip())
    except ValueError:
        return None


def cmd_status() -> int:
    pid = _read_pid()
    if pid and _is_running(pid):
        print(f"Jarvis is running (pid {pid}).")
        return 0
    print("Jarvis is not running.")
    return 1


def cmd_stop() -> int:
    pid = _read_pid()
    if not pid or not _is_running(pid):
        print("Jarvis is not running.")
        PID_FILE.unlink(missing_ok=True)
        return 1
    os.kill(pid, signal.SIGTERM)
    for _ in range(50):
        if not _is_running(pid):
            print("Jarvis stopped.")
            return 0
        time.sleep(0.1)
    print(f"Jarvis (pid {pid}) did not stop within 5s.")
    return 1


def _extract_command(text: str) -> str | None:
    """Return the instruction following the last wake-phrase mention in text,
    or None if the wake phrase isn't present at all.

    Matches anywhere in the transcript, not just a prefix — natural speech and
    STT artifacts often put filler before it ("okay, jarvis, turn off channel
    1"). Uses the last match so an earlier stray "jarvis" in ambient
    conversation doesn't eat the real command that follows it.
    """
    matches = list(WAKE_PATTERN.finditer(text))
    if not matches:
        return None
    return text[matches[-1].end():].strip()


def _contains_word(text: str, words: tuple[str, ...]) -> bool:
    lowered = text.lower()
    return any(re.search(rf"\b{re.escape(w)}\b", lowered) for w in words)


# Fast-path parser for the small set of well-defined voice commands from the
# design review's "Voice → CLI mapping" examples — not general NLU. Each
# builder takes the regex match and returns a dict: description (spoken back
# before confirming), stateful (whether it touches hardware), and call(confirm)
# invoking the actual bench-jarvis MCP tool function directly (same process,
# no MCP transport needed).
COMMAND_PATTERNS = [
    (
        re.compile(r"^(?:power|turn) on (?:psu )?channel (\d)$", re.I),
        lambda m: {
            "description": f"power on PSU channel {m.group(1)}",
            "stateful": True,
            "call": lambda confirm: psu_power(state="on", channel=int(m.group(1)), confirm=confirm),
        },
    ),
    (
        re.compile(r"^(?:power|turn) off (?:psu )?channel (\d)$", re.I),
        lambda m: {
            "description": f"power off PSU channel {m.group(1)}",
            "stateful": True,
            "call": lambda confirm: psu_power(state="off", channel=int(m.group(1)), confirm=confirm),
        },
    ),
    (
        re.compile(r"^(?:what'?s|what is) the (?:voltage|current|power) on channel (\d)$", re.I),
        lambda m: {
            "description": f"measure PSU channel {m.group(1)}",
            "stateful": False,
            "call": lambda confirm: psu_measure(channel=int(m.group(1))),
        },
    ),
    (
        re.compile(r"^(?:check )?channel (\d) status$", re.I),
        lambda m: {
            "description": f"check PSU channel {m.group(1)} status",
            "stateful": False,
            "call": lambda confirm: psu_status(channel=int(m.group(1))),
        },
    ),
    (
        re.compile(r"^(?:run|apply) (?:the )?profile (\S+)(?: on channel (\d))?$", re.I),
        lambda m: {
            "description": f"apply PSU profile '{m.group(1)}'"
            + (f" on channel {m.group(2)}" if m.group(2) else ""),
            "stateful": True,
            "call": lambda confirm: psu_profile_apply(
                name=m.group(1), channel=int(m.group(2)) if m.group(2) else 0, confirm=confirm
            ),
        },
    ),
    (
        re.compile(r"^(?:power|turn) on (?:the )?js220$", re.I),
        lambda m: {
            "description": "power on JS220",
            "stateful": True,
            "call": lambda confirm: js220_power(state="on", confirm=confirm),
        },
    ),
    (
        # JS220 power off never passes force=True here — a misheard voice
        # command must not be able to disconnect the DUT. Force-disconnect is
        # a text/CLI-only action per the design review.
        re.compile(r"^(?:power|turn) off (?:the )?js220$", re.I),
        lambda m: {
            "description": "power off JS220",
            "stateful": True,
            "call": lambda confirm: js220_power(state="off", confirm=confirm),
        },
    ),
    (
        re.compile(r"^list (?:the )?js220(?: devices)?$", re.I),
        lambda m: {
            "description": "list JS220 devices",
            "stateful": False,
            "call": lambda confirm: js220_list(),
        },
    ),
    (
        re.compile(r"^(?:js220 )?(?:info|status)$", re.I),
        lambda m: {
            "description": "show JS220 info",
            "stateful": False,
            "call": lambda confirm: js220_info(),
        },
    ),
]


def _parse_command(command: str):
    normalized = command.strip().rstrip(".!?").lower()
    for pattern, builder in COMMAND_PATTERNS:
        m = pattern.match(normalized)
        if m:
            return builder(m)
    return None


def _transcribe_loop(model, audio_queue: "queue.Queue", stop_event: threading.Event) -> None:
    """Pull mic audio off audio_queue, transcribe it in fixed-size chunks,
    filter for the "Hey Jarvis"/"Jarvis" wake phrase, parse it into an MCP
    tool call, and dispatch it — state-changing calls only after a confirmed
    yes/no reply on a later chunk (no wake phrase needed for that reply, since
    it's a continuation of the same exchange).

    Chunk-based (not silence/VAD-segmented) — simple first cut. A command (or
    a confirmation reply) can still get split across a chunk boundary.
    """
    samples_per_chunk = int(SAMPLE_RATE * CHUNK_SECONDS)
    buffer = np.empty((0,), dtype="float32")
    pending = None  # dict from a COMMAND_PATTERNS builder, awaiting yes/no
    pending_ttl = 0

    while not stop_event.is_set():
        try:
            block = audio_queue.get(timeout=0.5)
        except queue.Empty:
            continue
        buffer = np.concatenate([buffer, block[:, 0]])
        if len(buffer) < samples_per_chunk:
            continue
        audio, buffer = buffer[:samples_per_chunk], buffer[samples_per_chunk:]
        segments, _ = model.transcribe(audio, language="en")
        text = " ".join(seg.text.strip() for seg in segments).strip()
        if not text:
            continue

        if pending is not None:
            if _contains_word(text, AFFIRM_WORDS):
                print(f"[jarvis] confirmed: {pending['description']}", flush=True)
                print(pending["call"](True), flush=True)
                pending = None
            elif _contains_word(text, DENY_WORDS):
                print(f"[jarvis] cancelled: {pending['description']}", flush=True)
                pending = None
            else:
                pending_ttl -= 1
                if pending_ttl <= 0:
                    print(
                        f"[jarvis] confirmation timed out, cancelled: {pending['description']}",
                        flush=True,
                    )
                    pending = None
                else:
                    print(
                        f"[jarvis] still waiting for confirmation (say 'yes' or 'no'): "
                        f"{pending['description']}",
                        flush=True,
                    )
            continue

        command = _extract_command(text)
        if command is None:
            print(f"[jarvis] discarded (no wake phrase): {text}", file=sys.stderr, flush=True)
            continue
        if not command:
            print("[jarvis] heard wake phrase with no command", flush=True)
            continue

        parsed = _parse_command(command)
        if parsed is None:
            print(f"[jarvis] didn't recognize command: {command}", flush=True)
            continue

        if parsed["stateful"]:
            pending = parsed
            pending_ttl = CONFIRM_TIMEOUT_CHUNKS
            print(
                f"[jarvis] about to {parsed['description']} — say 'yes'/'confirm' or 'no'/'cancel'.",
                flush=True,
            )
        else:
            print(parsed["call"](False), flush=True)


def _on_audio_factory(audio_queue: "queue.Queue"):
    def _on_audio(indata, frames, time_info, status):
        if status:
            print(f"jarvis: audio status: {status}", file=sys.stderr, flush=True)
        audio_queue.put(indata.copy())

    return _on_audio


def cmd_start() -> int:
    pid = _read_pid()
    if pid and _is_running(pid):
        print(f"Jarvis is already running (pid {pid}).")
        return 1

    RUN_DIR.mkdir(parents=True, exist_ok=True)
    PID_FILE.write_text(str(os.getpid()))

    stop_event = threading.Event()
    signal.signal(signal.SIGTERM, lambda signum, frame: stop_event.set())
    signal.signal(signal.SIGINT, lambda signum, frame: stop_event.set())

    stream = None
    transcribe_thread = None

    if sd is None:
        print(
            "Jarvis started WITHOUT mic capture — 'sounddevice' is not installed "
            "(pip install -e '.[audio]') or no audio device is available. "
            "Process/PID lifecycle only.",
            flush=True,
        )
    else:
        audio_queue: "queue.Queue" = queue.Queue()
        stream = sd.InputStream(
            channels=1, samplerate=SAMPLE_RATE, callback=_on_audio_factory(audio_queue)
        )
        stream.start()

        if WhisperModel is None:
            print(
                "Jarvis listening (mic open) WITHOUT transcription — 'faster-whisper' is "
                "not installed (pip install -e '.[stt]'). Audio is captured and discarded.",
                flush=True,
            )
        else:
            print(f"Loading local STT model '{STT_MODEL_SIZE}'...", flush=True)
            model = WhisperModel(STT_MODEL_SIZE, device="cpu", compute_type="int8")
            transcribe_thread = threading.Thread(
                target=_transcribe_loop, args=(model, audio_queue, stop_event), daemon=True
            )
            transcribe_thread.start()
            print(
                "Jarvis listening (mic open, local STT running). Stop with /Jarvis stop.",
                flush=True,
            )

    try:
        stop_event.wait()
    finally:
        if stream is not None:
            stream.stop()
            stream.close()
        if transcribe_thread is not None:
            transcribe_thread.join(timeout=5)
        PID_FILE.unlink(missing_ok=True)
        print("Jarvis stopped, mic released.")
    return 0


def main(argv: list[str]) -> int:
    commands = {"start": cmd_start, "stop": cmd_stop, "status": cmd_status}
    if len(argv) != 2 or argv[1] not in commands:
        print("usage: jarvis_trigger.py {start|stop|status}", file=sys.stderr)
        return 2
    return commands[argv[1]]()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
