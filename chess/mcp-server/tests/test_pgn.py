"""Tests for PGN generation."""

from chess_mcp.pgn import format_clock_annotation, generate_pgn
from chess_mcp.types import MoveRecord


def test_tag_pairs_present() -> None:
    pgn = generate_pgn(moves=[], result=None)
    assert '[Event "MCP Chess Game"]' in pgn
    assert '[Site "localhost"]' in pgn
    assert '[Date "????.??.??"]' in pgn
    assert '[Round "1"]' in pgn
    assert '[White "White"]' in pgn
    assert '[Black "Black"]' in pgn
    assert '[Result "*"]' in pgn


def test_movetext_format() -> None:
    moves = [
        MoveRecord(move_number=1, color="white", san="e4", lan="e2e4"),
        MoveRecord(move_number=1, color="black", san="e5", lan="e7e5"),
        MoveRecord(move_number=2, color="white", san="Nf3", lan="g1f3"),
    ]
    pgn = generate_pgn(moves=moves, result="*")
    assert "1. e4 e5 2. Nf3 *" in pgn


def test_result_token() -> None:
    pgn = generate_pgn(moves=[], result="1-0")
    assert "1-0" in pgn
    assert '[Result "1-0"]' in pgn


def test_clock_annotations_format() -> None:
    assert format_clock_annotation(5400000) == "{[%clk 1:30:00.0]}"
    assert format_clock_annotation(299500) == "{[%clk 0:04:59.5]}"
    assert format_clock_annotation(0) == "{[%clk 0:00:00.0]}"


def test_no_annotations_when_untimed() -> None:
    moves = [MoveRecord(move_number=1, color="white", san="e4", lan="e2e4")]
    pgn = generate_pgn(moves=moves, result="*", clock_times=None)
    assert "%clk" not in pgn


def test_clock_annotations_in_pgn() -> None:
    moves = [
        MoveRecord(move_number=1, color="white", san="e4", lan="e2e4"),
        MoveRecord(move_number=1, color="black", san="e5", lan="e7e5"),
    ]
    pgn = generate_pgn(
        moves=moves,
        result="*",
        clock_times=[295000, 298000],
    )
    assert "{[%clk 0:04:55.0]}" in pgn
    assert "{[%clk 0:04:58.0]}" in pgn


def test_scholars_mate_full_pgn() -> None:
    moves = [
        MoveRecord(move_number=1, color="white", san="e4", lan="e2e4"),
        MoveRecord(move_number=1, color="black", san="e5", lan="e7e5"),
        MoveRecord(move_number=2, color="white", san="Bc4", lan="f1c4"),
        MoveRecord(move_number=2, color="black", san="Nc6", lan="b8c6"),
        MoveRecord(move_number=3, color="white", san="Qh5", lan="d1h5"),
        MoveRecord(move_number=3, color="black", san="Nf6", lan="g8f6"),
        MoveRecord(move_number=4, color="white", san="Qxf7#", lan="h5f7"),
    ]
    pgn = generate_pgn(moves=moves, result="1-0")
    assert "1. e4 e5 2. Bc4 Nc6 3. Qh5 Nf6 4. Qxf7# 1-0" in pgn
    assert '[Result "1-0"]' in pgn


def test_custom_fen_tag() -> None:
    fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    pgn = generate_pgn(moves=[], result="*", start_fen=fen)
    assert '[SetUp "1"]' in pgn
    assert f'[FEN "{fen}"]' in pgn


def test_export_fen_format() -> None:
    """export_game with format='fen' returns just the FEN string."""
    # This is tested at the GameManager level in test_actions.py
    pass
