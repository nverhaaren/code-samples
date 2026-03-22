"""Tests for recording and replay."""

import json
import tempfile
from pathlib import Path

from chess_mcp.recording import Recorder


def test_game_start_marker() -> None:
    rec = Recorder()
    rec.record_game_start("game-1", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
    records = rec.get_records()
    assert len(records) == 1
    assert records[0]["type"] == "game_start"
    assert records[0]["game_id"] == "game-1"


def test_game_end_marker() -> None:
    rec = Recorder()
    rec.record_game_end("game-1", "1-0", "checkmate")
    records = rec.get_records()
    assert len(records) == 1
    assert records[0]["type"] == "game_end"
    assert records[0]["result"] == "1-0"


def test_tool_call_fields() -> None:
    rec = Recorder()
    rec.record_tool_call(
        session_id="s1",
        tool="make_move",
        params={"move": "e4"},
        result={"ok": True},
        elapsed_ms=42,
    )
    records = rec.get_records()
    assert len(records) == 1
    r = records[0]
    assert r["session_id"] == "s1"
    assert r["tool"] == "make_move"
    assert r["params"] == {"move": "e4"}
    assert r["result"] == {"ok": True}
    assert r["elapsed_ms"] == 42
    assert "ts_ms" in r


def test_read_roundtrip() -> None:
    with tempfile.NamedTemporaryFile(mode="w", suffix=".jsonl", delete=False) as f:
        path = f.name

    rec = Recorder(path=path)
    rec.record_game_start("game-1", "initial")
    rec.record_tool_call("s1", "make_move", {"move": "e4"}, {"ok": True}, 10)
    rec.record_game_end("game-1", "1-0", "checkmate")

    # Read back
    records = Recorder.read(path)
    assert len(records) == 3
    assert records[0]["type"] == "game_start"
    assert records[1]["tool"] == "make_move"
    assert records[2]["type"] == "game_end"

    Path(path).unlink()


def test_timestamps_present() -> None:
    rec = Recorder()
    rec.record_tool_call("s1", "get_status", {}, {}, 5)
    records = rec.get_records()
    assert "ts_ms" in records[0]
    assert isinstance(records[0]["ts_ms"], int)


def test_seeded_elapsed_zero() -> None:
    rec = Recorder(seeded=True)
    rec.record_tool_call("s1", "make_move", {"move": "e4"}, {}, 999)
    records = rec.get_records()
    assert records[0]["elapsed_ms"] == 0
    assert records[0]["ts_ms"] == 0


def test_session_id_recorded() -> None:
    rec = Recorder()
    rec.record_tool_call("player-1", "make_move", {}, {}, 0)
    rec.record_tool_call("player-2", "make_move", {}, {}, 0)
    records = rec.get_records()
    assert records[0]["session_id"] == "player-1"
    assert records[1]["session_id"] == "player-2"
