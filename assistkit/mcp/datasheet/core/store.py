import sqlite3
import uuid
from datetime import datetime, timezone
from pathlib import Path

import chromadb

from config import POOL_DIR


# ── ChromaDB ─────────────────────────────────────────────────────────────────

def _chroma_client() -> chromadb.PersistentClient:
    path = POOL_DIR / "chroma"
    path.mkdir(parents=True, exist_ok=True)
    return chromadb.PersistentClient(path=str(path))


def get_collection():
    return _chroma_client().get_or_create_collection(
        "datasheets",
        metadata={"hnsw:space": "cosine"},
    )


# ── SQLite ────────────────────────────────────────────────────────────────────

def _db_path() -> Path:
    POOL_DIR.mkdir(parents=True, exist_ok=True)
    return POOL_DIR / "metadata.db"


def get_conn() -> sqlite3.Connection:
    conn = sqlite3.connect(str(_db_path()))
    conn.row_factory = sqlite3.Row
    _init_schema(conn)
    return conn


def _init_schema(conn: sqlite3.Connection) -> None:
    conn.executescript("""
        CREATE TABLE IF NOT EXISTS parts (
            id         TEXT PRIMARY KEY,
            part       TEXT NOT NULL,
            source     TEXT,
            date_added TEXT NOT NULL,
            page_count INTEGER,
            chunk_count INTEGER
        );
        CREATE TABLE IF NOT EXISTS query_log (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT    NOT NULL,
            tool      TEXT    NOT NULL,
            part      TEXT,
            query     TEXT    NOT NULL,
            result    TEXT
        );
    """)
    conn.commit()


# ── Part registry ─────────────────────────────────────────────────────────────

def register_part(part: str, source: str, page_count: int, chunk_count: int) -> str:
    """Insert or replace a part record; return its id."""
    conn = get_conn()
    existing = conn.execute("SELECT id FROM parts WHERE part = ?", (part,)).fetchone()
    part_id = existing["id"] if existing else str(uuid.uuid4())
    conn.execute(
        """INSERT OR REPLACE INTO parts (id, part, source, date_added, page_count, chunk_count)
           VALUES (?, ?, ?, ?, ?, ?)""",
        (part_id, part, source, datetime.now(timezone.utc).isoformat(), page_count, chunk_count),
    )
    conn.commit()
    conn.close()
    return part_id


def list_parts() -> list[sqlite3.Row]:
    conn = get_conn()
    rows = conn.execute("SELECT * FROM parts ORDER BY date_added DESC").fetchall()
    conn.close()
    return rows


def get_part(part: str) -> sqlite3.Row | None:
    conn = get_conn()
    row = conn.execute("SELECT * FROM parts WHERE part = ?", (part,)).fetchone()
    conn.close()
    return row


def delete_part(part: str) -> bool:
    """Remove part from SQLite and ChromaDB. Return True if it existed."""
    conn = get_conn()
    row = conn.execute("SELECT id FROM parts WHERE part = ?", (part,)).fetchone()
    if not row:
        conn.close()
        return False
    part_id = row["id"]
    conn.execute("DELETE FROM parts WHERE id = ?", (part_id,))
    conn.commit()
    conn.close()

    col = get_collection()
    col.delete(where={"part_id": {"$eq": part_id}})
    return True


def clear_all_parts() -> int:
    conn = get_conn()
    count = conn.execute("SELECT COUNT(*) FROM parts").fetchone()[0]
    conn.execute("DELETE FROM parts")
    conn.commit()
    conn.close()

    client = _chroma_client()
    client.delete_collection("datasheets")
    return count


# ── Vector upsert / query ─────────────────────────────────────────────────────

def upsert_chunks(part_id: str, part: str, chunks: list[dict], embeddings: list[list[float]]) -> None:
    col = get_collection()
    ids = [f"{part_id}_{c['chunk_index']}" for c in chunks]
    metadatas = [
        {"part_id": part_id, "part": part, "page": c.get("page") or 0, "chunk_index": c["chunk_index"]}
        for c in chunks
    ]
    documents = [c["text"] for c in chunks]
    col.upsert(ids=ids, embeddings=embeddings, documents=documents, metadatas=metadatas)


def query_chunks(embedding: list[float], n_results: int = 5, part: str | None = None) -> list[dict]:
    col = get_collection()
    where = {"part": {"$eq": part}} if part else None
    kwargs: dict = {"query_embeddings": [embedding], "n_results": n_results, "include": ["documents", "metadatas", "distances"]}
    if where:
        kwargs["where"] = where
    results = col.query(**kwargs)
    hits = []
    for doc, meta, dist in zip(results["documents"][0], results["metadatas"][0], results["distances"][0]):
        hits.append({"text": doc, "part": meta["part"], "page": meta.get("page"), "score": 1 - dist})
    return hits


# ── Query log ─────────────────────────────────────────────────────────────────

def log_query(tool: str, query: str, result: str, part: str | None = None) -> None:
    conn = get_conn()
    conn.execute(
        "INSERT INTO query_log (timestamp, tool, part, query, result) VALUES (?, ?, ?, ?, ?)",
        (datetime.now(timezone.utc).isoformat(), tool, part, query, result[:500]),
    )
    conn.commit()
    conn.close()


def get_history(part: str | None = None, tool: str | None = None, limit: int = 50) -> list[sqlite3.Row]:
    conn = get_conn()
    clauses, params = [], []
    if part:
        clauses.append("part = ?"); params.append(part)
    if tool:
        clauses.append("tool = ?"); params.append(tool)
    where = ("WHERE " + " AND ".join(clauses)) if clauses else ""
    rows = conn.execute(f"SELECT * FROM query_log {where} ORDER BY id DESC LIMIT ?", (*params, limit)).fetchall()
    conn.close()
    return rows


def clear_history() -> int:
    conn = get_conn()
    count = conn.execute("SELECT COUNT(*) FROM query_log").fetchone()[0]
    conn.execute("DELETE FROM query_log")
    conn.commit()
    conn.close()
    return count


def get_last_result() -> sqlite3.Row | None:
    conn = get_conn()
    row = conn.execute("SELECT * FROM query_log ORDER BY id DESC LIMIT 1").fetchone()
    conn.close()
    return row
