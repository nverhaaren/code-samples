"""Shared fixtures for chess MCP tests."""

import os
import pathlib

import pytest


def _find_engine_binary() -> str:
    """Locate the chess engine binary, checking common build paths."""
    # Relative to the mcp-server directory
    base = pathlib.Path(__file__).resolve().parent.parent.parent
    candidates = [
        base / "build" / "chess",
        base / "build" / "Debug" / "chess",
        base / "build" / "Release" / "chess",
    ]
    # Allow override via environment variable
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


@pytest.fixture
def engine_path() -> str:
    """Return the path to the chess engine binary."""
    return _find_engine_binary()
