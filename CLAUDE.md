# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository layout

This repo (`assistkit`) holds two unrelated kinds of Claude Code customization, both under `assistkit/`:

- `assistkit/claude/commands/` — slash command definitions (plain Markdown, no code) that get synced into `~/.claude/commands/` for use in *other* repos (`kt`, `iot-firefly-ext`). Run `assistkit/sync-commands.sh` to copy them.
- `assistkit/mcp/datasheet/` — `assistkit-datasheet-mcp`, a standalone Python MCP server. It has its own `CLAUDE.md` — read that file when working inside `assistkit/mcp/datasheet/`; the notes below are a pointer, not a duplicate.

## Command sync (`assistkit/claude/commands/`)

Each `.md` file is a slash command body (no frontmatter) — the filename becomes the command name after sync. They encode git-worktree workflows for other repos on this machine, with paths, branch-naming rules, and step-by-step git commands baked in:

- `kt-jira`, `kt-worktree-add`, `kt-worktree-mirror`, `kt-worktree-list`, `kt-worktree-prune`, `kt-worktree-remove` — operate on the `kt` monorepo at `/home/smith/workspace/code/KeepTruckin/kt`, worktrees under `/home/smith/workspace/code_worktree/`.
- `ext-iot-firefly-worktree-mirror`, `ext-iot-firefly-worktree-remove` — same pattern for `iot-firefly-ext` at `/home/smith/workspace/code/iot-firefly-ext`.

After editing a command file, run `assistkit/sync-commands.sh` to install it to `~/.claude/commands/`.

Branch naming convention used throughout these commands: `<type>/<JIRA-ID>-<short-description>` where `<type>` is `feature|bugfix|hotfix|mirror|none` (`none` drops the prefix).

## assistkit-datasheet-mcp (`assistkit/mcp/datasheet/`)

Hardware datasheet indexing/query MCP server (FastMCP). Full details, commands, and conventions live in `assistkit/mcp/datasheet/CLAUDE.md` — key points:

- Run: `python server.py` (from that directory, with deps installed via `pip install -e .`).
- Architecture: `app.py` defines the shared `mcp` FastMCP instance; `server.py` imports each `tools/*` module (registering their `@mcp.tool()` functions as a side effect) then calls `mcp.run()`. `core/` holds the framework-agnostic logic (`parser.py` PDF/text chunking, `embedder.py` sentence-transformers wrapper, `store.py` ChromaDB + SQLite persistence) that every tool module calls into.
- State lives entirely in `pool/` (gitignored): ChromaDB vectors at `pool/chroma/`, a SQLite `metadata.db` for the part registry and query log, and `pool/exports/` for `ds_export` output.
- All tunables (`POOL_DIR`, `EMBED_MODEL`, `CHUNK_SIZE`, `CHUNK_OVERLAP`) come from `.env`/environment via `config.py` — never hardcode these in tool code.
- `ds_add_part` auto-fetches datasheets by guessing vendor URL patterns (TI/ST/NXP/Bosch); on failure it reports every URL it tried rather than failing silently.
- `ds_code` builds an SDK-aware code-generation prompt: it retrieves relevant datasheet chunks via vector search, then (if `sdk_path` is given) scores candidate SDK source files with `tools/codegen.py`'s keyword + part-family heuristics before including them as context.
