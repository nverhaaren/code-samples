"""Tests for chess clock integration with GameManager."""

import pytest

from chess_mcp.game import GameManager
from chess_mcp.types import GameState


@pytest.fixture
async def timed_game(engine_path: str) -> GameManager:
    """GameManager with a timed game (5 min sudden death)."""
    gm = GameManager(engine_path)
    await gm.start()
    await gm.create_game(
        session_id="s1",
        time_control={"mode": "sudden_death", "time_ms": 300_000},
    )
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    yield gm  # type: ignore[misc]
    await gm.stop()


async def test_timed_game_creation(timed_game: GameManager) -> None:
    """Timed game is created with clock state in status."""
    status = timed_game.get_status(session_id="s1")
    assert status.clock is not None
    assert status.clock["white_ms"] == 300_000
    assert status.clock["black_ms"] == 300_000


async def test_make_move_updates_clock(timed_game: GameManager) -> None:
    """Clock is updated after each move."""
    record = await timed_game.make_move(session_id="s1", move="e4")
    assert record.clock_ms is not None
    # First white move: clock wasn't running, so no deduction
    assert record.clock_ms == 300_000


async def test_history_includes_clock_ms(timed_game: GameManager) -> None:
    """Move history records include clock_ms for timed games."""
    await timed_game.make_move(session_id="s1", move="e4")
    await timed_game.make_move(session_id="s2", move="e5")
    history = timed_game.get_history(session_id="s1")
    assert all(r.clock_ms is not None for r in history)


async def test_untimed_game_no_clock(engine_path: str) -> None:
    """Untimed game has no clock in status."""
    gm = GameManager(engine_path)
    await gm.start()
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    status = gm.get_status(session_id="s1")
    assert status.clock is None
    record = await gm.make_move(session_id="s1", move="e4")
    assert record.clock_ms is None
    await gm.stop()


async def test_fischer_timed_game(engine_path: str) -> None:
    """Fischer time control adds increment after each move."""
    gm = GameManager(engine_path)
    await gm.start()
    await gm.create_game(
        session_id="s1",
        time_control={"mode": "fischer", "time_ms": 300_000, "increment_ms": 5_000},
    )
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    record = await gm.make_move(session_id="s1", move="e4")
    # First move: no deduction + 5s increment
    assert record.clock_ms == 305_000
    await gm.stop()


async def test_pgn_has_clock_annotations(timed_game: GameManager) -> None:
    """PGN export includes clock annotations for timed games."""
    await timed_game.make_move(session_id="s1", move="e4")
    await timed_game.make_move(session_id="s2", move="e5")
    pgn = timed_game.export_game(format="pgn")
    assert "%clk" in pgn


async def test_pgn_no_clock_annotations_untimed(engine_path: str) -> None:
    """PGN export has no clock annotations for untimed games."""
    gm = GameManager(engine_path)
    await gm.start()
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    await gm.make_move(session_id="s1", move="e4")
    pgn = gm.export_game(format="pgn")
    assert "%clk" not in pgn
    await gm.stop()
