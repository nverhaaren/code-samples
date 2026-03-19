"""Chess clock implementation supporting multiple time control modes.

Modes:
- sudden_death: Fixed time, no increment
- fischer: Fixed time + increment added after each move
- bronstein: Delay before clock starts ticking; unused delay not banked
- multi_period: Multiple time periods with move count thresholds
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class TimePeriod:
    """A single time control period."""

    time_ms: int  # Initial time in milliseconds
    increment_ms: int = 0  # Fischer increment per move
    delay_ms: int = 0  # Bronstein delay per move
    moves: int | None = None  # Moves in this period (None = unlimited)


@dataclass
class ClockState:
    """Current state of one player's clock."""

    remaining_ms: int
    period_index: int = 0
    moves_in_period: int = 0


class ChessClock:
    """Chess clock managing time for both players.

    White's clock doesn't start until White's first move.
    """

    def __init__(
        self,
        periods: list[TimePeriod],
        seeded: bool = False,
    ) -> None:
        self._periods = periods
        self._seeded = seeded

        # Initialize both players with first period's time
        first = periods[0]
        self._white = ClockState(remaining_ms=first.time_ms)
        self._black = ClockState(remaining_ms=first.time_ms)

        self._white_started = False  # White's clock doesn't start until first move
        self._is_white_turn = True
        self._last_tick_ms: int | None = None  # timestamp of last move/start
        self._game_started = False

    @property
    def white_remaining_ms(self) -> int:
        return self._white.remaining_ms

    @property
    def black_remaining_ms(self) -> int:
        return self._black.remaining_ms

    def _current_player(self) -> ClockState:
        return self._white if self._is_white_turn else self._black

    def _current_period(self, player: ClockState) -> TimePeriod:
        idx = min(player.period_index, len(self._periods) - 1)
        return self._periods[idx]

    def start_game(self, timestamp_ms: int = 0) -> None:
        """Mark the game as started."""
        self._game_started = True
        # White's clock doesn't start until White's first move
        self._last_tick_ms = timestamp_ms

    def move_made(self, is_white: bool, timestamp_ms: int = 0) -> int:
        """Record that a move was made. Returns remaining time for the player.

        Args:
            is_white: Whether white made the move
            timestamp_ms: Current timestamp in milliseconds

        Returns:
            Remaining time in ms for the player who moved
        """
        player = self._white if is_white else self._black
        period = self._current_period(player)

        # Calculate elapsed time
        if self._seeded:
            elapsed = 0
        elif self._last_tick_ms is not None and (
            not is_white or self._white_started
        ):
            elapsed = timestamp_ms - self._last_tick_ms
        else:
            elapsed = 0

        # Mark white as started after first move
        if is_white and not self._white_started:
            self._white_started = True

        # Deduct time based on mode
        if period.delay_ms > 0:
            # Bronstein: first delay_ms of thinking time is free
            effective_elapsed = max(0, elapsed - period.delay_ms)
            player.remaining_ms -= effective_elapsed
        else:
            player.remaining_ms -= elapsed

        # Add Fischer increment (after deduction)
        if period.increment_ms > 0:
            player.remaining_ms += period.increment_ms

        # Track moves in period for multi-period
        player.moves_in_period += 1

        # Check for period transition
        if (
            period.moves is not None
            and player.moves_in_period >= period.moves
            and player.period_index + 1 < len(self._periods)
        ):
            player.period_index += 1
            next_period = self._current_period(player)
            player.remaining_ms += next_period.time_ms
            player.moves_in_period = 0

        # Update turn tracking
        self._is_white_turn = not is_white
        self._last_tick_ms = timestamp_ms

        return player.remaining_ms

    def is_flagged(self, is_white: bool) -> bool:
        """Check if a player's time has run out."""
        player = self._white if is_white else self._black
        return player.remaining_ms <= 0

    def get_state(self) -> dict[str, Any]:
        """Get clock state for both players."""
        return {
            "white_ms": self._white.remaining_ms,
            "black_ms": self._black.remaining_ms,
            "white_started": self._white_started,
        }


def create_clock(
    mode: str,
    time_ms: int,
    increment_ms: int = 0,
    delay_ms: int = 0,
    periods: list[dict[str, Any]] | None = None,
    seeded: bool = False,
) -> ChessClock:
    """Factory for creating clocks from configuration.

    Args:
        mode: 'sudden_death', 'fischer', 'bronstein', or 'multi_period'
        time_ms: Initial time in milliseconds
        increment_ms: Fischer increment per move
        delay_ms: Bronstein delay per move
        periods: List of period configs for multi_period mode
        seeded: If True, elapsed time is always 0 (deterministic)
    """
    if mode == "sudden_death":
        return ChessClock([TimePeriod(time_ms=time_ms)], seeded=seeded)
    elif mode == "fischer":
        return ChessClock(
            [TimePeriod(time_ms=time_ms, increment_ms=increment_ms)],
            seeded=seeded,
        )
    elif mode == "bronstein":
        return ChessClock(
            [TimePeriod(time_ms=time_ms, delay_ms=delay_ms)],
            seeded=seeded,
        )
    elif mode == "multi_period":
        if not periods:
            raise ValueError("multi_period mode requires periods list")
        period_list = [
            TimePeriod(
                time_ms=p["time_ms"],
                increment_ms=p.get("increment_ms", 0),
                delay_ms=p.get("delay_ms", 0),
                moves=p.get("moves"),
            )
            for p in periods
        ]
        return ChessClock(period_list, seeded=seeded)
    else:
        raise ValueError(f"unknown clock mode: {mode}")
