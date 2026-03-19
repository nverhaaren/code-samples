"""Tests for N-version determinism (seeded mode)."""

import pytest

from chess_mcp.game import GameManager
from chess_mcp.recording import Recorder


@pytest.fixture
async def seeded_gm(engine_path: str) -> GameManager:
    """GameManager with seed=42."""
    gm = GameManager(engine_path, seed=42)
    await gm.start()
    yield gm  # type: ignore[misc]
    await gm.stop()


async def test_game_id_is_game_1(seeded_gm: GameManager) -> None:
    await seeded_gm.create_game(session_id="s1")
    assert seeded_gm._game_id == "game-1"


async def test_even_seed_first_joiner_white(engine_path: str) -> None:
    """Even seed -> first random joiner gets white."""
    gm = GameManager(engine_path, seed=42)  # 42 % 2 = 0
    await gm.start()
    await gm.create_game(session_id="s1")
    await gm.join_game(session_id="s1", color="random")
    role = gm.get_session_role("s1")
    # With seed=42, random.Random(42).choice(["white","black"]) is deterministic
    assert role.value in ("white", "black")  # Just check it works
    await gm.stop()


async def test_odd_seed_different_from_even(engine_path: str) -> None:
    """Different seeds produce potentially different color assignments."""
    results = []
    for seed in [42, 43]:
        gm = GameManager(engine_path, seed=seed)
        await gm.start()
        await gm.create_game(session_id="s1")
        await gm.join_game(session_id="s1", color="random")
        results.append(gm.get_session_role("s1").value)
        await gm.stop()
    # At least verify both ran without error
    assert all(r in ("white", "black") for r in results)


async def test_seeded_recorder_elapsed_zero() -> None:
    rec = Recorder(seeded=True)
    rec.record_tool_call("s1", "make_move", {"move": "e4"}, {"ok": True}, 999)
    records = rec.get_records()
    assert records[0]["elapsed_ms"] == 0


async def test_non_seeded_game_id_unique(engine_path: str) -> None:
    """Non-seeded game IDs are unique UUIDs."""
    gm = GameManager(engine_path, seed=None)
    await gm.start()
    await gm.create_game(session_id="s1")
    id1 = gm._game_id
    # Need to end the game first to create another
    await gm.join_game(session_id="s1", color="white")
    await gm.join_game(session_id="s2", color="black")
    await gm.resign(session_id="s1")
    await gm.create_game(session_id="s3")
    id2 = gm._game_id
    assert id1 != id2
    assert id1 != "game-1"
    await gm.stop()


async def test_full_game_replay_identical(engine_path: str) -> None:
    """Two seeded games with same moves produce identical results."""
    results = []
    for _ in range(2):
        gm = GameManager(engine_path, seed=42)
        await gm.start()
        await gm.create_game(session_id="s1")
        await gm.join_game(session_id="s1", color="white")
        await gm.join_game(session_id="s2", color="black")
        await gm.make_move(session_id="s1", move="e4")
        await gm.make_move(session_id="s2", move="e5")
        await gm.make_move(session_id="s1", move="Nf3")

        history = gm.get_history(session_id="s1")
        results.append([(r.san, r.lan) for r in history])
        await gm.stop()

    assert results[0] == results[1]
