"""Tests for SAN generation (lan_to_san)."""

import pytest

from chess_mcp.notation import lan_to_san


# Helper: for tests that need it, we provide legal_moves as the full set of
# LAN moves. We can use subsets where disambiguation is the focus.

INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def test_pawn_push_e4() -> None:
    """'e2e4' from initial position -> 'e4'."""
    san = lan_to_san(
        lan="e2e4",
        fen_before=INITIAL_FEN,
        legal_moves=["e2e3", "e2e4"],  # subset is fine
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "e4"


def test_knight_move_nf3() -> None:
    """'g1f3' -> 'Nf3'."""
    san = lan_to_san(
        lan="g1f3",
        fen_before=INITIAL_FEN,
        legal_moves=["g1f3", "g1h3"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "Nf3"


def test_pawn_capture_exd5() -> None:
    """Pawn capture 'e4d5' -> 'exd5'."""
    # Position after 1. e4 d5
    fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2"
    san = lan_to_san(
        lan="e4d5",
        fen_before=fen,
        legal_moves=["e4d5", "e4e5"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "exd5"


def test_piece_capture_nxe5() -> None:
    """Knight captures on e5: 'f3e5' -> 'Nxe5'."""
    # Position with Nf3, pawn on e5
    fen = "rnbqkbnr/pppp1ppp/8/4p3/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2"
    san = lan_to_san(
        lan="f3e5",
        fen_before=fen,
        legal_moves=["f3e5", "f3d4", "f3g5", "f3h4"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "Nxe5"


def test_kingside_castling() -> None:
    """'e1g1' with king -> 'O-O'."""
    # White can castle kingside
    fen = "rnbqkbnr/pppppppp/8/8/8/5NP1/PPPPPP1P/RNBQKB1R w KQkq - 0 3"
    san = lan_to_san(
        lan="e1g1",
        fen_before=fen,
        legal_moves=["e1g1"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "O-O"


def test_queenside_castling() -> None:
    """'e1c1' with king -> 'O-O-O'."""
    fen = "rnbqkbnr/pppppppp/8/8/8/2NQ4/PPPPPPPP/R3KBNR w KQkq - 0 4"
    san = lan_to_san(
        lan="e1c1",
        fen_before=fen,
        legal_moves=["e1c1"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "O-O-O"


def test_promotion_e8q() -> None:
    """'e7e8q' -> 'e8=Q'."""
    fen = "8/4P3/8/8/8/8/8/4K2k w - - 0 1"
    san = lan_to_san(
        lan="e7e8q",
        fen_before=fen,
        legal_moves=["e7e8q", "e7e8r", "e7e8b", "e7e8n"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "e8=Q"


def test_promotion_capture_dxe8q() -> None:
    """'d7e8q' capturing on e8 -> 'dxe8=Q'."""
    fen = "4r3/3P4/8/8/8/8/8/4K2k w - - 0 1"
    san = lan_to_san(
        lan="d7e8q",
        fen_before=fen,
        legal_moves=["d7e8q", "d7e8r", "d7d8q"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "dxe8=Q"


def test_check_suffix() -> None:
    """Move that gives check gets '+' suffix."""
    # Knight on f7 giving check to king on h8-ish
    fen = "4k3/5N2/8/8/8/8/8/4K3 w - - 0 1"
    san = lan_to_san(
        lan="f7d6",
        fen_before=fen,
        legal_moves=["f7d6", "f7e5"],
        in_check_after=True,
        is_checkmate_after=False,
    )
    assert san == "Nd6+"


def test_checkmate_suffix() -> None:
    """Move that gives checkmate gets '#' suffix."""
    # Scholar's mate final position: Qxf7#
    fen = "rnbqkbnr/pppp1ppp/8/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 3"
    # Qh5 is already played, now Qxf7 is the mating move
    fen_before_mate = "rnbqkbnr/pppp1ppp/8/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 2 4"
    san = lan_to_san(
        lan="h5f7",
        fen_before=fen_before_mate,
        legal_moves=["h5f7"],
        in_check_after=True,
        is_checkmate_after=True,
    )
    assert san == "Qxf7#"


def test_file_disambiguation_rae1() -> None:
    """Two rooks on a-file and h-file; rook moves to e4 -> 'Rae4'. Needs file disambiguation."""
    # Rooks on a4 and h4, king on e1. Move Ra4 to e4.
    fen = "4k3/8/8/8/R6R/8/8/4K3 w - - 0 1"
    san = lan_to_san(
        lan="a4e4",
        fen_before=fen,
        legal_moves=["a4e4", "h4e4"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "Rae4"


def test_rank_disambiguation() -> None:
    """Two rooks on same file, need rank disambiguation: 'R1e3'."""
    fen = "4k3/4R3/8/8/8/8/8/4RK2 w - - 0 1"
    san = lan_to_san(
        lan="e1e3",
        fen_before=fen,
        legal_moves=["e1e3", "e7e3", "e1e2", "e7e6"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "R1e3"


def test_full_disambiguation() -> None:
    """Three queens that can reach same square — need full disambiguation."""
    # Queens on a1, a3, c1; all can reach b2
    fen = "4k3/8/8/8/8/Q7/8/Q1Q1K3 w - - 0 1"
    san = lan_to_san(
        lan="a1b2",
        fen_before=fen,
        legal_moves=["a1b2", "a3b2", "c1b2"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "Qa1b2"


def test_en_passant_capture() -> None:
    """En passant: 'e5d6' with pawn on e5, ep target d6 -> 'exd6'."""
    fen = "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"
    san = lan_to_san(
        lan="e5d6",
        fen_before=fen,
        legal_moves=["e5d6", "e5e6"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "exd6"


def test_geometric_disambiguation_pinned_piece() -> None:
    """Pinned piece still causes disambiguation (geometric reachability, not legal moves).

    Rook on a4 is pinned to white king on a1 by black rook on a7. Another white
    rook on h4. Both rooks geometrically reach e4, so 'h4e4' still needs file
    disambiguation even though a4 rook can't legally move to e4.
    """
    fen = "4k3/r7/8/8/R6R/8/8/K7 w - - 0 1"
    san = lan_to_san(
        lan="h4e4",
        fen_before=fen,
        # Only h4e4 is legal (a4 rook is pinned), but geometric check still sees a4 rook
        legal_moves=["h4e4", "h4g4", "h4f4"],
        in_check_after=True,
        is_checkmate_after=False,
    )
    assert san == "Rhe4+"


def test_black_castling_kingside() -> None:
    """Black kingside castling: 'e8g8' -> 'O-O'."""
    fen = "rnbqk2r/ppppppbp/5np1/8/8/5NP1/PPPPPPBP/RNBQK2R b KQkq - 4 3"
    san = lan_to_san(
        lan="e8g8",
        fen_before=fen,
        legal_moves=["e8g8", "e8f8"],
        in_check_after=False,
        is_checkmate_after=False,
    )
    assert san == "O-O"
