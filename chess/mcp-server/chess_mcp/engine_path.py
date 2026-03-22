"""Shared engine binary discovery logic."""

from __future__ import annotations

import os
import pathlib


def find_engine_binary() -> str:
    """Locate the chess engine binary, checking common build paths.

    Checks CHESS_ENGINE_PATH environment variable first, then searches
    relative to the mcp-server directory for common build output locations.

    Raises:
        FileNotFoundError: If no engine binary is found.
    """
    # Relative to this file: chess_mcp/ -> mcp-server/ -> chess/
    base = pathlib.Path(__file__).resolve().parent.parent.parent
    candidates = [
        base / "build" / "chess",
        base / "build" / "Debug" / "chess",
        base / "build" / "Release" / "chess",
    ]
    env_path = os.environ.get("CHESS_ENGINE_PATH")
    if env_path:
        candidates.insert(0, pathlib.Path(env_path))

    for candidate in candidates:
        if candidate.is_file():
            return str(candidate)

    raise FileNotFoundError(
        f"Chess engine binary not found. Searched: {[str(c) for c in candidates]}. "
        "Set CHESS_ENGINE_PATH to override."
    )
