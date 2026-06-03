# assistkit-datasheet-mcp

A hardware datasheet indexing and querying MCP server for Claude Code.
Index PDF datasheets locally, then use natural-language tools to search, explain, and generate code from them.

## Setup

```bash
pip install -e .
cp .env.example .env   # edit POOL_DIR if needed
python server.py
```

## Tools

| Tool | Status | Description |
|------|--------|-------------|
| `ds_add_pdf` | Done | Parse and index a PDF datasheet |
| `ds_add_text` | Done | Index a raw text snippet |
| `ds_add_part` | Stub | Auto-fetch by part number from web |
| `ds_list` | Done | List all indexed parts |
| `ds_remove` | Done | Remove a part from the pool |
| `ds_clear` | Done | Wipe the entire pool |
| `ds_export` | Done | Export last query result to Markdown |
| `ds_history` | Done | View past queries |
| `ds_history_clear` | Done | Clear query log |
| `ds_search` | Done | Semantic search across all datasheets |
| `ds_find` | Stub | Extract a specific parameter value |
| `ds_explain` | Stub | Explain a spec in plain English |
| `ds_pins` | Stub | Look up pin functions |
| `ds_register` | Stub | Decode a register value |
| `ds_code` | Stub | Generate driver/init code |
| `ds_compare` | Stub | Compare two parts on key specs |

## Configuration

All settings are read from environment variables (`.env` or shell):

| Variable | Default | Description |
|----------|---------|-------------|
| `POOL_DIR` | `./pool` | Where ChromaDB and SQLite data are stored |
| `EMBED_MODEL` | `all-MiniLM-L6-v2` | sentence-transformers model name |
| `CHUNK_SIZE` | `500` | Characters per text chunk |
| `CHUNK_OVERLAP` | `50` | Overlap between consecutive chunks |

## Example usage

```
ds_add_pdf path=/datasheets/BME280.pdf
ds_search query="I2C address options"
ds_search query="power consumption in sleep mode" part=BME280
ds_list
ds_remove part=BME280
```
