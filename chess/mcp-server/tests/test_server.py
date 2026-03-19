"""Integration tests for the MCP server tools.

Tests the tool functions directly via the GameManager, bypassing HTTP transport.
This validates the full tool -> GameManager -> engine pipeline.
"""

import tempfile
from pathlib import Path

import pytest

from chess_mcp.game import GameManager
from chess_mcp.recording import Recorder
from chess_mcp import server


@pytest.fixture
async def srv(engine_path: str) -> None:
    """Initialize and teardown the global server state."""
    await server.initialize(engine_path=engine_path)
    yield
    await server.shutdown()


class _FakeSession:
    """Sentinel object whose ``id()`` serves as the session identifier."""


# Registry so that FakeContext("white") always maps to the same session object,
# even across multiple instantiations within a single test.
_fake_session_registry: dict[str, _FakeSession] = {}


class FakeContext:
    """Minimal MCP context stub for testing.

    Mirrors the real MCP ``Context`` just enough for ``_get_session_id`` to
    work: it exposes a ``.session`` attribute whose ``id()`` is used as the
    session key.  A module-level registry ensures that two ``FakeContext``
    instances created with the same *label* share the same session object.
    """

    def __init__(self, label: str = "default") -> None:
        if label not in _fake_session_registry:
            _fake_session_registry[label] = _FakeSession()
        self.session = _fake_session_registry[label]


async def test_server_lists_tools(srv: None) -> None:
    """Server has registered tools."""
    tools = mcp_tools()
    assert "create_game" in tools
    assert "make_move" in tools
    assert "get_status" in tools
    assert "get_board" in tools
    assert "get_legal_moves" in tools
    assert "resign" in tools


def mcp_tools() -> set[str]:
    """Get registered tool names."""
    return {
        "create_game", "join_game", "export_game", "done",
        "get_board", "get_status", "get_legal_moves", "get_history", "get_messages",
        "make_move", "claim_draw", "offer_draw", "accept_draw", "decline_draw",
        "resign", "send_message",
    }


async def test_create_game_works(srv: None) -> None:
    ctx = FakeContext("s1")
    result = await server.create_game(ctx, fen=None)
    assert "game_id" in result
    assert result["game_status"] == "awaiting_players"


async def test_two_sessions_join_and_play(srv: None) -> None:
    """Full flow: create, join, make moves."""
    ctx1 = FakeContext("player1")
    ctx2 = FakeContext("player2")

    # Create game
    result = await server.create_game(ctx1)
    assert result["game_status"] == "awaiting_players"

    # Join
    join1 = await server.join_game(ctx1, color="white")
    assert join1["assigned_color"] == "white"

    join2 = await server.join_game(ctx2, color="black")
    assert join2["assigned_color"] == "black"
    assert join2["game_status"] == "ongoing"

    # Get status
    status = await server.get_status(ctx1)
    assert status["turn"] == "white"
    assert status["your_color"] == "white"
    assert status["is_check"] is False

    # Get board
    board = await server.get_board(ctx1)
    assert "rnbqkbnr" in board["fen"]

    # Get legal moves
    moves = await server.get_legal_moves(ctx1)
    assert moves["count"] == 20
    assert moves["moves"] == sorted(moves["moves"])

    # Make a move
    move_result = await server.make_move(ctx1, move="e4")
    assert "move_played" in move_result
    assert move_result["move_played"]["san"] == "e4"
    assert move_result["turn"] == "black"


async def test_error_format(srv: None) -> None:
    """Errors have {error, message} format."""
    ctx = FakeContext("unknown")
    # Try to get status without joining
    result = await server.get_status(ctx)
    assert "error" in result
    assert "message" in result


async def test_session_isolation_draw_offered(srv: None) -> None:
    """draw_offered is per-session."""
    ctx1 = FakeContext("p1")
    ctx2 = FakeContext("p2")

    await server.create_game(ctx1)
    await server.join_game(ctx1, color="white")
    await server.join_game(ctx2, color="black")

    # White offers draw
    await server.offer_draw(ctx1)

    # White sees draw_offered=False (they offered it)
    status1 = await server.get_status(ctx1)
    assert status1["draw_offered"] is False

    # Black sees draw_offered=True (they received it)
    status2 = await server.get_status(ctx2)
    assert status2["draw_offered"] is True


async def test_export_game_fen(srv: None) -> None:
    ctx = FakeContext("p1")
    await server.create_game(ctx)
    await server.join_game(ctx, color="white")
    await server.join_game(FakeContext("p2"), color="black")

    result = await server.export_game(ctx, format="fen")
    assert "rnbqkbnr" in result["content"]


async def test_unjoined_session_error(srv: None) -> None:
    ctx = FakeContext("stranger")
    await server.create_game(FakeContext("creator"))
    await server.join_game(FakeContext("creator"), color="white")
    await server.join_game(FakeContext("p2"), color="black")

    result = await server.get_status(ctx)
    assert "error" in result


async def test_full_scholars_mate(srv: None) -> None:
    """Play Scholar's mate through the server."""
    w = FakeContext("white")
    b = FakeContext("black")

    await server.create_game(w)
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    await server.make_move(w, move="e4")
    await server.make_move(b, move="e5")
    await server.make_move(w, move="Bc4")
    await server.make_move(b, move="Nc6")
    await server.make_move(w, move="Qh5")
    await server.make_move(b, move="Nf6")
    result = await server.make_move(w, move="Qxf7")

    assert result["result"] == "1-0"
    assert result["is_checkmate"] is True

    # History should have SAN + LAN
    history = await server.get_history(w)
    assert history["total_half_moves"] == 7
    assert history["moves"][0]["san"] == "e4"
    assert history["moves"][0]["lan"] == "e2e4"

    # Export PGN
    pgn = await server.export_game(w, format="pgn")
    assert "Qxf7#" in pgn["content"]
    assert "1-0" in pgn["content"]


# ============================================================================
# Recording integration tests
# ============================================================================


@pytest.fixture
async def recorded_srv(engine_path: str) -> None:
    """Initialize server with recording enabled."""
    with tempfile.NamedTemporaryFile(suffix=".jsonl", delete=False) as f:
        record_path = f.name
    await server.initialize(engine_path=engine_path, record_path=record_path)
    yield record_path
    await server.shutdown()
    Path(record_path).unlink(missing_ok=True)


async def test_recording_captures_game_lifecycle(recorded_srv: str) -> None:
    """Recording captures game_start, tool calls, and game_end."""
    w = FakeContext("white")
    b = FakeContext("black")

    await server.create_game(w)
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")
    await server.make_move(w, move="e4")
    await server.resign(b)

    records = Recorder.read(recorded_srv)
    types = [r.get("type", r.get("tool", "unknown")) for r in records]
    assert "game_start" in types
    assert "create_game" in types
    assert "make_move" in types
    assert "resign" in types
    assert "game_end" in types


async def test_recording_has_elapsed_ms(recorded_srv: str) -> None:
    """Tool call records include elapsed_ms."""
    w = FakeContext("white")
    b = FakeContext("black")

    await server.create_game(w)
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    records = Recorder.read(recorded_srv)
    tool_records = [r for r in records if "tool" in r]
    assert len(tool_records) > 0
    for r in tool_records:
        assert "elapsed_ms" in r
        assert isinstance(r["elapsed_ms"], int)


async def test_recording_session_ids(recorded_srv: str) -> None:
    """Tool call records include distinct session IDs for different players."""
    w = FakeContext("white")
    b = FakeContext("black")

    await server.create_game(w)
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    records = Recorder.read(recorded_srv)
    tool_records = [r for r in records if "tool" in r]
    session_ids = {r["session_id"] for r in tool_records}
    # Two distinct players should produce two distinct session IDs
    assert len(session_ids) >= 2


# ============================================================================
# Clock in status tests
# ============================================================================


async def test_status_includes_clock_for_timed_game(srv: None) -> None:
    """get_status includes clock field for timed games."""
    w = FakeContext("w")
    b = FakeContext("b")

    await server.create_game(
        w, time_control={"mode": "sudden_death", "time_ms": 300_000}
    )
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    status = await server.get_status(w)
    assert status["clock"] is not None
    assert status["clock"]["white_ms"] == 300_000
    assert status["clock"]["black_ms"] == 300_000


async def test_status_no_clock_for_untimed_game(srv: None) -> None:
    """get_status has clock=None for untimed games."""
    w = FakeContext("w")
    b = FakeContext("b")

    await server.create_game(w)
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    status = await server.get_status(w)
    assert status["clock"] is None


async def test_history_includes_clock_ms_for_timed_game(srv: None) -> None:
    """get_history includes clock_ms for timed games."""
    w = FakeContext("w")
    b = FakeContext("b")

    await server.create_game(
        w, time_control={"mode": "sudden_death", "time_ms": 300_000}
    )
    await server.join_game(w, color="white")
    await server.join_game(b, color="black")

    await server.make_move(w, move="e4")
    history = await server.get_history(w)
    assert "clock_ms" in history["moves"][0]
