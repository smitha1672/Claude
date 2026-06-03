# assistkit-datasheet-mcp

## Stack
- Language: Python 3.11
- MCP SDK: mcp[cli] (FastMCP)
- Vector store: ChromaDB (persistent)
- PDF parser: PyMuPDF (fitz)
- Embeddings: sentence-transformers

## Commands
- Start server: `python server.py`
- Install deps: `pip install -e .`
- Run tests: `pytest`

## Conventions
- Commit format: `feat(JIRA-ID): description`
- Branch format: `JIRA-ID-short-description`
- Pool data lives in `pool/` — never commit it
- No hardcoded paths — use config.py or .env
- Tool implementations live in tools/, core logic in core/

## Milestones
- [x] Scaffold — all tools registered, server starts
- [x] ds_add_pdf + ds_add_text + ds_list + ds_remove + ds_clear
- [x] ds_search — semantic search
- [ ] ds_find, ds_explain, ds_pins, ds_register
- [ ] ds_add_part — auto-fetch by part number
- [ ] ds_code — generate driver code
- [ ] ds_compare
- [ ] ds_export, ds_history, ds_history_clear
