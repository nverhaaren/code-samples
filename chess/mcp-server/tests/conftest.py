"""Shared fixtures for chess MCP tests."""

import pytest

from chess_mcp.engine_path import find_engine_binary


@pytest.fixture
def engine_path() -> str:
    """Return the path to the chess engine binary."""
    return find_engine_binary()
