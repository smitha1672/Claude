#!/usr/bin/env python3
"""Process lifecycle for the /Jarvis trigger: opens continuous mic capture on
start, releases it cleanly on stop. Webcam capture is intentionally deferred
(no use case defined yet, per scratch/Jarvis_MCP_Project_Review.md). STT and
command dispatch are not wired in yet — see _on_audio below for the hook
point a future milestone will fill in.
"""
import os
import signal
import sys
import threading
import time
from pathlib import Path

BASE_DIR = Path(__file__).parent
RUN_DIR = BASE_DIR / "run"
PID_FILE = RUN_DIR / "jarvis.pid"

try:
    import sounddevice as sd
except ImportError:
    sd = None


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


def _on_audio(indata, frames, time_info, status):
    # Hook point for the local STT milestone (Whisper.cpp/faster-whisper) —
    # not wired in yet.
    pass


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
    if sd is not None:
        stream = sd.InputStream(channels=1, samplerate=16000, callback=_on_audio)
        stream.start()
        print(
            "Jarvis listening (mic open, continuous capture). Stop with /Jarvis stop.",
            flush=True,
        )
    else:
        print(
            "Jarvis started WITHOUT mic capture — 'sounddevice' is not installed "
            "(pip install -e '.[audio]') or no audio device is available. "
            "Process/PID lifecycle only.",
            flush=True,
        )

    try:
        stop_event.wait()
    finally:
        if stream is not None:
            stream.stop()
            stream.close()
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
