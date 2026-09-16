Control the Jarvis voice-controlled bench assistant trigger process — opens
continuous mic capture in the background, and shuts it down cleanly on stop.
Webcam capture is intentionally deferred (no use case defined yet). Speech-to-text
and command dispatch are not wired in yet — this only manages the mic-capture
process lifecycle.

Script: /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/jarvis_trigger.py
Python: /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/.venv/bin/python
        (fall back to `python3` if that venv doesn't exist yet)
Log:    /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/run/jarvis.log

Usage:
  /Jarvis          → start continuous listening in the background
  /Jarvis stop     → stop the running Jarvis process cleanly
  /Jarvis status   → report whether Jarvis is currently running

Cases:

1. Start (no argument, or "start"):
   mkdir -p /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/run
   nohup <python> /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/jarvis_trigger.py start \
     > /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/run/jarvis.log 2>&1 &
   sleep 1
   cat /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/run/jarvis.log
   <python> /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/jarvis_trigger.py status
   Report back whether the mic actually opened (check the log line) and confirm
   the process is running.

2. Stop:
   <python> /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/jarvis_trigger.py stop

3. Status:
   <python> /home/smith/Public/code/Claude/assistkit/mcp/bench-jarvis/jarvis_trigger.py status
