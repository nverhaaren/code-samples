"""Game lifecycle management and session handling for the chess MCP server."""

from __future__ import annotations

import random
import time
import uuid
from dataclasses import dataclass, field

from typing import Any

from .clock import ChessClock, create_clock
from .engine import ChessEngine
from .material import is_insufficient_material
from .notation import lan_to_san
from .pgn import generate_pgn
from .types import (
    EngineState,
    GameError,
    GameState,
    GameStatus,
    MoveRecord,
    MoveResult,
    SessionRole,
)


@dataclass
class Session:
    """Tracks a connected MCP session's role and state."""

    session_id: str
    role: SessionRole = SessionRole.UNJOINED
    messages: list[dict[str, str | int]] = field(default_factory=list)


class GameManager:
    """Central orchestrator between MCP tools and the chess engine.

    State machine: no_game -> awaiting_players -> ongoing -> game_over
    """

    def __init__(self, engine_path: str, seed: int | None = None) -> None:
        self._engine_path = engine_path
        self._engine: ChessEngine | None = None
        self._seed = seed
        self._rng = random.Random(seed) if seed is not None else random.Random()

        # Game state
        self._state = GameState.NO_GAME
        self._sessions: dict[str, Session] = {}
        self._white_session: str | None = None
        self._black_session: str | None = None
        self._engine_state: EngineState | None = None

        # Game data
        self._game_id: str | None = None
        self._move_history: list[MoveRecord] = []
        self._position_history: list[str] = []  # FEN position keys for repetition
        self._result: str | None = None
        self._termination_reason: str | None = None
        self._draw_offered_by: str | None = None  # session_id of draw offerer
        self._last_activity: float = 0.0
        self._clock: ChessClock | None = None
        self._time_control: dict[str, Any] | None = None
        self._clock_times: list[int | None] = []  # remaining ms after each half-move

    @property
    def state(self) -> GameState:
        return self._state

    async def start(self) -> None:
        """Start the engine subprocess."""
        self._engine = ChessEngine(self._engine_path)
        await self._engine.start()

    async def stop(self) -> None:
        """Stop the engine subprocess."""
        if self._engine:
            await self._engine.stop()
            self._engine = None

    def _get_or_create_session(self, session_id: str) -> Session:
        if session_id not in self._sessions:
            self._sessions[session_id] = Session(session_id=session_id)
        return self._sessions[session_id]

    def _require_session(self, session_id: str) -> Session:
        """Get session, raising if not joined."""
        session = self._sessions.get(session_id)
        if session is None or session.role == SessionRole.UNJOINED:
            raise GameError("not joined: session has not joined the game")
        return session

    def _require_player(self, session_id: str) -> Session:
        """Get session, raising if not a player (spectators/unjoined blocked)."""
        session = self._require_session(session_id)
        if session.role == SessionRole.SPECTATOR:
            raise GameError("spectator: spectators cannot perform this action")
        return session

    def _require_ongoing(self) -> None:
        if self._state != GameState.ONGOING:
            raise GameError("game is not ongoing")

    def _fen_position_key(self, fen: str) -> str:
        """Extract the position key (first 4 FEN fields) for repetition detection."""
        parts = fen.split()
        return " ".join(parts[:4])

    def _record_position(self) -> None:
        """Record current position for repetition detection."""
        if self._engine_state:
            self._position_history.append(
                self._fen_position_key(self._engine_state.fen)
            )

    def _position_count(self) -> int:
        """Count occurrences of the current position."""
        if not self._engine_state:
            return 0
        key = self._fen_position_key(self._engine_state.fen)
        return self._position_history.count(key)

    # ========================================================================
    # Game lifecycle
    # ========================================================================

    async def create_game(
        self,
        session_id: str,
        fen: str | None = None,
        time_control: dict[str, Any] | None = None,
    ) -> None:
        """Create a new game. Allowed from no_game or game_over states."""
        if self._state in (GameState.AWAITING_PLAYERS, GameState.ONGOING):
            raise GameError(
                f"cannot create game: game is in state {self._state.value}"
            )

        assert self._engine is not None

        # Reset all game state
        self._sessions.clear()
        self._white_session = None
        self._black_session = None
        self._move_history = []
        self._position_history = []
        self._result = None
        self._termination_reason = None
        self._draw_offered_by = None
        self._last_activity = time.monotonic()
        self._clock_times = []

        # Set up clock if time control is specified
        self._time_control = time_control
        if time_control:
            self._clock = create_clock(
                mode=time_control.get("mode", "sudden_death"),
                time_ms=time_control.get("time_ms", 300_000),
                increment_ms=time_control.get("increment_ms", 0),
                delay_ms=time_control.get("delay_ms", 0),
                periods=time_control.get("periods"),
                seeded=self._seed is not None,
            )
        else:
            self._clock = None

        if self._seed is not None:
            self._game_id = "game-1"
        else:
            self._game_id = str(uuid.uuid4())

        # Initialize engine
        if fen:
            try:
                self._engine_state = await self._engine.from_fen(fen)
            except Exception:
                raise GameError(f"invalid FEN: {fen}")
        else:
            self._engine_state = await self._engine.new_game()

        self._record_position()
        self._state = GameState.AWAITING_PLAYERS

    async def join_game(self, session_id: str, color: str) -> None:
        """Join the current game with a color choice."""
        if self._state not in (GameState.AWAITING_PLAYERS, GameState.ONGOING):
            raise GameError("no game to join")

        session = self._get_or_create_session(session_id)
        if session.role != SessionRole.UNJOINED:
            raise GameError("already joined: session has already joined the game")

        if color == "spectator":
            session.role = SessionRole.SPECTATOR
            return

        if color == "random":
            # Assign random color from available
            available = []
            if self._white_session is None:
                available.append("white")
            if self._black_session is None:
                available.append("black")
            if not available:
                raise GameError("both colors are taken")
            color = self._rng.choice(available)

        if color == "white":
            if self._white_session is not None:
                raise GameError("white is taken")
            self._white_session = session_id
            session.role = SessionRole.WHITE
        elif color == "black":
            if self._black_session is not None:
                raise GameError("black is taken")
            self._black_session = session_id
            session.role = SessionRole.BLACK
        else:
            raise GameError(f"invalid color: {color}")

        # Transition to ongoing when both players have joined
        if self._white_session is not None and self._black_session is not None:
            self._state = GameState.ONGOING

    def get_session_role(self, session_id: str) -> SessionRole:
        session = self._sessions.get(session_id)
        if session is None:
            return SessionRole.UNJOINED
        return session.role

    # ========================================================================
    # Query tools
    # ========================================================================

    def get_board(self, session_id: str) -> str:
        """Return the current FEN string."""
        self._require_session(session_id)
        assert self._engine_state is not None
        return self._engine_state.fen

    def get_status(self, session_id: str) -> GameStatus:
        """Return full game status."""
        session = self._require_session(session_id)
        assert self._engine_state is not None

        your_color: str | None = None
        if session.role == SessionRole.WHITE:
            your_color = "white"
        elif session.role == SessionRole.BLACK:
            your_color = "black"

        return GameStatus(
            state=self._state,
            fen=self._engine_state.fen,
            turn=self._engine_state.turn,
            in_check=self._engine_state.in_check,
            is_checkmate=self._engine_state.is_checkmate,
            is_stalemate=self._engine_state.is_stalemate,
            can_claim_draw=self._engine_state.can_claim_draw,
            is_automatic_draw=self._engine_state.is_automatic_draw,
            halfmove_clock=self._engine_state.halfmove_clock,
            fullmove_number=self._engine_state.fullmove_number,
            result=self._result,
            termination_reason=self._termination_reason,
            your_color=your_color,
            draw_offered=(
                self._draw_offered_by is not None
                and self._draw_offered_by != session_id
            ),
            clock=self._clock.get_state() if self._clock else None,
        )

    def get_legal_moves(self, session_id: str) -> list[str]:
        """Return sorted list of legal moves in unhyphenated LAN."""
        self._require_session(session_id)
        assert self._engine_state is not None
        return sorted(self._engine_state.legal_moves)

    def get_history(self, session_id: str) -> list[MoveRecord]:
        """Return the move history."""
        self._require_session(session_id)
        return list(self._move_history)

    def done(self, session_id: str) -> dict[str, str | None]:
        """Acknowledge game completion. Only valid in game_over state."""
        self._require_session(session_id)
        if self._state != GameState.GAME_OVER:
            raise GameError("game is not over")
        return {
            "result": self._result,
            "termination_reason": self._termination_reason,
        }

    # ========================================================================
    # Action tools (stubs — full implementation in PR 6)
    # ========================================================================

    async def make_move(self, session_id: str, move: str) -> MoveRecord:
        """Make a move. Full implementation in PR 6."""
        self._require_ongoing()
        session = self._require_player(session_id)
        assert self._engine is not None
        assert self._engine_state is not None

        # Check turn
        expected_role = (
            SessionRole.WHITE
            if self._engine_state.turn == "white"
            else SessionRole.BLACK
        )
        if session.role != expected_role:
            raise GameError("not your turn")

        # Check pending draw offer (blocks opponent's move)
        if (
            self._draw_offered_by is not None
            and self._draw_offered_by != session_id
        ):
            raise GameError("must respond to draw offer before moving")

        # Withdraw own draw offer if making a move
        if self._draw_offered_by == session_id:
            self._draw_offered_by = None

        # Get state before move for SAN generation
        fen_before = self._engine_state.fen
        legal_moves_before = list(self._engine_state.legal_moves)

        # Make the move via engine
        result = await self._engine.make_move(move)
        self._engine_state = result.state
        self._last_activity = time.monotonic()

        # Generate SAN
        san = lan_to_san(
            lan=result.move_lan,
            fen_before=fen_before,
            legal_moves=legal_moves_before,
            in_check_after=result.state.in_check,
            is_checkmate_after=result.state.is_checkmate,
        )

        # Update clock
        clock_ms: int | None = None
        if self._clock:
            is_white = session.role == SessionRole.WHITE
            now_ms = int(time.monotonic() * 1000)
            if not self._clock._game_started:
                self._clock.start_game(timestamp_ms=now_ms)
            clock_ms = self._clock.move_made(is_white=is_white, timestamp_ms=now_ms)

        # Record move
        move_number = (len(self._move_history) // 2) + 1
        color = "white" if session.role == SessionRole.WHITE else "black"
        record = MoveRecord(
            move_number=move_number,
            color=color,
            san=san,
            lan=result.move_lan,
            clock_ms=clock_ms,
        )
        self._move_history.append(record)
        self._clock_times.append(clock_ms)
        self._record_position()

        # Check for game-ending conditions
        if result.state.is_checkmate:
            self._result = "1-0" if color == "white" else "0-1"
            self._termination_reason = "checkmate"
            self._state = GameState.GAME_OVER
        elif result.state.is_stalemate:
            self._result = "1/2-1/2"
            self._termination_reason = "stalemate"
            self._state = GameState.GAME_OVER
        elif result.state.is_automatic_draw:
            # Check if it's checkmate first (checkmate takes priority over 75-move)
            self._result = "1/2-1/2"
            if self._engine_state.halfmove_clock >= 150:
                self._termination_reason = "seventy-five-move rule"
            else:
                self._termination_reason = "fivefold repetition"
            self._state = GameState.GAME_OVER
        elif is_insufficient_material(result.state.fen):
            self._result = "1/2-1/2"
            self._termination_reason = "insufficient material"
            self._state = GameState.GAME_OVER
        elif self._clock and self._clock.is_flagged(is_white=session.role == SessionRole.WHITE):
            # The player who just moved flagged (shouldn't happen normally, but check)
            pass  # Flag is checked for the opponent below

        # Check opponent's flag (time ran out on their turn — detected when we move)
        # Actually, flag should be checked for the moving player since we deducted time.
        # If the moving player's clock went negative, they lose on time.
        if self._state == GameState.ONGOING and self._clock:
            is_white_moved = session.role == SessionRole.WHITE
            if self._clock.is_flagged(is_white=is_white_moved):
                # Moving player flagged
                winner = "Black" if is_white_moved else "White"
                loser = "White" if is_white_moved else "Black"
                # Check if opponent has insufficient material -> draw
                if is_insufficient_material(result.state.fen):
                    self._result = "1/2-1/2"
                    self._termination_reason = (
                        f"Draw \u2014 {loser}'s clock expired but "
                        f"{winner} has insufficient material"
                    )
                else:
                    self._result = "1-0" if winner == "White" else "0-1"
                    self._termination_reason = (
                        f"{winner} wins on time \u2014 {loser}'s clock expired"
                    )
                self._state = GameState.GAME_OVER

        return record

    async def resign(self, session_id: str) -> None:
        """Resign the game."""
        self._require_ongoing()
        session = self._require_player(session_id)

        if session.role == SessionRole.WHITE:
            self._result = "0-1"
            self._termination_reason = "White resigns \u2014 Black wins"
        else:
            self._result = "1-0"
            self._termination_reason = "Black resigns \u2014 White wins"
        self._state = GameState.GAME_OVER

    async def offer_draw(self, session_id: str) -> None:
        """Offer a draw to the opponent."""
        self._require_ongoing()
        self._require_player(session_id)

        if self._draw_offered_by is not None:
            raise GameError("draw already offered")

        self._draw_offered_by = session_id

        # Send message to opponent
        opponent_id = self._get_opponent_session(session_id)
        if opponent_id:
            color = "white" if self.get_session_role(session_id) == SessionRole.WHITE else "black"
            opponent_session = self._sessions[opponent_id]
            opponent_session.messages.append({
                "from": "server",
                "text": f"{color.capitalize()} offers a draw",
                "move_number": self._engine_state.fullmove_number if self._engine_state else 0,
            })

    async def accept_draw(self, session_id: str) -> None:
        """Accept a pending draw offer."""
        self._require_ongoing()
        self._require_player(session_id)

        if self._draw_offered_by is None:
            raise GameError("no draw offer to accept")
        if self._draw_offered_by == session_id:
            raise GameError("cannot accept your own draw offer")

        self._result = "1/2-1/2"
        self._termination_reason = "Draw by agreement"
        self._state = GameState.GAME_OVER
        self._draw_offered_by = None

    async def decline_draw(self, session_id: str) -> None:
        """Decline a pending draw offer."""
        self._require_ongoing()
        self._require_player(session_id)

        if self._draw_offered_by is None:
            raise GameError("no draw offer to decline")
        if self._draw_offered_by == session_id:
            raise GameError("cannot decline your own draw offer")

        self._draw_offered_by = None

    async def claim_draw(self, session_id: str) -> None:
        """Claim a draw under 50-move rule or threefold repetition. Turn-gated."""
        self._require_ongoing()
        session = self._require_player(session_id)
        assert self._engine_state is not None

        # Turn-gated
        expected_role = (
            SessionRole.WHITE
            if self._engine_state.turn == "white"
            else SessionRole.BLACK
        )
        if session.role != expected_role:
            raise GameError("not your turn")

        # Check conditions
        fifty_move = self._engine_state.halfmove_clock >= 100
        threefold = self._position_count() >= 3

        if not fifty_move and not threefold:
            raise GameError("draw claim not available")

        self._result = "1/2-1/2"
        # Fifty-move takes priority when both conditions are met
        if fifty_move:
            claimant = "White" if session.role == SessionRole.WHITE else "Black"
            self._termination_reason = f"Draw by fifty-move rule (claimed by {claimant})"
        else:
            claimant = "White" if session.role == SessionRole.WHITE else "Black"
            self._termination_reason = f"Draw by threefold repetition (claimed by {claimant})"
        self._state = GameState.GAME_OVER

    def _get_opponent_session(self, session_id: str) -> str | None:
        """Get the opponent's session ID."""
        session = self._sessions.get(session_id)
        if session is None:
            return None
        if session.role == SessionRole.WHITE:
            return self._black_session
        elif session.role == SessionRole.BLACK:
            return self._white_session
        return None

    def send_message(self, session_id: str, text: str) -> None:
        """Send a message to the opponent."""
        self._require_ongoing()
        session = self._require_player(session_id)

        opponent_id = self._get_opponent_session(session_id)
        if opponent_id is None:
            raise GameError("no opponent to message")

        color = "white" if session.role == SessionRole.WHITE else "black"
        move_number = self._engine_state.fullmove_number if self._engine_state else 0
        opponent_session = self._sessions[opponent_id]
        opponent_session.messages.append({
            "from": color,
            "text": text,
            "move_number": move_number,
        })

    def get_messages(self, session_id: str, clear: bool = True) -> list[dict[str, str | int]]:
        """Get messages for this session."""
        session = self._require_player(session_id)
        messages = list(session.messages)
        if clear:
            session.messages.clear()
        return messages

    def export_game(self, format: str = "fen") -> str:
        """Export game in the requested format."""
        if self._state == GameState.NO_GAME:
            raise GameError("no active game")
        assert self._engine_state is not None

        if format == "fen":
            return self._engine_state.fen
        elif format == "pgn":
            return self._generate_pgn()
        else:
            raise GameError(f"unsupported format: {format}")

    def _generate_pgn(self) -> str:
        """Generate PGN text for the current game."""
        return generate_pgn(
            moves=self._move_history,
            result=self._result,
            clock_times=self._clock_times if self._clock else None,
        )
