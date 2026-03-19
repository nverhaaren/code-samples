"""MCP server for the chess engine.

Registers all chess tools and delegates to GameManager.
Uses FastMCP with streamable HTTP transport.
"""

from __future__ import annotations

import os
import pathlib
import time
from typing import Any

from mcp.server.fastmcp import Context, FastMCP

from .game import GameManager
from .recording import Recorder
from .types import GameError, GameState

# Create the MCP server
mcp = FastMCP("Chess MCP Server")

# Global GameManager instance — initialized in lifespan
_game_manager: GameManager | None = None
_recorder: Recorder | None = None


def _find_engine_binary() -> str:
    """Locate the chess engine binary."""
    env_path = os.environ.get("CHESS_ENGINE_PATH")
    if env_path:
        return env_path
    base = pathlib.Path(__file__).resolve().parent.parent.parent
    candidates = [
        base / "build" / "chess",
        base / "build" / "Debug" / "chess",
        base / "build" / "Release" / "chess",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return str(candidate)
    raise FileNotFoundError("Chess engine binary not found. Set CHESS_ENGINE_PATH.")


def _get_session_id(ctx: Context) -> str:
    """Extract session ID from MCP context."""
    # Use the MCP session ID if available, fall back to a default
    session_id = getattr(ctx, "session_id", None)
    if session_id:
        return str(session_id)
    # Try to get from request context
    request_context = getattr(ctx, "request_context", None)
    if request_context:
        meta = getattr(request_context, "meta", None)
        if meta:
            session_id = getattr(meta, "sessionId", None)
            if session_id:
                return str(session_id)
    return "default"


def _gm() -> GameManager:
    """Get the global GameManager, raising if not initialized."""
    if _game_manager is None:
        raise RuntimeError("GameManager not initialized")
    return _game_manager


def _error_response(code: str, message: str) -> dict[str, Any]:
    """Create a standard error response."""
    return {"error": code, "message": message}


def _record(
    session_id: str,
    tool: str,
    params: dict[str, Any],
    result: dict[str, Any],
    elapsed_ms: int,
) -> None:
    """Record a tool call if recording is active."""
    if _recorder is not None:
        _recorder.record_tool_call(session_id, tool, params, result, elapsed_ms)


# ============================================================================
# Game Management Tools
# ============================================================================


@mcp.tool()
async def create_game(
    ctx: Context,
    fen: str | None = None,
    time_control: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Create a new chess game. Optionally provide a FEN starting position and time control."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().create_game(session_id=session_id, fen=fen, time_control=time_control)
        result = {
            "game_id": _gm()._game_id,
            "fen": _gm()._engine_state.fen if _gm()._engine_state else None,
            "game_status": "awaiting_players",
        }
        if _recorder is not None:
            _recorder.record_game_start(
                _gm()._game_id,
                _gm()._engine_state.fen if _gm()._engine_state else "unknown",
            )
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "create_game", {"fen": fen, "time_control": time_control}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("wrong_state", str(e))


@mcp.tool()
async def join_game(
    ctx: Context,
    color: str,
) -> dict[str, Any]:
    """Join the current game. Color: 'white', 'black', 'random', or 'spectator'."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().join_game(session_id=session_id, color=color)
        role = _gm().get_session_role(session_id)
        result = {
            "assigned_color": role.value,
            "game_status": _gm().state.value,
        }
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "join_game", {"color": color}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("join_error", str(e))


@mcp.tool()
async def export_game(
    ctx: Context,
    format: str = "pgn",
) -> dict[str, Any]:
    """Export the game in FEN or PGN format."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        content = _gm().export_game(format=format)
        result = {"content": content}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "export_game", {"format": format}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("no_active_game", str(e))


@mcp.tool()
async def done(ctx: Context) -> dict[str, Any]:
    """Signal that this client is finished with the game. Only valid after game over."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        result_data = _gm().done(session_id=session_id)
        result = {"acknowledged": True, **result_data}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "done", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("game_not_over", str(e))


# ============================================================================
# Query Tools
# ============================================================================


@mcp.tool()
async def get_board(ctx: Context) -> dict[str, Any]:
    """Get the current board position as a FEN string."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        fen = _gm().get_board(session_id=session_id)
        result = {"fen": fen}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "get_board", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("not_joined", str(e))


@mcp.tool()
async def get_status(ctx: Context) -> dict[str, Any]:
    """Get the current game status including turn, check, draw info."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        status = _gm().get_status(session_id=session_id)
        last_move = None
        history = _gm().get_history(session_id=session_id)
        if history:
            last = history[-1]
            last_move = {"san": last.san, "lan": last.lan}
        result = {
            "server_state": status.state.value,
            "turn": status.turn,
            "fen": status.fen,
            "fullmove_number": status.fullmove_number,
            "halfmove_clock": status.halfmove_clock,
            "is_check": status.in_check,
            "is_checkmate": status.is_checkmate,
            "is_stalemate": status.is_stalemate,
            "can_claim_draw": status.can_claim_draw,
            "is_automatic_draw": status.is_automatic_draw,
            "draw_offered": status.draw_offered,
            "your_color": status.your_color,
            "last_move": last_move,
            "result": status.result,
            "termination_reason": status.termination_reason,
            "clock": status.clock,
        }
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "get_status", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("not_joined", str(e))


@mcp.tool()
async def get_legal_moves(ctx: Context) -> dict[str, Any]:
    """Get all legal moves for the side to move, sorted by LAN."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        moves = _gm().get_legal_moves(session_id=session_id)
        result = {"moves": moves, "count": len(moves)}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "get_legal_moves", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("not_joined", str(e))


@mcp.tool()
async def get_history(ctx: Context) -> dict[str, Any]:
    """Get the move history."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        history = _gm().get_history(session_id=session_id)
        moves = []
        for record in history:
            move_dict: dict[str, Any] = {
                "move_number": record.move_number,
                "color": record.color,
                "san": record.san,
                "lan": record.lan,
            }
            if record.clock_ms is not None:
                move_dict["clock_ms"] = record.clock_ms
            moves.append(move_dict)
        result = {"moves": moves, "total_half_moves": len(moves)}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "get_history", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("not_joined", str(e))


@mcp.tool()
async def get_messages(ctx: Context, clear: bool = True) -> dict[str, Any]:
    """Get messages sent to this player. Not available to spectators."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        messages = _gm().get_messages(session_id=session_id, clear=clear)
        result = {"messages": messages}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "get_messages", {"clear": clear}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("not_joined", str(e))


# ============================================================================
# Action Tools
# ============================================================================


@mcp.tool()
async def make_move(ctx: Context, move: str) -> dict[str, Any]:
    """Make a chess move. Accepts SAN (e.g. 'Nf3') or LAN (e.g. 'g1f3')."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        record = await _gm().make_move(session_id=session_id, move=move)
        # Return status + move played
        status_dict = await get_status(ctx)
        move_played: dict[str, Any] = {"san": record.san, "lan": record.lan}
        if record.clock_ms is not None:
            move_played["clock_ms"] = record.clock_ms
        status_dict["move_played"] = move_played
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "make_move", {"move": move}, status_dict, elapsed)
        # Record game end if game is over
        if _recorder is not None and _gm().state == GameState.GAME_OVER:
            _recorder.record_game_end(
                _gm()._game_id,
                status_dict.get("result", "*"),
                status_dict.get("termination_reason", "unknown"),
            )
        return status_dict
    except GameError as e:
        return _error_response("move_error", str(e))
    except Exception as e:
        return _error_response("illegal_move", str(e))


@mcp.tool()
async def claim_draw(ctx: Context) -> dict[str, Any]:
    """Claim a draw under the fifty-move rule or threefold repetition. Turn-gated."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().claim_draw(session_id=session_id)
        result = await get_status(ctx)
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "claim_draw", {}, result, elapsed)
        if _recorder is not None:
            _recorder.record_game_end(
                _gm()._game_id,
                result.get("result", "1/2-1/2"),
                result.get("termination_reason", "draw_claim"),
            )
        return result
    except GameError as e:
        return _error_response("draw_error", str(e))


@mcp.tool()
async def offer_draw(ctx: Context) -> dict[str, Any]:
    """Offer a draw to your opponent."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().offer_draw(session_id=session_id)
        result = {"offered": True}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "offer_draw", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("draw_error", str(e))


@mcp.tool()
async def accept_draw(ctx: Context) -> dict[str, Any]:
    """Accept a pending draw offer."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().accept_draw(session_id=session_id)
        result = await get_status(ctx)
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "accept_draw", {}, result, elapsed)
        if _recorder is not None:
            _recorder.record_game_end(
                _gm()._game_id,
                result.get("result", "1/2-1/2"),
                result.get("termination_reason", "draw_agreement"),
            )
        return result
    except GameError as e:
        return _error_response("draw_error", str(e))


@mcp.tool()
async def decline_draw(ctx: Context) -> dict[str, Any]:
    """Decline a pending draw offer."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().decline_draw(session_id=session_id)
        result = {"declined": True}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "decline_draw", {}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("draw_error", str(e))


@mcp.tool()
async def resign(ctx: Context) -> dict[str, Any]:
    """Resign the game."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        await _gm().resign(session_id=session_id)
        result = await get_status(ctx)
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "resign", {}, result, elapsed)
        if _recorder is not None:
            _recorder.record_game_end(
                _gm()._game_id,
                result.get("result", "*"),
                result.get("termination_reason", "resignation"),
            )
        return result
    except GameError as e:
        return _error_response("resign_error", str(e))


@mcp.tool()
async def send_message(ctx: Context, text: str) -> dict[str, Any]:
    """Send a message to your opponent."""
    t0 = time.monotonic_ns()
    try:
        session_id = _get_session_id(ctx)
        _gm().send_message(session_id=session_id, text=text)
        result = {"sent": True}
        elapsed = (time.monotonic_ns() - t0) // 1_000_000
        _record(session_id, "send_message", {"text": text}, result, elapsed)
        return result
    except GameError as e:
        return _error_response("message_error", str(e))


async def initialize(
    engine_path: str | None = None,
    seed: int | None = None,
    record_path: str | None = None,
) -> None:
    """Initialize the game manager and optional recorder."""
    global _game_manager, _recorder
    path = engine_path or _find_engine_binary()
    _game_manager = GameManager(path, seed=seed)
    await _game_manager.start()
    if record_path is not None:
        _recorder = Recorder(path=record_path, seeded=(seed is not None))


async def shutdown() -> None:
    """Shutdown the game manager."""
    global _game_manager, _recorder
    if _game_manager:
        await _game_manager.stop()
        _game_manager = None
    _recorder = None
