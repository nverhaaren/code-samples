"""Tests for insufficient material detection."""

from chess_mcp.material import is_insufficient_material

INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def test_k_vs_k() -> None:
    """King vs King is insufficient."""
    assert is_insufficient_material("4k3/8/8/8/8/8/8/4K3 w - - 0 1") is True


def test_k_n_vs_k() -> None:
    """King + Knight vs King is insufficient."""
    assert is_insufficient_material("4k3/8/8/8/8/8/8/4KN2 w - - 0 1") is True


def test_k_b_vs_k() -> None:
    """King + Bishop vs King is insufficient."""
    assert is_insufficient_material("4k3/8/8/8/8/8/8/4KB2 w - - 0 1") is True


def test_k_b_vs_k_b_same_color_squares() -> None:
    """K+B vs K+B on same-color squares is insufficient."""
    # Both bishops on light squares (c1 and f8 are dark, c4 and f5 are light)
    # c1 = dark, f1 = light. Let's use: bishops on c1 (dark) and f8 (dark)
    # Actually: square color = (file + rank) % 2. a1=(0+0)%2=0 (dark)
    # c1=(2+0)%2=0 dark, f8=(5+7)%2=0 dark -> same color
    assert is_insufficient_material("5b2/8/8/8/8/8/4k3/2B1K3 w - - 0 1") is True


def test_k_b_vs_k_b_different_color_squares() -> None:
    """K+B vs K+B on different-color squares is sufficient."""
    # c1=(2+0)%2=0 dark, c8=(2+7)%2=1 light -> different colors
    assert is_insufficient_material("2b5/8/8/8/8/8/4k3/2B1K3 w - - 0 1") is False


def test_k_nn_vs_k() -> None:
    """King + 2 Knights vs King is NOT insufficient (mate is possible in theory)."""
    assert is_insufficient_material("4k3/8/8/8/8/8/8/3NKN2 w - - 0 1") is False


def test_k_r_vs_k() -> None:
    """King + Rook vs King is sufficient."""
    assert is_insufficient_material("4k3/8/8/8/8/8/8/4KR2 w - - 0 1") is False


def test_k_p_vs_k() -> None:
    """King + Pawn vs King is sufficient."""
    assert is_insufficient_material("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1") is False


def test_initial_position() -> None:
    """Initial position is not insufficient material."""
    assert is_insufficient_material(INITIAL_FEN) is False
