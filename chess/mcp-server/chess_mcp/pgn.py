"""PGN (Portable Game Notation) generation.

Generates PGN text with proper tag pairs, SAN movetext, and optional
clock annotations in {[%clk h:mm:ss.s]} format.
"""

from __future__ import annotations

from .types import MoveRecord


def format_clock_annotation(ms: int) -> str:
    """Format milliseconds as PGN clock annotation: {[%clk h:mm:ss.s]}.

    Hours not zero-padded; minutes and seconds zero-padded to 2 digits.
    Time truncated to tenths of a second.
    """
    total_seconds = ms / 1000.0
    hours = int(total_seconds // 3600)
    minutes = int((total_seconds % 3600) // 60)
    seconds = total_seconds % 60
    # Truncate to tenths
    tenths = int((seconds * 10)) % 10
    whole_seconds = int(seconds)
    return f"{{[%clk {hours}:{minutes:02d}:{whole_seconds:02d}.{tenths}]}}"


def generate_pgn(
    moves: list[MoveRecord],
    result: str | None = None,
    white_name: str = "White",
    black_name: str = "Black",
    event: str = "MCP Chess Game",
    site: str = "localhost",
    date: str = "????.??.??",
    round_num: str = "1",
    start_fen: str | None = None,
    clock_times: list[int | None] | None = None,
) -> str:
    """Generate PGN text.

    Args:
        moves: Move history as MoveRecord list
        result: Game result string ("1-0", "0-1", "1/2-1/2", or None/"*")
        white_name: White player name
        black_name: Black player name
        event: Event tag
        site: Site tag
        date: Date tag
        round_num: Round tag
        start_fen: Custom starting FEN (adds FEN and SetUp tags)
        clock_times: Optional list of remaining clock times in ms after each half-move
    """
    lines: list[str] = []

    # Tag pairs
    result_str = result if result else "*"
    lines.append(f'[Event "{event}"]')
    lines.append(f'[Site "{site}"]')
    lines.append(f'[Date "{date}"]')
    lines.append(f'[Round "{round_num}"]')
    lines.append(f'[White "{white_name}"]')
    lines.append(f'[Black "{black_name}"]')
    lines.append(f'[Result "{result_str}"]')

    if start_fen:
        lines.append('[SetUp "1"]')
        lines.append(f'[FEN "{start_fen}"]')

    lines.append("")

    # Movetext
    movetext_parts: list[str] = []
    for i, record in enumerate(moves):
        part = ""
        if record.color == "white":
            part = f"{record.move_number}. {record.san}"
        else:
            part = record.san

        # Add clock annotation if available
        if clock_times and i < len(clock_times) and clock_times[i] is not None:
            part += " " + format_clock_annotation(clock_times[i])

        movetext_parts.append(part)

    movetext_parts.append(result_str)
    lines.append(" ".join(movetext_parts))

    return "\n".join(lines)
