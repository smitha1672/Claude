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

---

## Scenarios

### 1. Index a datasheet and search it

```
add /home/smith/datasheets/MSPM0C1105.pdf
```
```
what power modes does the MSPM0C1105 support?
```
```
search for how to configure the watchdog timer
```

---

### 2. Look up a specific parameter

```
what is the operating voltage range of the MSPM0C1105?
```
```
what is the I2C clock speed in fast mode plus?
```
```
what is the sleep current in STANDBY0 mode?
```

---

### 3. Understand a spec or timing diagram

```
explain the MSPM0C1105 startup timing sequence
```
```
explain what STOP0 and STOP2 power modes mean and when to use each
```
```
explain the difference between SLEEP and STANDBY mode
```

---

### 4. Look up pins

```
what are the functions of pin PA0 on the MSPM0C1105?
```
```
show all I2C pins on the MSPM0C1105
```
```
which pins support wakeup from SHUTDOWN mode?
```

---

### 5. Decode a register

```
describe the PMCTL register on the MSPM0C1105
```
```
decode PMCTL value 0x04 on the MSPM0C1105
```

---

### 6. Generate driver code with SDK

```
add /home/smith/datasheets/MSPM0C1105.pdf
write an I2C controller init function for MSPM0C1105 using SDK /home/smith/ti/mspm0_sdk_2_09_00_01
```
```
write a function to enter STANDBY0 mode on MSPM0C1105 using SDK /home/smith/ti/mspm0_sdk_2_09_00_01
```
```
generate a UART init for 115200 baud on MSPM0C1105 using SDK /home/smith/ti/mspm0_sdk_2_09_00_01
```

---

### 7. Compare two parts

First, index both:
```
add /home/smith/datasheets/MSPM0C1105.pdf
add /home/smith/datasheets/STM32G031.pdf
```

Then compare:
```
compare MSPM0C1105 and STM32G031
```
```
compare MSPM0C1105 and STM32G031 on flash, RAM, and sleep current
```

---

### 8. Auto-fetch a datasheet by part number

```
fetch BME280 datasheet
```
```
fetch MSPM0C1104 datasheet
```

If the auto-fetch fails, it will tell you which URLs were tried. Use `ds_add_pdf` with a local file as a fallback.

---

### 9. Review query history and export results

```
show my query history
```
```
show history for MSPM0C1105
```
```
show history for ds_code queries
```
```
export the last result
```

---

### 10. Manage the pool

```
list all indexed datasheets
```
```
remove MSPM0C1105
```
```
clear the entire pool
```

---

## Tools

| Tool | Description |
|------|-------------|
| `ds_add_pdf` | Parse and index a PDF datasheet; optionally link an SDK folder |
| `ds_add_text` | Index a raw text snippet |
| `ds_add_part` | Auto-fetch datasheet by part number (TI, ST, NXP, Bosch) |
| `ds_list` | List all indexed parts |
| `ds_remove` | Remove a part from the pool |
| `ds_clear` | Wipe the entire pool |
| `ds_search` | Semantic search across all indexed datasheets |
| `ds_find` | Extract a specific parameter value (voltage, address, current, etc.) |
| `ds_explain` | Explain a spec or timing diagram in plain English |
| `ds_pins` | Look up pin functions and recommended connections |
| `ds_register` | Decode a register value using the datasheet register map |
| `ds_code` | Generate SDK-aware C driver or init code |
| `ds_compare` | Compare two parts on key specs |
| `ds_export` | Save the last query result to a Markdown file |
| `ds_history` | View past queries, filterable by part or tool |
| `ds_history_clear` | Clear the query log |

---

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
