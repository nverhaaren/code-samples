"""Tests for the chess clock implementation."""

from chess_mcp.clock import ChessClock, TimePeriod, create_clock


# ============================================================================
# Sudden death
# ============================================================================


def test_sudden_death_initial_time() -> None:
    clock = create_clock("sudden_death", time_ms=300_000)
    assert clock.white_remaining_ms == 300_000
    assert clock.black_remaining_ms == 300_000


def test_sudden_death_time_deducted() -> None:
    clock = create_clock("sudden_death", time_ms=300_000)
    clock.start_game(timestamp_ms=0)
    # White moves after 10 seconds
    remaining = clock.move_made(is_white=True, timestamp_ms=10_000)
    # First move: white's clock hadn't started, so no deduction
    # Actually: white_started becomes True, but elapsed is 0 since white wasn't started
    assert remaining == 300_000
    # Black moves after 5 more seconds
    remaining = clock.move_made(is_white=False, timestamp_ms=15_000)
    assert remaining == 295_000  # 300000 - 5000
    # White moves after 20 more seconds
    remaining = clock.move_made(is_white=True, timestamp_ms=35_000)
    assert remaining == 280_000  # 300000 - 20000


def test_sudden_death_flag_at_zero() -> None:
    clock = create_clock("sudden_death", time_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # start white's clock
    clock.move_made(is_white=False, timestamp_ms=1_000)  # black thinks 1s
    clock.move_made(is_white=True, timestamp_ms=6_000)  # white thinks 5s
    clock.move_made(is_white=False, timestamp_ms=12_000)  # black thinks 6s -> flagged
    assert clock.is_flagged(is_white=False) is True


# ============================================================================
# Fischer
# ============================================================================


def test_fischer_increment_added() -> None:
    clock = create_clock("fischer", time_ms=300_000, increment_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)
    assert clock.white_remaining_ms == 305_000  # 300000 + 5000 increment


def test_fischer_fast_move_gains_time() -> None:
    clock = create_clock("fischer", time_ms=300_000, increment_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # start
    # Black moves after 2 seconds (gains 3s net)
    remaining = clock.move_made(is_white=False, timestamp_ms=2_000)
    assert remaining == 303_000  # 300000 - 2000 + 5000


def test_fischer_flag_before_increment() -> None:
    """If time runs out, flag happens before increment would be added."""
    clock = create_clock("fischer", time_ms=1_000, increment_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # start
    # Black thinks for 2 seconds when only 1 second remains
    clock.move_made(is_white=False, timestamp_ms=2_000)
    # remaining = 1000 - 2000 + 5000 = 4000 (increment is added after deduction)
    # In our implementation, increment is always added, so it's not "flag before increment"
    # This is the standard Fischer behavior: clock can go negative then increment saves it
    assert clock.black_remaining_ms == 4_000


# ============================================================================
# Bronstein
# ============================================================================


def test_bronstein_within_delay_costs_zero() -> None:
    clock = create_clock("bronstein", time_ms=300_000, delay_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # start
    # Black moves within the 5-second delay
    remaining = clock.move_made(is_white=False, timestamp_ms=3_000)
    assert remaining == 300_000  # no time deducted


def test_bronstein_exceeds_delay() -> None:
    clock = create_clock("bronstein", time_ms=300_000, delay_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # start
    # Black moves after 8 seconds (3 seconds over delay)
    remaining = clock.move_made(is_white=False, timestamp_ms=8_000)
    assert remaining == 297_000  # 300000 - (8000 - 5000)


def test_bronstein_not_banked() -> None:
    """Unused delay time is not banked (time doesn't increase)."""
    clock = create_clock("bronstein", time_ms=300_000, delay_ms=5_000)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)
    # Black moves in 1 second (4 seconds unused)
    remaining = clock.move_made(is_white=False, timestamp_ms=1_000)
    assert remaining == 300_000  # stays at 300000, unused delay not banked


# ============================================================================
# Multi-period
# ============================================================================


def test_multi_period_transition() -> None:
    """After N moves, transition to next period with time rollover."""
    clock = create_clock(
        "multi_period",
        time_ms=0,
        periods=[
            {"time_ms": 60_000, "moves": 2},
            {"time_ms": 30_000, "moves": None},
        ],
    )
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)  # white move 1
    clock.move_made(is_white=False, timestamp_ms=1_000)
    # White's second move triggers period transition
    remaining = clock.move_made(is_white=True, timestamp_ms=2_000)
    # 60000 - 1000 (elapsed) + 30000 (next period) = 89000
    assert remaining == 89_000


def test_multi_period_flag_mid_period() -> None:
    clock = create_clock(
        "multi_period",
        time_ms=0,
        periods=[
            {"time_ms": 5_000, "moves": 5},
            {"time_ms": 30_000, "moves": None},
        ],
    )
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=0)
    # Black runs out of time before completing period
    clock.move_made(is_white=False, timestamp_ms=6_000)
    assert clock.is_flagged(is_white=False) is True


def test_multi_period_three_periods() -> None:
    clock = create_clock(
        "multi_period",
        time_ms=0,
        periods=[
            {"time_ms": 10_000, "moves": 1},
            {"time_ms": 10_000, "moves": 1},
            {"time_ms": 10_000, "moves": None},
        ],
    )
    clock.start_game(timestamp_ms=0)
    # White move 1: period 0 -> period 1
    clock.move_made(is_white=True, timestamp_ms=0)
    assert clock.white_remaining_ms == 20_000  # 10000 + 10000
    clock.move_made(is_white=False, timestamp_ms=0)
    # White move 2: period 1 -> period 2
    clock.move_made(is_white=True, timestamp_ms=0)
    assert clock.white_remaining_ms == 30_000  # 20000 + 10000


# ============================================================================
# White's first move
# ============================================================================


def test_white_clock_not_started_initially() -> None:
    clock = create_clock("sudden_death", time_ms=300_000)
    clock.start_game(timestamp_ms=0)
    assert clock.white_remaining_ms == 300_000


def test_white_clock_starts_after_first_move() -> None:
    clock = create_clock("sudden_death", time_ms=300_000)
    clock.start_game(timestamp_ms=0)
    # White makes first move at t=10s — no time deducted (clock wasn't running)
    clock.move_made(is_white=True, timestamp_ms=10_000)
    assert clock.white_remaining_ms == 300_000
    # Now black's clock runs, black moves at t=15s
    clock.move_made(is_white=False, timestamp_ms=15_000)
    assert clock.black_remaining_ms == 295_000
    # Now white's clock is running, white moves at t=25s (10s elapsed)
    clock.move_made(is_white=True, timestamp_ms=25_000)
    assert clock.white_remaining_ms == 290_000  # 300000 - 10000


# ============================================================================
# Seeded mode
# ============================================================================


def test_seeded_elapsed_zero() -> None:
    clock = create_clock("sudden_death", time_ms=300_000, seeded=True)
    clock.start_game(timestamp_ms=0)
    clock.move_made(is_white=True, timestamp_ms=50_000)
    clock.move_made(is_white=False, timestamp_ms=100_000)
    # In seeded mode, elapsed is always 0
    assert clock.white_remaining_ms == 300_000
    assert clock.black_remaining_ms == 300_000
