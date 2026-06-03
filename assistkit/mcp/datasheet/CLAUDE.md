# assistkit-datasheet-mcp

## Stack
- Language: Python 3.10+
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
- [x] ds_find, ds_explain, ds_pins, ds_register
- [x] ds_add_part — tries TI/ST/NXP/Bosch URL patterns, falls back to error with tried URLs
- [x] ds_code — SDK-aware driver code generation (keyword + part-family file scoring)
- [x] ds_compare
- [x] ds_export, ds_history, ds_history_clear
