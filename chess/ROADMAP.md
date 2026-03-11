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

### Phase 2: Testing *(current)*
- [x] Add Catch2 v3 via CMake `FetchContent`
- [x] Write 63 unit tests covering: move encoding, initial position, all piece types,
  en passant, castling, check/checkmate/stalemate, game rules
- [x] Add `make test` and `make coverage` targets
- 100% of tests passing
- Coverage measurement pending CI setup (Phase 3)

### Phase 3: CI
- Add GitHub Actions workflow to build and run tests on push and pull request

### Phase 4: Documentation
- [x] Docstrings on all non-trivial methods
- [x] Inline comments on non-obvious logic (castling check path, en passant expiry, chkchk mutation pattern, coordinate convention, magic numbers in toString)

### Phase 5: Code Freshening *(no behavior changes)*
- [x] `NULL` → `nullptr` throughout
- [x] C-style headers → C++ equivalents (`<cstdio>`, `<cstring>`, `<ctime>`, `<iostream>`, `<string>`)
- [x] `char` arrays → `std::string` where straightforward (`ChessBoard::szForm`, `Main.cpp` input)
- [x] Add `override` to all virtual overrides (including destructors)
- [x] Remove `system("PAUSE")`
- [x] Remove local `abs()` definition; use `std::abs` from `<cstdlib>`
- [x] Add `.clang-format` config (Google-based, 4-space indent) and format all files
- [x] Add `.clang-tidy` config and clang-tidy step to CI
- [x] Remove Hungarian-notation prefixes (`sz`, `aap`, `f_`) from all identifiers
- [x] Rename coordinate locals to single-char names (`cx`/`cy` for current position,
  `ox`/`oy` for origin); rename other locals to descriptive names (`diagDiff`, `diagSum`,
  `capturedPawn`, `input`, `kingSide`)

### Phase 6: Architectural Improvements

#### ChessMove redesign *(done — PR #10, merged 2026-02-20)*
- [x] `ChessMove` plain `int8_t` fields replacing packed `short int`
- [x] Promotion field (`PieceType`) on `ChessMove` (approach 1: in-move)
- [x] UB audit after ChessMove changes (assert on King::move() castling dynamic_cast)

#### Vector moves + promotion encapsulation *(done — PR #11, merged 2026-03-11)*
- [x] Replace raw move arrays with `std::vector<ChessMove>`
- [x] Encapsulate pawn promotion within `ChessGame` (remove from `Main.cpp`)

#### unique_ptr + position caching *(done — PR #12, merged 2026-03-11)*
- [x] Replace raw owning pointers with `std::unique_ptr<ChessPiece>`
- [x] Store piece position on the piece (avoid O(64) scan); keep board as secondary index

#### Raw pointer cleanup *(done — PR #13, merged 2026-03-11)*
- [x] Replace `ChessBoard*` back-references with `ChessBoard&` references

#### Remaining improvements (in order)
1. Add move history (`std::vector<ChessMove>` in `ChessGame`) — PR 6e
2. FEN serialization/deserialization (pull forward from Phase 7) — enables threefold repetition
3. 50-move rule and threefold repetition draw detection — uses move history + FEN
4. Complete algebraic notation parsing — CLI improvement, independent
5. Evaluate bitboard representation for move generation (research spike, optional)

### Phase 7: LLM Tool API
- FEN serialization and deserialization
- Structured board state output (JSON or equivalent)
- Clean public C++ API (camelCase, consistent with existing codebase):
  - `getBoardFen() -> std::string`
  - `getLegalMoves() -> std::vector<ChessMove>`
  - `makeMove(from, to) -> Result`
  - `sendMessage(text) -> void`
- Design suitable for subprocess/FFI use or direct embedding

### Phase 8: Fix remaining known issues
- Address known issues not requiring substantial rework (PURPOSE.md step 7)
- Resolve any test failures uncovered during Phase 2
- Research spike: traditional minimax/alpha-beta AI opponent (scope TBD)

### Phase 9: LLM Player Mode
- Integration with Anthropic API (Claude as the LLM player)
- Human vs. LLM game loop
- LLM receives board state + legal moves; responds with a move and optional message
- Optional: LLM vs. LLM mode

---

## N-Version Cross-Validation

### Motivation

Rather than relying on a single implementation as the source of truth, we maintain two
independent implementations of the chess engine (potentially in different languages or by
different authors/LLMs). Instead of running both versions live in production, we use a
test harness — including property-based / generative testing — to produce a large selection
of inputs and compare the outputs of both implementations. Any disagreement is flagged for
resolution.

This is a variation of [N-version programming](https://en.wikipedia.org/wiki/N-version_programming)
adapted for an LLM-assisted development workflow: the two versions serve as mutual
validators, catching bugs that conventional tests might miss.

### Tested Surface

The primary tested surface is the **MCP (Model Context Protocol) server** interface.
Both implementations expose the same MCP tool interface, and the test harness invokes
identical requests against each, comparing responses.

Secondary tested surfaces:
- **Algebraic game replay**: feed the same sequence of algebraic moves to both engines
  and compare board state after each move (FEN, legal moves, game status).
- **Legal move generation**: from identical positions, both engines must produce the same
  set of legal moves (order-independent).
- **Game status detection**: check, checkmate, stalemate, draw conditions must agree.

### Disagreement Resolution

When the two implementations produce different outputs for the same input:

1. **Automated triage**: Log the disagreement with full input, both outputs, and a diff.
2. **LLM arbitration (fast path)**: Two LLM instances (one per implementation) are each
   shown the disagreement and the relevant spec section. If both quickly converge on
   which output is correct (or both identify a bug in their own implementation), the
   fix is applied automatically and the test re-run.
3. **Manager arbitration (slow path)**: If the LLMs cannot agree, or if the disagreement
   involves an ambiguity in the spec, the issue is escalated — e.g. filed as a GitHub
   issue for human review. The spec is then clarified to prevent future ambiguity.

### Chess Engine Specification

A detailed, unambiguous specification is a prerequisite for N-version testing. Without it,
disagreements cannot be objectively resolved — there is no ground truth to appeal to.

The spec must cover at minimum:

#### Board & Coordinate System
- Board representation conventions (rank/file naming, internal coordinate mapping)
- Initial position (standard chess starting position, expressed in FEN)

#### Piece Movement Rules
- Legal moves for each piece type, including edge cases:
  - Pawn: single push, double push from starting rank, diagonal capture, en passant
    (when available, when it expires), promotion (to Q/R/B/N, mandatory on reaching
    back rank)
  - Knight: L-shaped movement, not blocked by intervening pieces
  - Bishop: diagonal sliding, blocked by first piece in path
  - Rook: orthogonal sliding, blocked by first piece in path
  - Queen: combination of bishop + rook movement
  - King: one square in any direction, castling (kingside and queenside)
- Castling conditions: king and rook have not moved, no pieces between them, king not
  in check, king does not pass through or land on an attacked square
- For each rule, the spec must state the exact preconditions and postconditions so that
  correctness is mechanically verifiable

#### Check, Checkmate, Stalemate
- Definition of check (king attacked by opponent piece)
- A move is legal only if it does not leave the player's own king in check
- Checkmate: player to move is in check and has no legal moves
- Stalemate: player to move is not in check and has no legal moves

#### Draw Conditions
- Stalemate (as above)
- 50-move rule: 50 full moves (100 half-moves) with no pawn move and no capture
- Threefold repetition: same position (board, side to move, castling rights, en passant
  target square) occurs three times (not necessarily consecutively)
- Insufficient material (optional — king vs king, king+bishop vs king, king+knight vs
  king, king+bishop vs king+bishop with same-colored bishops)

#### FEN Representation
- Full FEN string format: piece placement, active color, castling availability,
  en passant target square, halfmove clock, fullmove number
- Exact serialization rules (e.g. rank separator, empty-square run-length encoding)
- Deserialization must round-trip: `parse(serialize(position)) == position`

#### Algebraic Notation
- Standard Algebraic Notation (SAN) for move input/output
- Disambiguation rules (when two pieces of the same type can reach the same square)
- Special notation: `O-O` (kingside castling), `O-O-O` (queenside castling),
  `=Q` (promotion), `+` (check), `#` (checkmate), `e.p.` (en passant, optional)
- Long algebraic notation as an alternative (unambiguous: `e2e4`, `e1g1`)

#### MCP Tool Interface
- Exact tool names, parameter schemas (JSON Schema), and response schemas
- Error response format for illegal moves, invalid positions, etc.
- Idempotency and state management semantics (does each call operate on a session?)

#### Game Lifecycle
- How a game is created (new game, or from a FEN position)
- Turn order enforcement
- How the game ends (checkmate, stalemate, draw claim, resignation)
- What state is returned after each move (updated FEN, game status, legal moves for
  next player)

### Test Harness Design

The test harness is a standalone program (likely Python) that:

1. **Generates inputs** using a combination of:
   - Predefined positions (from a curated test suite)
   - Random legal game play-throughs (random moves from the initial position)
   - Property-based testing (e.g. Hypothesis) to explore edge cases: positions near
     castling, en passant, promotion, 50-move boundary, repetition
   - Fuzzing of FEN strings and move inputs to test error handling
2. **Sends identical requests** to both implementations (via MCP protocol or direct API)
3. **Compares outputs** field-by-field, with order-independent comparison for move lists
4. **Reports disagreements** with full context for triage

### Implementation Plan

1. **Write the spec** (can begin before the second implementation exists — the spec also
   benefits the primary implementation by making expected behavior explicit)
2. **Build the MCP server** for the C++ implementation (Phase 7 prerequisite)
3. **Build or commission the second implementation** (could be in Python, Rust, or another
   language; could be written by a different LLM)
4. **Build the test harness** with property-based test generation
5. **Run cross-validation** and resolve disagreements iteratively
6. **Maintain the spec** as the authoritative reference — update it whenever an ambiguity
   is discovered and resolved
