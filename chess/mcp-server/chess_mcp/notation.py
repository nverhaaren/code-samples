"""SAN (Standard Algebraic Notation) generation from LAN moves.

Converts unhyphenated LAN (e.g. "e2e4") to SAN (e.g. "e4") given the
board state before the move, legal moves list, and post-move check/mate info.
"""

from __future__ import annotations

# Piece letters for SAN output
_PIECE_CHARS: dict[str, str] = {
    "P": "",  # pawns have no prefix
    "N": "N",
    "B": "B",
    "R": "R",
    "Q": "Q",
    "K": "K",
    "p": "",
    "n": "N",
    "b": "B",
    "r": "R",
    "q": "Q",
    "k": "K",
}

_PROMO_CHARS: dict[str, str] = {
    "q": "Q",
    "r": "R",
    "b": "B",
    "n": "N",
}


def _parse_fen_board(fen: str) -> list[list[str | None]]:
    """Parse FEN piece placement into 8x8 board. board[rank][file], rank 0=1, file 0=a."""
    rows = fen.split()[0].split("/")
    board: list[list[str | None]] = []
    # FEN rows go from rank 8 (index 0) to rank 1 (index 7)
    for row in reversed(rows):
        rank: list[str | None] = []
        for ch in row:
            if ch.isdigit():
                rank.extend([None] * int(ch))
            else:
                rank.append(ch)
        board.append(rank)
    return board


def _square_to_coords(sq: str) -> tuple[int, int]:
    """'e4' -> (rank=3, file=4)."""
    return int(sq[1]) - 1, ord(sq[0]) - ord("a")


def _coords_to_square(rank: int, file: int) -> str:
    return chr(ord("a") + file) + str(rank + 1)


def _is_white_piece(piece: str) -> bool:
    return piece.isupper()


def _can_reach_geometrically(
    piece_type: str, from_rank: int, from_file: int, to_rank: int, to_file: int,
    board: list[list[str | None]],
) -> bool:
    """Check if a piece of the given type can geometrically reach the target square.

    This checks movement patterns only (not legality like pins). Used for SAN
    disambiguation per FIDE rules (geometric reachability, not legal-move based).
    """
    dr = to_rank - from_rank
    df = to_file - from_file
    pt = piece_type.upper()

    if pt == "N":
        return (abs(dr), abs(df)) in [(1, 2), (2, 1)]

    if pt == "B":
        if abs(dr) != abs(df) or dr == 0:
            return False
        return _path_clear(from_rank, from_file, to_rank, to_file, board)

    if pt == "R":
        if dr != 0 and df != 0:
            return False
        if dr == 0 and df == 0:
            return False
        return _path_clear(from_rank, from_file, to_rank, to_file, board)

    if pt == "Q":
        if dr == 0 and df == 0:
            return False
        if dr != 0 and df != 0 and abs(dr) != abs(df):
            return False
        return _path_clear(from_rank, from_file, to_rank, to_file, board)

    return False  # Kings and pawns don't need disambiguation


def _path_clear(
    from_rank: int, from_file: int, to_rank: int, to_file: int,
    board: list[list[str | None]],
) -> bool:
    """Check that all squares between from and to are empty (sliding pieces)."""
    dr = 0 if to_rank == from_rank else (1 if to_rank > from_rank else -1)
    df = 0 if to_file == from_file else (1 if to_file > from_file else -1)
    r, f = from_rank + dr, from_file + df
    while (r, f) != (to_rank, to_file):
        if board[r][f] is not None:
            return False
        r += dr
        f += df
    return True


def lan_to_san(
    lan: str,
    fen_before: str,
    legal_moves: list[str],
    in_check_after: bool,
    is_checkmate_after: bool,
) -> str:
    """Convert an unhyphenated LAN move to SAN notation.

    Args:
        lan: Unhyphenated LAN move (e.g. "e2e4", "g1f3", "e1g1", "e7e8q").
        fen_before: FEN string of the position before the move.
        legal_moves: List of all legal moves in unhyphenated LAN.
        in_check_after: Whether the opponent is in check after the move.
        is_checkmate_after: Whether the opponent is in checkmate after the move.

    Returns:
        SAN string (e.g. "e4", "Nf3", "O-O", "e8=Q", "Qxf7#").
    """
    board = _parse_fen_board(fen_before)
    fen_parts = fen_before.split()
    ep_square = fen_parts[3] if len(fen_parts) > 3 else "-"

    # Parse source and destination
    src = lan[:2]
    dst = lan[2:4]
    promo_char = lan[4] if len(lan) > 4 else None

    src_rank, src_file = _square_to_coords(src)
    dst_rank, dst_file = _square_to_coords(dst)

    piece = board[src_rank][src_file]
    assert piece is not None, f"No piece at {src} in FEN: {fen_before}"

    piece_type = piece.upper()
    is_white = _is_white_piece(piece)

    # Detect castling: king moves 2 squares laterally
    if piece_type == "K" and abs(dst_file - src_file) == 2:
        san = "O-O" if dst_file > src_file else "O-O-O"
        if is_checkmate_after:
            san += "#"
        elif in_check_after:
            san += "+"
        return san

    # Determine if capture
    target = board[dst_rank][dst_file]
    is_capture = target is not None
    # En passant: pawn captures diagonally to empty square at ep target
    if piece_type == "P" and src_file != dst_file and target is None:
        if ep_square != "-":
            ep_rank, ep_file = _square_to_coords(ep_square)
            if dst_rank == ep_rank and dst_file == ep_file:
                is_capture = True

    # Build SAN
    san = ""

    if piece_type == "P":
        if is_capture:
            san += chr(ord("a") + src_file) + "x"
        san += _coords_to_square(dst_rank, dst_file)
        if promo_char:
            san += "=" + _PROMO_CHARS[promo_char.lower()]
    else:
        san += _PIECE_CHARS[piece]

        # Disambiguation for non-pawn, non-king pieces
        if piece_type in ("N", "B", "R", "Q"):
            # Find all same-type, same-color pieces that can geometrically reach dst
            ambiguous: list[tuple[int, int]] = []
            for r in range(8):
                for f in range(8):
                    if (r, f) == (src_rank, src_file):
                        continue
                    other = board[r][f]
                    if other is None:
                        continue
                    if other.upper() != piece_type:
                        continue
                    if _is_white_piece(other) != is_white:
                        continue
                    if _can_reach_geometrically(piece_type, r, f, dst_rank, dst_file, board):
                        ambiguous.append((r, f))

            if ambiguous:
                same_file = any(f == src_file for _, f in ambiguous)
                same_rank = any(r == src_rank for r, _ in ambiguous)
                if not same_file:
                    san += chr(ord("a") + src_file)
                elif not same_rank:
                    san += str(src_rank + 1)
                else:
                    san += chr(ord("a") + src_file) + str(src_rank + 1)

        if is_capture:
            san += "x"
        san += _coords_to_square(dst_rank, dst_file)

    # Check/mate suffix
    if is_checkmate_after:
        san += "#"
    elif in_check_after:
        san += "+"

    return san
