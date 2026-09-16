#!/usr/bin/env python3
"""Process lifecycle for the /Jarvis trigger: opens continuous mic capture on
start, transcribes it locally with faster-whisper in fixed-size chunks, and
releases everything cleanly on stop. Webcam capture is intentionally deferred
(no use case defined yet, per scratch/Jarvis_MCP_Project_Review.md). The "Hey
Jarvis" phrase filter and command dispatch into the bench-jarvis MCP tools are
not wired in yet — transcribed text is only printed/logged for now.
"""
import os
import queue
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


def _transcribe_loop(model, audio_queue: "queue.Queue", stop_event: threading.Event) -> None:
    """Pull mic audio off audio_queue, transcribe it in fixed-size chunks.

    Chunk-based (not silence/VAD-segmented) — simple first cut. A command can
    get split across a chunk boundary; that's a known limitation for the "Hey
    Jarvis" filter / dispatch milestones to account for, not fixed here.
    """
    samples_per_chunk = int(SAMPLE_RATE * CHUNK_SECONDS)
    buffer = np.empty((0,), dtype="float32")

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
        if text:
            print(f"[jarvis heard] {text}", flush=True)


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
