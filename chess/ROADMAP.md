# Chess Modernization Roadmap

## Goal

Modernize this C++ chess engine to serve as a tool for LLMs. The target end state is a mode where an LLM can play chess against a human: the LLM uses a structured API to query board state, retrieve legal moves, submit moves, and optionally send messages to the human player.

Secondary goals: improved internal board representation, and eventually a traditional minimax/alpha-beta AI opponent.

**Target C++ standard**: C++20

## Current Known Issues

Problems in the existing code that will be addressed during modernization:

### Build / Tooling
- No build system (no CMake, no Makefile)
- No tests
- No CI
- Windows-specific `system("PAUSE")` calls in `Main.cpp`

### Correctness / Completeness
- `parseAlgMove()` in `Main.cpp` is an incomplete stub — standard algebraic notation is not fully supported
- Pawn promotion is handled in `Main.cpp` outside the game logic (not encapsulated)
- No draw detection: 50-move rule, threefold repetition not implemented
- No move history / game record

### Design / Style (pre-C++11)
- `NULL` defined manually in header instead of using `nullptr`
- Raw owning pointers throughout — no smart pointers, no RAII for pieces
- Move lists are raw heap-allocated arrays (`new ChessMove[N+1]`), caller-owned, terminated by sentinel
- C-style headers (`<stdio.h>`, `<string.h>`) instead of `<cstdio>`, `<cstring>`
- Manual `char` arrays instead of `std::string`
- Local `int abs(int)` definition in `chess.cpp` shadows / conflicts with `<cstdlib>`
- No `override` keyword on virtual method overrides
- Piece position computed by O(64) full-board scan on every call rather than cached

### API / Interfaces
- Board state exposed only as ASCII art (`toString()`) — no machine-readable format
- No FEN serialization/deserialization
- No structured API suitable for external callers (LLMs or otherwise)

---

## Phases

### Phase 0: Baseline *(current)*
- [x] Write README and ROADMAP
- Document coordinate conventions and class overview
- Identify all known issues (above)

### Phase 1: Build System & Compilation *(current)*
- [x] Add `CMakeLists.txt` targeting C++20 (separates `chess_lib` from CLI)
- [x] Add convenience `Makefile` wrapping CMake
- [x] Confirm clean compilation with modern compiler (GCC 15, C++20)
- [x] Fix initialization-order bug in `ChessGame` (member declaration order matched init list)
- Remaining `Main.cpp` warnings are from the incomplete `parseAlgMove` stub — tracked for Phase 5/6

### Phase 2: Testing
- Add [Catch2 v3](https://github.com/catchorg/Catch2) via CMake `FetchContent`
- Write unit tests covering core logic:
  - Move generation for each piece type
  - Check / checkmate / stalemate detection
  - En passant and castling
  - Pawn promotion
- Target ≥ 80% line coverage on `chess.cpp`

### Phase 3: CI
- Add GitHub Actions workflow to build and run tests on push and pull request

### Phase 4: Documentation
- Docstrings on all non-trivial methods
- Inline comments on non-obvious logic (castling check path, en passant expiry, etc.)

### Phase 5: Code Freshening *(no behavior changes)*
- `NULL` → `nullptr` throughout
- C-style headers → C++ equivalents (`<cstdio>`, `<cstring>`, etc.)
- `char` arrays → `std::string` where straightforward
- Add `override` to all virtual overrides
- Remove `system("PAUSE")`; replace with portable alternatives
- Remove local `abs()` definition; use `<cstdlib>` or `<cmath>`
- Add `clang-format` config and format all files
- Add `clang-tidy` to CI

### Phase 6: Architectural Improvements
- Replace raw owning pointers with `std::unique_ptr<ChessPiece>`
- Store piece position on the piece (avoid O(64) scan); keep board as secondary index
- Replace raw move arrays with `std::vector<ChessMove>`
- Encapsulate pawn promotion within `ChessGame` (remove from `Main.cpp`)
- Complete algebraic notation parsing
- Add move history (`std::vector<ChessMove>` in `ChessGame`)
- Evaluate bitboard representation for move generation (research spike)
- Add 50-move rule and threefold repetition draw detection

### Phase 7: LLM Tool API
- FEN serialization and deserialization
- Structured board state output (JSON or equivalent)
- Clean public C++ API:
  - `get_board_fen() -> std::string`
  - `get_legal_moves() -> std::vector<ChessMove>`
  - `make_move(from, to) -> Result`
  - `send_message(text) -> void`
- Design suitable for subprocess/FFI use or direct embedding

### Phase 8: LLM Player Mode
- Integration with Anthropic API (Claude as the LLM player)
- Human vs. LLM game loop
- LLM receives board state + legal moves; responds with a move and optional message
- Optional: LLM vs. LLM mode
