"""Async subprocess wrapper for the C++ chess engine's JSON bridge mode."""

from __future__ import annotations

import asyncio
import json
from typing import Any

from .types import EngineError, EngineState, MoveResult


def _normalize_lan(lan: str) -> str:
    """Remove hyphens from hyphenated LAN (e.g. 'e2-e4' -> 'e2e4')."""
    return lan.replace("-", "")


def _parse_state(data: dict[str, Any]) -> EngineState:
    """Parse a state dict from the engine JSON into an EngineState."""
    return EngineState(
        fen=data["fen"],
        turn=data["turn"],
        legal_moves=[_normalize_lan(m) for m in data["legalMoves"]],
        in_check=data["inCheck"],
        is_checkmate=data["isCheckmate"],
        is_stalemate=data["isStalemate"],
        can_claim_draw=data["canClaimDraw"],
        is_automatic_draw=data["isAutomaticDraw"],
        halfmove_clock=data["halfmoveClock"],
        fullmove_number=data["fullmoveNumber"],
        move_history=[_normalize_lan(m) for m in data["moveHistory"]],
    )


class ChessEngine:
    """Async wrapper around the C++ chess engine's --json-bridge mode.

    Usage:
        engine = ChessEngine("/path/to/chess")
        await engine.start()
        state = await engine.new_game()
        result = await engine.make_move("e4")
        await engine.stop()
    """

    def __init__(self, binary_path: str) -> None:
        self._binary_path = binary_path
        self._process: asyncio.subprocess.Process | None = None

    async def start(self) -> None:
        """Start the engine subprocess in JSON bridge mode."""
        self._process = await asyncio.create_subprocess_exec(
            self._binary_path,
            "--json-bridge",
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

    async def stop(self) -> None:
        """Send quit command and wait for the process to exit."""
        if self._process is None:
            return
        try:
            await self._send_command({"command": "quit"})
        except Exception:
            pass
        try:
            self._process.kill()
        except ProcessLookupError:
            pass
        await self._process.wait()
        self._process = None

    async def _send_command(self, cmd: dict[str, Any]) -> dict[str, Any]:
        """Send a JSON command and read the JSON response line."""
        assert self._process is not None, "Engine not started"
        assert self._process.stdin is not None
        assert self._process.stdout is not None

        line = json.dumps(cmd) + "\n"
        self._process.stdin.write(line.encode())
        await self._process.stdin.drain()

        response_line = await self._process.stdout.readline()
        if not response_line:
            raise EngineError("Engine process closed unexpectedly")

        return json.loads(response_line)  # type: ignore[no-any-return]

    async def new_game(self) -> EngineState:
        """Start a new game from the initial position."""
        resp = await self._send_command({"command": "new_game"})
        if not resp.get("ok"):
            raise EngineError(resp.get("error", "unknown error"))
        return _parse_state(resp["state"])

    async def from_fen(self, fen: str) -> EngineState:
        """Load a position from a FEN string."""
        resp = await self._send_command({"command": "from_fen", "fen": fen})
        if not resp.get("ok"):
            raise EngineError(resp.get("error", "unknown error"))
        return _parse_state(resp["state"])

    async def make_move(self, move: str) -> MoveResult:
        """Make a move (SAN or LAN). Returns the resulting state and the LAN of the move."""
        resp = await self._send_command({"command": "make_move", "move": move})
        if not resp.get("ok"):
            raise EngineError(resp.get("error", "unknown error"))
        return MoveResult(
            state=_parse_state(resp["state"]),
            move_lan=_normalize_lan(resp["move_lan"]),
        )

    async def get_state(self) -> EngineState:
        """Get the current game state."""
        resp = await self._send_command({"command": "get_state"})
        if not resp.get("ok"):
            raise EngineError(resp.get("error", "unknown error"))
        return _parse_state(resp["state"])

    async def parse_san(self, san: str) -> str | None:
        """Parse SAN notation, returning unhyphenated LAN or None if invalid."""
        resp = await self._send_command({"command": "parse_san", "san": san})
        if not resp.get("ok"):
            return None
        return _normalize_lan(resp["lan"])
