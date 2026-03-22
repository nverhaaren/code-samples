"""Tests for game lifecycle and session management (GameManager)."""

import pytest

from chess_mcp.game import GameManager
from chess_mcp.types import GameError, GameState, SessionRole


INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


@pytest.fixture
async def gm(engine_path: str) -> GameManager:
    """Create a GameManager with a running engine."""
    manager = GameManager(engine_path)
    await manager.start()
    yield manager  # type: ignore[misc]
    await manager.stop()


# ============================================================================
# State transitions
# ============================================================================


async def test_initial_state_is_no_game(gm: GameManager) -> None:
    assert gm.state == GameState.NO_GAME


async def test_create_game_transitions_to_awaiting(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    assert gm.state == GameState.AWAITING_PLAYERS


async def test_two_joins_transition_to_ongoing(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    assert gm.state == GameState.ONGOING


async def test_create_in_ongoing_raises(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    with pytest.raises(GameError):
        await gm.create_game(session_id="s1")


# ============================================================================
# Sessions
# ============================================================================


async def test_join_white(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    assert gm.get_session_role("s1") == SessionRole.WHITE


async def test_join_black(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="black")
    assert gm.get_session_role("s1") == SessionRole.BLACK


async def test_join_random(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="random")
    role = gm.get_session_role("s1")
    assert role in (SessionRole.WHITE, SessionRole.BLACK)


async def test_join_spectator(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="spectator")
    assert gm.get_session_role("s1") == SessionRole.SPECTATOR


async def test_color_taken_raises(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    with pytest.raises(GameError, match="taken"):
        await gm.join_game(session_id="s2", color="white")


async def test_already_joined_raises(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    with pytest.raises(GameError, match="already joined"):
        await gm.join_game(session_id="s1", color="black")


# ============================================================================
# Permissions
# ============================================================================


async def test_spectator_cannot_make_move(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    await gm.join_game(session_id="spec", color="spectator")
    with pytest.raises(GameError, match="spectator"):
        await gm.make_move(session_id="spec", move="e4")


async def test_unjoined_cannot_query(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    with pytest.raises(GameError, match="not joined"):
        gm.get_status(session_id="unknown")


# ============================================================================
# Queries
# ============================================================================


async def test_get_board_returns_fen(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    board = gm.get_board(session_id="s1")
    assert board == INITIAL_FEN


async def test_get_status_has_all_fields(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    status = gm.get_status(session_id="s1")
    assert status.state == GameState.ONGOING
    assert status.fen == INITIAL_FEN
    assert status.turn == "white"
    assert status.in_check is False
    assert status.your_color == "white"
    assert status.result is None
    assert status.draw_offered is False


async def test_legal_moves_sorted(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    moves = gm.get_legal_moves(session_id="s1")
    assert moves == sorted(moves)
    assert len(moves) == 20  # initial position has 20 legal moves


async def test_get_history_empty_initially(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    history = gm.get_history(session_id="s1")
    assert history == []


# ============================================================================
# Done command
# ============================================================================


async def test_done_in_game_over(gm: GameManager) -> None:
    """done() works when game is over (returns result info)."""
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    # Play fool's mate
    await gm.make_move(session_id="s1", move="f3")
    await gm.make_move(session_id="s2", move="e5")
    await gm.make_move(session_id="s1", move="g4")
    await gm.make_move(session_id="s2", move="Qh4")
    assert gm.state == GameState.GAME_OVER
    result = gm.done(session_id="s1")
    assert result is not None


async def test_done_during_ongoing_raises(gm: GameManager) -> None:
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    with pytest.raises(GameError, match="game is not over"):
        gm.done(session_id="s1")


# ============================================================================
# Create with FEN
# ============================================================================


async def test_create_with_fen(gm: GameManager) -> None:
    fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    await gm.create_game(session_id="s1", fen=fen)
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    board = gm.get_board(session_id="s1")
    assert board == fen


async def test_create_with_invalid_fen_raises(gm: GameManager) -> None:
    with pytest.raises(GameError, match="invalid FEN"):
        await gm.create_game(session_id="s1", fen="not valid fen")


# ============================================================================
# State machine: create after game_over resets
# ============================================================================


async def test_create_after_game_over_works(gm: GameManager) -> None:
    """Can create a new game after the previous one ended."""
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    # Play fool's mate
    await gm.make_move(session_id="s1", move="f3")
    await gm.make_move(session_id="s2", move="e5")
    await gm.make_move(session_id="s1", move="g4")
    await gm.make_move(session_id="s2", move="Qh4")
    assert gm.state == GameState.GAME_OVER
    # Create new game
    await gm.create_game(session_id="s1")
    assert gm.state == GameState.AWAITING_PLAYERS
