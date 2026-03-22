"""Insufficient material detection from FEN.

Checks whether a position has insufficient material for either side to
deliver checkmate, per FIDE rules.
"""

from __future__ import annotations


def _square_color(rank: int, file: int) -> int:
    """Return 0 for dark square, 1 for light square."""
    return (rank + file) % 2


def is_insufficient_material(fen: str) -> bool:
    """Check if the position has insufficient material to deliver checkmate.

    Insufficient cases:
    - K vs K
    - K+N vs K
    - K+B vs K
    - K+B vs K+B where both bishops are on same-color squares

    Everything else (including K+N+N vs K) is considered sufficient.
    """
    piece_placement = fen.split()[0]

    white_pieces: list[str] = []
    black_pieces: list[str] = []
    bishop_squares: list[int] = []  # square colors for all bishops

    rank = 7  # Start from rank 8
    file = 0
    for ch in piece_placement:
        if ch == "/":
            rank -= 1
            file = 0
            continue
        if ch.isdigit():
            file += int(ch)
            continue

        piece_type = ch.upper()
        if ch.isupper():
            white_pieces.append(piece_type)
        else:
            black_pieces.append(piece_type)

        if piece_type == "B":
            bishop_squares.append(_square_color(rank, file))

        file += 1

    # Remove kings for counting
    w = [p for p in white_pieces if p != "K"]
    b = [p for p in black_pieces if p != "K"]

    total = len(w) + len(b)

    # K vs K
    if total == 0:
        return True

    # K+N vs K or K vs K+N
    if total == 1:
        piece = (w + b)[0]
        if piece == "N":
            return True
        if piece == "B":
            return True
        return False

    # K+B vs K+B, same color squares
    if total == 2 and len(w) == 1 and len(b) == 1:
        if w[0] == "B" and b[0] == "B":
            # Both bishops on same color square
            return bishop_squares[0] == bishop_squares[1]

    return False
