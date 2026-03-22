"""JSON-lines recording of MCP tool invocations for replay and debugging."""

from __future__ import annotations

import json
import time
from pathlib import Path
from typing import Any


class Recorder:
    """Records MCP tool calls to a JSON-lines file.

    Each line is a JSON object with:
    - ts_ms: timestamp in milliseconds
    - session_id: which session made the call
    - tool: tool name
    - params: tool parameters
    - result: tool response
    - elapsed_ms: how long the call took

    Special markers:
    - game_start: marks the beginning of a game
    - game_end: marks the end of a game
    """

    def __init__(self, path: str | Path | None = None, seeded: bool = False) -> None:
        self._path = Path(path) if path else None
        self._seeded = seeded
        self._records: list[dict[str, Any]] = []
        self._start_time_ms = int(time.time() * 1000)

    def _now_ms(self) -> int:
        if self._seeded:
            return 0
        return int(time.time() * 1000)

    def record_game_start(self, game_id: str, fen: str) -> None:
        """Record game start marker."""
        entry: dict[str, Any] = {
            "ts_ms": self._now_ms(),
            "type": "game_start",
            "game_id": game_id,
            "fen": fen,
        }
        self._records.append(entry)
        self._flush(entry)

    def record_game_end(
        self, game_id: str, result: str | None, reason: str | None
    ) -> None:
        """Record game end marker."""
        entry: dict[str, Any] = {
            "ts_ms": self._now_ms(),
            "type": "game_end",
            "game_id": game_id,
            "result": result,
            "reason": reason,
        }
        self._records.append(entry)
        self._flush(entry)

    def record_tool_call(
        self,
        session_id: str,
        tool: str,
        params: dict[str, Any],
        result: dict[str, Any],
        elapsed_ms: int,
    ) -> None:
        """Record a tool call."""
        entry: dict[str, Any] = {
            "ts_ms": self._now_ms(),
            "session_id": session_id,
            "tool": tool,
            "params": params,
            "result": result,
            "elapsed_ms": 0 if self._seeded else elapsed_ms,
        }
        self._records.append(entry)
        self._flush(entry)

    def _flush(self, entry: dict[str, Any]) -> None:
        """Write entry to file if path is set."""
        if self._path:
            with open(self._path, "a") as f:
                f.write(json.dumps(entry) + "\n")

    def get_records(self) -> list[dict[str, Any]]:
        """Return all recorded entries."""
        return list(self._records)

    @staticmethod
    def read(path: str | Path) -> list[dict[str, Any]]:
        """Read a recording file."""
        records = []
        with open(path) as f:
            for line in f:
                line = line.strip()
                if line:
                    records.append(json.loads(line))
        return records
