"""Tests for the ChessEngine async wrapper."""

import pytest

from chess_mcp.engine import ChessEngine
from chess_mcp.types import EngineError


INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


@pytest.fixture
async def engine(engine_path: str) -> ChessEngine:
    """Create and start a ChessEngine, stop it after the test."""
    eng = ChessEngine(engine_path)
    await eng.start()
    yield eng  # type: ignore[misc]
    await eng.stop()


async def test_engine_starts_and_stops(engine_path: str) -> None:
    """Engine starts and stops cleanly without error."""
    eng = ChessEngine(engine_path)
    await eng.start()
    await eng.stop()


async def test_new_game_returns_initial_fen(engine: ChessEngine) -> None:
    state = await engine.new_game()
    assert state.fen == INITIAL_FEN
    assert state.turn == "white"


async def test_make_move_san(engine: ChessEngine) -> None:
    """make_move with SAN 'e4' succeeds."""
    await engine.new_game()
    result = await engine.make_move("e4")
    assert result.state.turn == "black"
    assert result.move_lan == "e2e4"  # unhyphenated


async def test_make_move_lan(engine: ChessEngine) -> None:
    """make_move with LAN 'e2e4' succeeds."""
    await engine.new_game()
    result = await engine.make_move("e2e4")
    assert result.state.turn == "black"
    assert result.move_lan == "e2e4"


async def test_illegal_move_raises(engine: ChessEngine) -> None:
    """Illegal move raises EngineError."""
    await engine.new_game()
    with pytest.raises(EngineError):
        await engine.make_move("e5")  # pawn can't go to e5 from starting pos


async def test_from_fen_valid(engine: ChessEngine) -> None:
    fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    state = await engine.from_fen(fen)
    assert state.fen == fen
    assert state.turn == "black"


async def test_from_fen_invalid_raises(engine: ChessEngine) -> None:
    with pytest.raises(EngineError):
        await engine.from_fen("not a valid fen")


async def test_parse_san_nf3(engine: ChessEngine) -> None:
    """parse_san('Nf3') returns 'g1f3' (normalized, unhyphenated)."""
    await engine.new_game()
    result = await engine.parse_san("Nf3")
    assert result == "g1f3"


async def test_parse_san_invalid_returns_none(engine: ChessEngine) -> None:
    await engine.new_game()
    result = await engine.parse_san("Zz9")
    assert result is None


async def test_fools_mate_checkmate(engine: ChessEngine) -> None:
    """Fool's mate sequence reaches is_checkmate."""
    await engine.new_game()
    await engine.make_move("f3")
    await engine.make_move("e5")
    await engine.make_move("g4")
    result = await engine.make_move("Qh4")
    assert result.state.is_checkmate is True


async def test_lan_output_unhyphenated(engine: ChessEngine) -> None:
    """Legal moves and move history use unhyphenated LAN."""
    await engine.new_game()
    result = await engine.make_move("e4")
    # Check that legal_moves are unhyphenated
    for move in result.state.legal_moves:
        assert "-" not in move, f"LAN should be unhyphenated: {move}"
    # Check move_history
    for move in result.state.move_history:
        assert "-" not in move, f"History LAN should be unhyphenated: {move}"
    # Check the returned move_lan
    assert "-" not in result.move_lan


async def test_get_state(engine: ChessEngine) -> None:
    """get_state returns current position."""
    await engine.new_game()
    await engine.make_move("e4")
    state = await engine.get_state()
    assert state.turn == "black"
    assert len(state.move_history) == 1
