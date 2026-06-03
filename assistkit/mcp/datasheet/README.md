# assistkit-datasheet-mcp

A hardware datasheet indexing and querying MCP server for Claude Code.
Index PDF datasheets locally, then use natural language to search, explain, and generate SDK-aware driver code.

## Setup

```bash
cd /path/to/assistkit/mcp/datasheet
python -m venv .venv && source .venv/bin/activate
pip install -e .
cp .env.example .env
```

Register with Claude Code (user scope — available in all sessions):

```bash
claude mcp add assistkit-datasheet --scope user \
  /path/to/.venv/bin/python /path/to/server.py
```

Verify inside a Claude Code session:

```
/mcp
```

## Usage

Just talk to Claude naturally — no special syntax needed.

```
add /path/to/MSPM0C1105.pdf
add /path/to/MSPM0C1105.pdf with SDK /home/smith/ti/mspm0_sdk_2_09_00_01
what power modes does MSPM0C1105 support?
write an I2C init function for MSPM0C1105
show everything in the pool
remove MSPM0C1105
```

## Tools

| Tool | Status | Description |
|------|--------|-------------|
| `ds_add_pdf` | Done | Parse and index a PDF datasheet; optionally link an SDK folder |
| `ds_add_text` | Done | Index a raw text snippet |
| `ds_add_part` | Stub | Auto-fetch datasheet by part number from the web |
| `ds_list` | Done | List all indexed parts |
| `ds_remove` | Done | Remove a part from the pool |
| `ds_clear` | Done | Wipe the entire pool |
| `ds_search` | Done | Semantic search across all indexed datasheets |
| `ds_find` | Stub | Extract a specific parameter value (voltage, address, etc.) |
| `ds_explain` | Stub | Explain a spec or timing diagram in plain English |
| `ds_pins` | Stub | Look up pin functions and recommended connections |
| `ds_register` | Stub | Decode a register value using the datasheet register map |
| `ds_code` | Stub | Generate SDK-aware driver or init code |
| `ds_compare` | Stub | Compare two parts on key specs |
| `ds_export` | Done | Save the last query result to a Markdown file |
| `ds_history` | Done | View past queries, filterable by part or tool |
| `ds_history_clear` | Done | Clear the query log |

## ds_code — SDK-aware code generation

`ds_code` searches the datasheet for the relevant section, then scans the linked SDK for matching headers and examples to generate code that uses the real SDK API.

**Step 1 — Index the datasheet and link the SDK:**
```
add /home/smith/Public/AssistKit/datasheet/mspm0c1105.pdf
with SDK /home/smith/ti/mspm0_sdk_2_09_00_01
```

**Step 2 — Generate code:**
```
write an I2C init function for MSPM0C1105
```

**Example output:**
```c
#include "ti/driverlib/driverlib.h"

void I2C0_init(void) {
    // FM+ mode, 1MHz — from datasheet p.54
    DL_I2C_setClockConfig(I2C0, DL_I2C_CLOCK_BUSCLK, 31);
    DL_I2C_enableController(I2C0);
}
```

## Configuration

All settings via `.env` or shell environment — no hardcoded values:

| Variable | Default | Description |
|----------|---------|-------------|
| `POOL_DIR` | `./pool` | Where ChromaDB and SQLite data are stored |
| `EMBED_MODEL` | `all-MiniLM-L6-v2` | sentence-transformers model name |
| `CHUNK_SIZE` | `500` | Characters per text chunk |
| `CHUNK_OVERLAP` | `50` | Overlap between consecutive chunks |

## Data layout

```
pool/
├── chroma/        # vector embeddings (ChromaDB)
├── exports/       # saved results from ds_export
└── metadata.db    # part registry + query log (SQLite)
```

`pool/` is gitignored — never committed.
