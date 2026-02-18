# Chess

A C++ chess engine supporting two-player command-line play. Written originally circa 2012; being modernized as a foundation for new features — specifically, exposing the engine as a tool for LLMs so that a language model can play chess against a human by querying board state and submitting moves.

## Current State

Functional but dated C++ code (pre-C++11 style). Supports:

- Full chess rules: all piece types, en passant, castling, pawn promotion
- Check, checkmate, and stalemate detection
- ASCII board display
- Command-line play with move entry in `a1-b2` format
- Random move generation (`rand` command)

See [ROADMAP.md](ROADMAP.md) for known issues and the modernization plan.

## Building

```bash
make            # configure and build (debug)
RELEASE=1 make  # release build
```

Or using CMake directly:

```bash
cmake -S . -B build
cmake --build build
./build/chess
```

Tests (once added in Phase 2):

```bash
make test
```

## Playing

```
Commands:
a1-b2       move a piece from a1 to b2
moves       list all legal moves for the current player
moves a1    list legal moves for the piece at a1
rand        make a random legal move
end         resign
```

## Coordinate Conventions

The board uses a row/column integer pair internally:

- `x` = row: 0 is white's back rank, 7 is black's back rank
- `y` = column: 0 is the a-file (queenside), 7 is the h-file (kingside)

Move notation in the command-line interface uses standard algebraic file+rank (`a1`–`h8`), which maps as:

- file (`a`–`h`) → `y` (0–7)
- rank (`1`–`8`) → `x` (0–7)

## Class Overview

| Class | Responsibility |
|---|---|
| `ChessMove` | Encodes a move as a `short int` (packed 3-bit fields). Arrays terminated by `ChessMove::end` (data == 0). |
| `ChessPiece` | Abstract base for all pieces. Subclasses implement `canMove()` and `getMoves()`. |
| `Pawn`, `Rook`, `Knight`, `Bishop`, `King`, `Queen` | Concrete piece types with full rule implementations. |
| `ChessBoard` | Owns the 8×8 grid of piece pointers. Manages piece lifetime. |
| `ChessGame` | Top-level game controller: turn tracking, move legality, checkmate/stalemate. |
