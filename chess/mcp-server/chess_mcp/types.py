"""Shared data types for the chess MCP server."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum


@dataclass(frozen=True)
class EngineState:
    """Mirrors the JSON state returned by the C++ engine's toJson()."""

    fen: str
    turn: str  # "white" or "black"
    legal_moves: list[str]  # LAN strings (unhyphenated, e.g. "e2e4")
    in_check: bool
    is_checkmate: bool
    is_stalemate: bool
    can_claim_draw: bool
    is_automatic_draw: bool
    halfmove_clock: int
    fullmove_number: int
    move_history: list[str]  # LAN strings (unhyphenated)


@dataclass(frozen=True)
class MoveResult:
    """Result of a successful make_move call."""

    state: EngineState
    move_lan: str  # unhyphenated LAN of the move made


class EngineError(Exception):
    """Raised when the engine returns an error response."""

    pass


class GameState(str, Enum):
    """Game lifecycle states."""

    NO_GAME = "no_game"
    AWAITING_PLAYERS = "awaiting_players"
    ONGOING = "ongoing"
    GAME_OVER = "game_over"


class SessionRole(str, Enum):
    """Role a session can have in the game."""

    WHITE = "white"
    BLACK = "black"
    SPECTATOR = "spectator"
    UNJOINED = "unjoined"


@dataclass
class GameStatus:
    """Full game status returned by get_status."""

    state: GameState
    fen: str
    turn: str
    in_check: bool
    is_checkmate: bool
    is_stalemate: bool
    can_claim_draw: bool
    is_automatic_draw: bool
    halfmove_clock: int
    fullmove_number: int
    result: str | None  # e.g. "1-0", "0-1", "1/2-1/2", None if ongoing
    termination_reason: str | None
    your_color: str | None  # the querying session's color
    draw_offered: bool  # whether a draw offer is pending
    clock: dict[str, int] | None = None  # {white_ms, black_ms} or None if untimed


@dataclass
class MoveRecord:
    """A single move in the game history."""

    move_number: int
    color: str  # "white" or "black"
    san: str
    lan: str
    clock_ms: int | None = None  # remaining time after move (None if untimed)


class GameError(Exception):
    """Raised when a game operation is invalid."""

    pass
