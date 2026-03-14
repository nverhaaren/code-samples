// Tests for the chess engine (chess.cpp / chess.h).
// Run via: make test
// Coverage: make coverage

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

#include "chess.h"

// ============================================================================
// Helper: set up a custom board position from scratch.
//
// Usage:
//   CustomBoard cb;
//   cb.place(0, 4, new King(WHITE, cb.b));
//   cb.place(7, 4, new King(BLACK, cb.b));
//   cb.activate();   // enable rules and start playing
//   cb.game.makeMove(...);
// ============================================================================
struct CustomBoard {
    ChessGame game;
    ChessBoard& b;

    CustomBoard() : b(game.getPieceBoard()) {
        game.setRules(false);
        for (int x = 0; x < 8; x++)
            for (int y = 0; y < 8; y++) game.setPiece(x, y, nullptr);
        // Clear all castling rights — tests that need castling must set them.
        b.setCastlingRight(true, true, false);
        b.setCastlingRight(true, false, false);
        b.setCastlingRight(false, true, false);
        b.setCastlingRight(false, false, false);
    }

    void place(int x, int y, ChessPiece* piece) {
        game.setPiece(x, y, std::unique_ptr<ChessPiece>(piece));
    }

    // Call after placing all pieces to enable normal rules.
    void activate() { game.setRules(true); }
};

// ============================================================================
// ChessMove
// ============================================================================

TEST_CASE("ChessMove: default constructor is end sentinel", "[ChessMove]") {
    ChessMove cm;
    REQUIRE(cm.isEnd());
}

TEST_CASE("ChessMove::end is end sentinel", "[ChessMove]") { REQUIRE(ChessMove::end.isEnd()); }

TEST_CASE("ChessMove: coordinate encoding roundtrips for all squares", "[ChessMove]") {
    for (int sx = 0; sx < 8; sx++)
        for (int sy = 0; sy < 8; sy++)
            for (int ex = 0; ex < 8; ex++)
                for (int ey = 0; ey < 8; ey++) {
                    ChessMove cm(sx, sy, ex, ey);
                    REQUIRE(cm.getStartX() == sx);
                    REQUIRE(cm.getStartY() == sy);
                    REQUIRE(cm.getEndX() == ex);
                    REQUIRE(cm.getEndY() == ey);
                    REQUIRE(!cm.isEnd());
                }
}

TEST_CASE("ChessMove: copy constructor preserves data", "[ChessMove]") {
    ChessMove orig(3, 4, 5, 6);
    ChessMove copy(orig);
    REQUIRE(copy.getStartX() == 3);
    REQUIRE(copy.getStartY() == 4);
    REQUIRE(copy.getEndX() == 5);
    REQUIRE(copy.getEndY() == 6);
    REQUIRE(!copy.isEnd());
}

TEST_CASE("ChessMove: assignment operator", "[ChessMove]") {
    ChessMove cm1(1, 2, 3, 4);
    ChessMove cm2;
    REQUIRE(cm2.isEnd());
    cm2 = cm1;
    REQUIRE(cm2.getStartX() == 1);
    REQUIRE(!cm2.isEnd());
}

TEST_CASE("ChessMove: self-assignment is safe", "[ChessMove]") {
    ChessMove cm(1, 2, 3, 4);
    cm = cm;
    REQUIRE(cm.getStartX() == 1);
}

TEST_CASE("ChessMove: toString format is 'a1-b2'", "[ChessMove]") {
    // a=y=0 rank1=x=0, b=y=1 rank2=x=1
    ChessMove cm(0, 0, 1, 1);
    REQUIRE(std::string(cm.toString()) == "a1-b2");
}

TEST_CASE("ChessMove: toString for various squares", "[ChessMove]") {
    ChessMove cm(7, 7, 0, 0);  // h8-a1
    REQUIRE(std::string(cm.toString()) == "h8-a1");
}

TEST_CASE("ChessMove: string constructor parses 'a1-b2'", "[ChessMove]") {
    ChessMove cm("a1-b2");
    REQUIRE(cm.getStartX() == 0);
    REQUIRE(cm.getStartY() == 0);
    REQUIRE(cm.getEndX() == 1);
    REQUIRE(cm.getEndY() == 1);
}

TEST_CASE("ChessMove: string constructor parses 'a1b2' (no separator)", "[ChessMove]") {
    ChessMove cm("a1b2");
    REQUIRE(cm.getStartX() == 0);
    REQUIRE(cm.getStartY() == 0);
    REQUIRE(cm.getEndX() == 1);
    REQUIRE(cm.getEndY() == 1);
}

TEST_CASE("ChessMove: getPromotion returns PAWN for non-promotion move", "[ChessMove]") {
    ChessMove cm(1, 2, 3, 4);
    REQUIRE(cm.getPromotion() == PAWN);
    REQUIRE(!cm.isEnd());
}

TEST_CASE("ChessMove: 5-arg constructor coordinate roundtrip and getPromotion", "[ChessMove]") {
    ChessMove cm(6, 3, 7, 3, QUEEN);
    REQUIRE(cm.getStartX() == 6);
    REQUIRE(cm.getStartY() == 3);
    REQUIRE(cm.getEndX() == 7);
    REQUIRE(cm.getEndY() == 3);
    REQUIRE(cm.getPromotion() == QUEEN);
    REQUIRE(!cm.isEnd());
}

TEST_CASE("ChessMove: toString includes promotion letter for promotion move", "[ChessMove]") {
    ChessMove q(6, 3, 7, 3, QUEEN);
    REQUIRE(std::string(q.toString()) == "d7-d8q");

    ChessMove r(6, 3, 7, 3, ROOK);
    REQUIRE(std::string(r.toString()) == "d7-d8r");

    ChessMove n(6, 3, 7, 3, KNIGHT);
    REQUIRE(std::string(n.toString()) == "d7-d8n");

    ChessMove b(6, 3, 7, 3, BISHOP);
    REQUIRE(std::string(b.toString()) == "d7-d8b");

    // Non-promotion move has no suffix
    ChessMove plain(1, 2, 3, 4);
    REQUIRE(std::string(plain.toString()) == "c2-e4");
}
// ============================================================================
// ChessBoard / ChessGame: initial position
// ============================================================================

TEST_CASE("ChessGame: white moves first", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getTurn() == WHITE);
}

TEST_CASE("ChessGame: initial white back rank piece types", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getPiece(0, 0)->getType() == ROOK);
    REQUIRE(game.getPiece(0, 1)->getType() == KNIGHT);
    REQUIRE(game.getPiece(0, 2)->getType() == BISHOP);
    REQUIRE(game.getPiece(0, 3)->getType() == QUEEN);
    REQUIRE(game.getPiece(0, 4)->getType() == KING);
    REQUIRE(game.getPiece(0, 5)->getType() == BISHOP);
    REQUIRE(game.getPiece(0, 6)->getType() == KNIGHT);
    REQUIRE(game.getPiece(0, 7)->getType() == ROOK);
}

TEST_CASE("ChessGame: initial black back rank piece types", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getPiece(7, 0)->getType() == ROOK);
    REQUIRE(game.getPiece(7, 3)->getType() == QUEEN);
    REQUIRE(game.getPiece(7, 4)->getType() == KING);
    REQUIRE(game.getPiece(7, 4)->getWhite() == BLACK);
}

TEST_CASE("ChessGame: all pieces in initial position have correct color", "[ChessGame]") {
    ChessGame game;
    for (int y = 0; y < 8; y++) {
        REQUIRE(game.getPiece(0, y)->getWhite() == WHITE);
        REQUIRE(game.getPiece(1, y)->getWhite() == WHITE);
        REQUIRE(game.getPiece(6, y)->getWhite() == BLACK);
        REQUIRE(game.getPiece(7, y)->getWhite() == BLACK);
    }
}

TEST_CASE("ChessGame: pawns on ranks 1 and 6 in initial position", "[ChessGame]") {
    ChessGame game;
    for (int y = 0; y < 8; y++) {
        REQUIRE(game.getPiece(1, y)->getType() == PAWN);
        REQUIRE(game.getPiece(6, y)->getType() == PAWN);
    }
}

TEST_CASE("ChessGame: middle rows empty in initial position", "[ChessGame]") {
    ChessGame game;
    for (int x = 2; x <= 5; x++)
        for (int y = 0; y < 8; y++) REQUIRE(game.getPiece(x, y) == NULL);
}

// ============================================================================
// Pawn
// ============================================================================

TEST_CASE("Pawn: white pawn can advance one or two squares from starting row", "[Pawn]") {
    ChessGame game;
    const ChessPiece* pawn = game.getPiece(1, 4);  // e2
    REQUIRE(pawn->getType() == PAWN);
    REQUIRE(pawn->canMove(2, 4));   // e3 — one step
    REQUIRE(pawn->canMove(3, 4));   // e4 — two steps
    REQUIRE(!pawn->canMove(4, 4));  // three steps — illegal
}

TEST_CASE("Pawn: black pawn can advance one or two squares from starting row", "[Pawn]") {
    ChessGame game;
    const ChessPiece* pawn = game.getPiece(6, 4);  // e7
    REQUIRE(pawn->canMove(5, 4));                  // e6
    REQUIRE(pawn->canMove(4, 4));                  // e5
    REQUIRE(!pawn->canMove(3, 4));
}

TEST_CASE("Pawn: cannot advance through a blocking piece", "[Pawn]") {
    CustomBoard cb;
    cb.place(1, 4, new Pawn(WHITE, true, cb.b, 1));
    cb.place(2, 4, new Rook(BLACK, false, cb.b));  // blocks square ahead
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* pawn = cb.game.getPiece(1, 4);
    REQUIRE(!pawn->canMove(2, 4));  // blocked
    REQUIRE(!pawn->canMove(3, 4));  // cannot jump over blocker
}

TEST_CASE("Pawn: double advance blocked if intermediate square occupied", "[Pawn]") {
    CustomBoard cb;
    cb.place(1, 4, new Pawn(WHITE, true, cb.b, 1));
    cb.place(3, 4, new Rook(BLACK, false, cb.b));  // destination blocked
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    // Single step to (2,4) is fine; double step to (3,4) is blocked
    const ChessPiece* pawn = cb.game.getPiece(1, 4);
    REQUIRE(pawn->canMove(2, 4));
    REQUIRE(!pawn->canMove(3, 4));
}

TEST_CASE("Pawn: diagonal capture of enemy piece", "[Pawn]") {
    CustomBoard cb;
    cb.place(2, 4, new Pawn(WHITE, true, cb.b, 1));
    cb.place(3, 5, new Rook(BLACK, false, cb.b));  // capturable
    cb.place(3, 3, new Rook(BLACK, false, cb.b));  // capturable
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* pawn = cb.game.getPiece(2, 4);
    REQUIRE(pawn->canMove(3, 5));
    REQUIRE(pawn->canMove(3, 3));
    REQUIRE(!pawn->canMove(3, 6));  // too far
}

TEST_CASE("Pawn: cannot capture on empty diagonal", "[Pawn]") {
    ChessGame game;
    const ChessPiece* pawn = game.getPiece(1, 4);
    REQUIRE(!pawn->canMove(2, 5));  // no piece to capture
    REQUIRE(!pawn->canMove(2, 3));
}

TEST_CASE("Pawn: cannot move sideways or backward", "[Pawn]") {
    ChessGame game;
    const ChessPiece* pawn = game.getPiece(1, 4);
    REQUIRE(!pawn->canMove(1, 5));
    REQUIRE(!pawn->canMove(1, 3));
    REQUIRE(!pawn->canMove(0, 4));
}

TEST_CASE("Pawn: makeMove advances pawn and clears source square", "[Pawn]") {
    ChessGame game;
    REQUIRE(game.makeMove(ChessMove(1, 4, 2, 4)));
    REQUIRE(game.getPiece(2, 4) != NULL);
    REQUIRE(game.getPiece(2, 4)->getType() == PAWN);
    REQUIRE(game.getPiece(1, 4) == NULL);
}

TEST_CASE("Pawn: getMoves returns at least one move from start", "[Pawn]") {
    ChessGame game;
    auto moves = game.getPiece(1, 4)->getMoves();
    REQUIRE(!moves.empty());
}

TEST_CASE("Pawn: en passant capture", "[Pawn]") {
    ChessGame game;
    game.makeMove(ChessMove(1, 4, 3, 4));  // e2-e4 white
    game.makeMove(ChessMove(6, 0, 5, 0));  // a7-a6 black (filler)
    game.makeMove(ChessMove(3, 4, 4, 4));  // e4-e5 white
    game.makeMove(ChessMove(6, 5, 4, 5));  // f7-f5 black double advance

    const ChessPiece* pawn = game.getPiece(4, 4);
    REQUIRE(pawn != NULL);
    REQUIRE(pawn->canMove(5, 5));  // en passant

    REQUIRE(game.makeMove(ChessMove(4, 4, 5, 5)));
    REQUIRE(game.getPiece(5, 5) != NULL);
    REQUIRE(game.getPiece(5, 5)->getType() == PAWN);
    REQUIRE(game.getPiece(4, 5) == NULL);  // captured pawn removed
}

TEST_CASE("Pawn: en passant opportunity expires after opponent's move", "[Pawn]") {
    ChessGame game;
    game.makeMove(ChessMove(1, 4, 3, 4));  // e2-e4
    game.makeMove(ChessMove(6, 0, 5, 0));  // a7-a6
    game.makeMove(ChessMove(3, 4, 4, 4));  // e4-e5
    game.makeMove(ChessMove(6, 5, 4, 5));  // f7-f5 (en passant opportunity)
    game.makeMove(ChessMove(1, 0, 2, 0));  // a2-a3 white passes
    game.makeMove(ChessMove(5, 0, 4, 0));  // a6-a5 black passes

    // En passant is no longer available
    const ChessPiece* pawn = game.getPiece(4, 4);
    REQUIRE(pawn != NULL);
    REQUIRE(!pawn->canMove(5, 5));
}

TEST_CASE("Pawn: en passant illegal when horizontally pinned (white)", "[Pawn][EnPassantPin]") {
    // White King a5 (row 4, col 0), White Pawn d5 (row 4, col 3),
    // Black Pawn e5 (row 4, col 4) with enPassant=true, Black Rook h5 (row 4, col 7).
    // dxe6 e.p. would remove both pawns from rank 5, exposing king to rook.
    CustomBoard cb;
    cb.place(0, 4, new King(BLACK, cb.b));                // black king e1 (not in the way)
    cb.place(4, 0, new King(WHITE, cb.b));                // white king a5
    auto* bpawn = new Pawn(BLACK, false, cb.b, 4);
    bpawn->setEnPassant(true);
    cb.place(4, 4, bpawn);                                // black pawn e5 (just double-pushed)
    cb.place(4, 3, new Pawn(WHITE, false, cb.b, 3));      // white pawn d5
    cb.place(4, 7, new Rook(BLACK, false, cb.b));         // black rook h5
    cb.activate();

    const ChessPiece* wpawn = cb.game.getPiece(4, 3);
    REQUIRE(wpawn != nullptr);
    // The en passant capture dxe6 (row 4 col 3 -> row 5 col 4) must be illegal.
    REQUIRE_FALSE(wpawn->canMove(5, 4));
}

TEST_CASE("Pawn: en passant illegal when horizontally pinned (black)", "[Pawn][EnPassantPin]") {
    // Black King h4 (row 3, col 7), Black Pawn e4 (row 3, col 4),
    // White Pawn d4 (row 3, col 3) with enPassant=true, White Rook a4 (row 3, col 0).
    // exd3 e.p. would remove both pawns from rank 4, exposing black king to rook.
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));                // white king e1 (not in the way)
    cb.place(3, 7, new King(BLACK, cb.b));                // black king h4
    auto* wpawn_ep = new Pawn(WHITE, false, cb.b, 3);
    wpawn_ep->setEnPassant(true);
    cb.place(3, 3, wpawn_ep);                             // white pawn d4 (just double-pushed)
    cb.place(3, 4, new Pawn(BLACK, false, cb.b, 4));      // black pawn e4
    cb.place(3, 0, new Rook(WHITE, false, cb.b));         // white rook a4
    cb.activate();

    const ChessPiece* bpawn = cb.game.getPiece(3, 4);
    REQUIRE(bpawn != nullptr);
    // The en passant capture exd3 (row 3 col 4 -> row 2 col 3) must be illegal.
    REQUIRE_FALSE(bpawn->canMove(2, 3));
}

TEST_CASE("Pawn: en passant legal when not pinned (regression)", "[Pawn][EnPassantPin]") {
    // White King a1 (row 0, col 0), White Pawn d5 (row 4, col 3),
    // Black Pawn e5 (row 4, col 4) with enPassant=true.
    // No rook on the rank — en passant should be legal.
    CustomBoard cb;
    cb.place(7, 4, new King(BLACK, cb.b));                // black king e8
    cb.place(0, 0, new King(WHITE, cb.b));                // white king a1
    auto* bpawn = new Pawn(BLACK, false, cb.b, 4);
    bpawn->setEnPassant(true);
    cb.place(4, 4, bpawn);                                // black pawn e5 (just double-pushed)
    cb.place(4, 3, new Pawn(WHITE, false, cb.b, 3));      // white pawn d5
    cb.activate();

    const ChessPiece* wpawn = cb.game.getPiece(4, 3);
    REQUIRE(wpawn != nullptr);
    REQUIRE(wpawn->canMove(5, 4));  // en passant should be legal
}

TEST_CASE("Pawn: en passant legal when king not on same rank", "[Pawn][EnPassantPin]") {
    // White King e1 (row 0, col 4), White Pawn d5 (row 4, col 3),
    // Black Pawn e5 (row 4, col 4) with enPassant=true,
    // Black Rook h5 (row 4, col 7) — same rank as pawns but king is NOT on rank.
    // Removing both pawns doesn't expose king. En passant should be legal.
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));                // white king e1
    cb.place(7, 4, new King(BLACK, cb.b));                // black king e8
    auto* bpawn = new Pawn(BLACK, false, cb.b, 4);
    bpawn->setEnPassant(true);
    cb.place(4, 4, bpawn);                                // black pawn e5
    cb.place(4, 3, new Pawn(WHITE, false, cb.b, 3));      // white pawn d5
    cb.place(4, 7, new Rook(BLACK, false, cb.b));         // black rook h5 (same rank, king not on rank)
    cb.activate();

    const ChessPiece* wpawn = cb.game.getPiece(4, 3);
    REQUIRE(wpawn != nullptr);
    // King is on row 0, not row 4 — removing both pawns doesn't expose king.
    REQUIRE(wpawn->canMove(5, 4));
}

// ============================================================================
// Rook
// ============================================================================

TEST_CASE("Rook: moves along rank and file", "[Rook]") {
    CustomBoard cb;
    cb.place(3, 3, new Rook(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* rook = cb.game.getPiece(3, 3);
    REQUIRE(rook->canMove(3, 0));
    REQUIRE(rook->canMove(3, 7));
    REQUIRE(rook->canMove(0, 3));
    REQUIRE(rook->canMove(6, 3));
    REQUIRE(!rook->canMove(4, 4));  // diagonal
    REQUIRE(!rook->canMove(3, 3));  // null move
}

TEST_CASE("Rook: blocked by friendly piece", "[Rook]") {
    CustomBoard cb;
    cb.place(3, 3, new Rook(WHITE, false, cb.b));
    cb.place(3, 5, new Pawn(WHITE, true, cb.b, 1));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* rook = cb.game.getPiece(3, 3);
    REQUIRE(rook->canMove(3, 4));
    REQUIRE(!rook->canMove(3, 5));  // friendly
    REQUIRE(!rook->canMove(3, 6));  // past friendly
}

TEST_CASE("Rook: can capture enemy, cannot jump over it", "[Rook]") {
    CustomBoard cb;
    cb.place(3, 3, new Rook(WHITE, false, cb.b));
    cb.place(3, 6, new Pawn(BLACK, true, cb.b, 1));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* rook = cb.game.getPiece(3, 3);
    REQUIRE(rook->canMove(3, 6));
    REQUIRE(!rook->canMove(3, 7));
}

TEST_CASE("Rook: getMoves covers all reachable squares from open center", "[Rook]") {
    CustomBoard cb;
    cb.place(3, 3, new Rook(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    auto moves = cb.game.getPiece(3, 3)->getMoves();
    // From (3,3) open board: 7 along rank + 7 along file - 2 occupied (King)
    // In practice: along x=3 (7 squares) + along y=3 (7 squares) = 14, minus
    // squares blocked by kings at (0,0) and (7,7) — those aren't on rank 3 or file 3
    // so 14 moves expected
    REQUIRE((int)moves.size() == 14);
}

// ============================================================================
// Knight
// ============================================================================

TEST_CASE("Knight: all eight L-shape moves from open center", "[Knight]") {
    CustomBoard cb;
    cb.place(4, 4, new Knight(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* knight = cb.game.getPiece(4, 4);
    REQUIRE(knight->canMove(5, 6));
    REQUIRE(knight->canMove(5, 2));
    REQUIRE(knight->canMove(3, 6));
    REQUIRE(knight->canMove(3, 2));
    REQUIRE(knight->canMove(6, 5));
    REQUIRE(knight->canMove(6, 3));
    REQUIRE(knight->canMove(2, 5));
    REQUIRE(knight->canMove(2, 3));
}

TEST_CASE("Knight: cannot move in non-L-shape", "[Knight]") {
    CustomBoard cb;
    cb.place(4, 4, new Knight(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* knight = cb.game.getPiece(4, 4);
    REQUIRE(!knight->canMove(5, 5));
    REQUIRE(!knight->canMove(4, 5));
    REQUIRE(!knight->canMove(6, 6));
}

TEST_CASE("Knight: can jump over intervening pieces", "[Knight]") {
    // Starting position: knight at (0,1) surrounded by pawns on row 1
    ChessGame game;
    const ChessPiece* knight = game.getPiece(0, 1);
    REQUIRE(knight->getType() == KNIGHT);
    REQUIRE(knight->canMove(2, 0));
    REQUIRE(knight->canMove(2, 2));
}

TEST_CASE("Knight: getMoves from corner returns 2 moves", "[Knight]") {
    CustomBoard cb;
    cb.place(0, 0, new Knight(WHITE, false, cb.b));
    cb.place(4, 4, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    auto moves = cb.game.getPiece(0, 0)->getMoves();
    REQUIRE((int)moves.size() == 2);  // (1,2) and (2,1)
}

// ============================================================================
// Bishop
// ============================================================================

TEST_CASE("Bishop: moves diagonally", "[Bishop]") {
    CustomBoard cb;
    cb.place(4, 4, new Bishop(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 1, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* bishop = cb.game.getPiece(4, 4);
    REQUIRE(bishop->canMove(5, 5));
    REQUIRE(bishop->canMove(6, 6));
    REQUIRE(bishop->canMove(3, 3));
    REQUIRE(bishop->canMove(2, 2));
    REQUIRE(bishop->canMove(5, 3));
    REQUIRE(bishop->canMove(3, 5));
}

TEST_CASE("Bishop: cannot move horizontally or vertically", "[Bishop]") {
    CustomBoard cb;
    cb.place(4, 4, new Bishop(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* bishop = cb.game.getPiece(4, 4);
    REQUIRE(!bishop->canMove(4, 5));
    REQUIRE(!bishop->canMove(5, 4));
}

TEST_CASE("Bishop: blocked by friendly piece", "[Bishop]") {
    CustomBoard cb;
    cb.place(4, 4, new Bishop(WHITE, false, cb.b));
    cb.place(6, 6, new Pawn(WHITE, true, cb.b, 1));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 1, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* bishop = cb.game.getPiece(4, 4);
    REQUIRE(bishop->canMove(5, 5));
    REQUIRE(!bishop->canMove(6, 6));
    REQUIRE(!bishop->canMove(7, 7));
}

TEST_CASE("Bishop: can capture enemy piece", "[Bishop]") {
    CustomBoard cb;
    cb.place(4, 4, new Bishop(WHITE, false, cb.b));
    cb.place(6, 6, new Pawn(BLACK, true, cb.b, 1));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 1, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* bishop = cb.game.getPiece(4, 4);
    REQUIRE(bishop->canMove(6, 6));
    REQUIRE(!bishop->canMove(7, 7));  // past enemy
}

// ============================================================================
// King
// ============================================================================

TEST_CASE("King: can step to any adjacent square", "[King]") {
    CustomBoard cb;
    cb.place(4, 4, new King(WHITE, cb.b));
    cb.place(7, 0, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* king = cb.game.getPiece(4, 4);
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            INFO("dx=" << dx << " dy=" << dy);
            REQUIRE(king->canMove(4 + dx, 4 + dy));
        }
    REQUIRE(!king->canMove(4 + 2, 4));
}

TEST_CASE("King: cannot step into check", "[King]") {
    CustomBoard cb;
    cb.place(4, 4, new King(WHITE, cb.b));
    cb.place(3, 0, new Rook(BLACK, false, cb.b));  // controls entire rank 3
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* king = cb.game.getPiece(4, 4);
    REQUIRE(!king->canMove(3, 4));  // rank 3 is controlled by rook
    REQUIRE(!king->canMove(3, 3));
    REQUIRE(!king->canMove(3, 5));
    REQUIRE(king->canMove(5, 4));  // safe
}

TEST_CASE("King: inCheck when attacked", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(5, 4, new Rook(BLACK, false, cb.b));
    cb.place(7, 0, new King(BLACK, cb.b));
    cb.activate();

    const King* king = dynamic_cast<const King*>(cb.game.getPiece(0, 4));
    REQUIRE(king != NULL);
    REQUIRE(king->inCheck());
}

TEST_CASE("King: not inCheck when not attacked", "[King]") {
    ChessGame game;
    const King* king = dynamic_cast<const King*>(game.getPiece(0, 4));
    REQUIRE(king != NULL);
    REQUIRE(!king->inCheck());
}

TEST_CASE("King: kingside castling moves king to g-file and rook to f-file", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, true, true);  // white kingside
    cb.activate();

    REQUIRE(cb.game.getPiece(0, 4)->canMove(0, 6));
    REQUIRE(cb.game.makeMove(ChessMove(0, 4, 0, 6)));
    REQUIRE(cb.game.getPiece(0, 6)->getType() == KING);
    REQUIRE(cb.game.getPiece(0, 5)->getType() == ROOK);
    REQUIRE(cb.game.getPiece(0, 7) == NULL);
    REQUIRE(cb.game.getPiece(0, 4) == NULL);
}

TEST_CASE("King: queenside castling moves king to c-file and rook to d-file", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, false, true);  // white queenside
    cb.activate();

    REQUIRE(cb.game.getPiece(0, 4)->canMove(0, 2));
    REQUIRE(cb.game.makeMove(ChessMove(0, 4, 0, 2)));
    REQUIRE(cb.game.getPiece(0, 2)->getType() == KING);
    REQUIRE(cb.game.getPiece(0, 3)->getType() == ROOK);
    REQUIRE(cb.game.getPiece(0, 0) == NULL);
}

TEST_CASE("King: cannot castle after king has moved", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(7, 0, new Rook(BLACK, false, cb.b));
    cb.b.setCastlingRight(true, true, true);  // white kingside
    cb.activate();

    cb.game.makeMove(ChessMove(0, 4, 0, 3));  // king moves
    cb.game.makeMove(ChessMove(7, 0, 6, 0));  // black filler
    cb.game.makeMove(ChessMove(0, 3, 0, 4));  // king returns
    cb.game.makeMove(ChessMove(6, 0, 5, 0));  // black filler

    REQUIRE(!cb.game.getPiece(0, 4)->canMove(0, 6));
}

TEST_CASE("King: cannot castle through attacked square", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(5, 5, new Rook(BLACK, false, cb.b));  // attacks (0,5) on f-file
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, true, true);  // white kingside
    cb.activate();

    REQUIRE(!cb.game.getPiece(0, 4)->canMove(0, 6));
}

TEST_CASE("King: cannot castle if rook has moved", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(7, 0, new Rook(BLACK, false, cb.b));
    cb.b.setCastlingRight(true, true, true);  // white kingside
    cb.activate();

    cb.game.makeMove(ChessMove(0, 7, 0, 6));  // rook moves
    cb.game.makeMove(ChessMove(7, 0, 6, 0));
    cb.game.makeMove(ChessMove(0, 6, 0, 7));  // rook returns
    cb.game.makeMove(ChessMove(6, 0, 5, 0));

    REQUIRE(!cb.game.getPiece(0, 4)->canMove(0, 6));
}

// ============================================================================
// Queen
// ============================================================================

TEST_CASE("Queen: combines rook and bishop movement", "[Queen]") {
    CustomBoard cb;
    cb.place(4, 4, new Queen(WHITE, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 1, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* queen = cb.game.getPiece(4, 4);
    // Rook-like
    REQUIRE(queen->canMove(4, 1));
    REQUIRE(queen->canMove(4, 7));
    REQUIRE(queen->canMove(1, 4));
    REQUIRE(queen->canMove(6, 4));
    // Bishop-like
    REQUIRE(queen->canMove(5, 5));
    REQUIRE(queen->canMove(3, 3));
    REQUIRE(queen->canMove(5, 3));
    REQUIRE(queen->canMove(3, 5));
    // Knight-move — illegal
    REQUIRE(!queen->canMove(5, 6));
    REQUIRE(!queen->canMove(4, 4));  // null move
}

// ============================================================================
// Check detection
// ============================================================================

TEST_CASE("Check: move that exposes king is illegal", "[Check]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 5, new Rook(WHITE, true, cb.b));   // pinned to king's rank
    cb.place(0, 7, new Rook(BLACK, false, cb.b));  // attacks along rank 0
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.activate();

    // White rook at (0,5) is pinned — moving off rank 0 exposes king
    const ChessPiece* rook = cb.game.getPiece(0, 5);
    REQUIRE(!rook->canMove(1, 5));  // reveals attack on king
    REQUIRE(rook->canMove(0, 6));   // stays on rank 0, king safe
}

TEST_CASE("Check: checkCheck returns true when king is in check", "[Check]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(5, 4, new Rook(BLACK, false, cb.b));
    cb.place(7, 0, new King(BLACK, cb.b));
    cb.activate();

    // checkCheck(WHITE) should return true (white king is in check)
    // Access via making a move that is illegal while in check
    const King* king = dynamic_cast<const King*>(cb.game.getPiece(0, 4));
    REQUIRE(king->inCheck());
}

// ============================================================================
// Checkmate and stalemate
// ============================================================================

TEST_CASE("Checkmate: fool's mate", "[ChessGame]") {
    ChessGame game;
    // 1. f3 e5 2. g4 Qh4#
    REQUIRE(game.makeMove(ChessMove(1, 5, 2, 5)));  // f3
    REQUIRE(game.makeMove(ChessMove(6, 4, 4, 4)));  // e5
    REQUIRE(game.makeMove(ChessMove(1, 6, 3, 6)));  // g4
    REQUIRE(game.makeMove(ChessMove(7, 3, 3, 7)));  // Qh4 — checkmate

    REQUIRE(game.checkmate(WHITE));
    REQUIRE(!game.stalemate(WHITE));
}

TEST_CASE("Stalemate: king trapped but not in check", "[ChessGame]") {
    CustomBoard cb;
    // White king at (0,0); black queen at (2,1) covers all escape squares.
    // King escape squares: (0,1) via same file as queen, (1,0) via diagonal,
    // (1,1) via same file. None of them attack (0,0) directly → not check.
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(2, 1, new Queen(BLACK, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const King* king = dynamic_cast<const King*>(cb.game.getPiece(0, 0));
    REQUIRE(!king->inCheck());
    REQUIRE(cb.game.stalemate(WHITE));
    REQUIRE(!cb.game.checkmate(WHITE));
}

TEST_CASE("Not stalemate when king has legal moves", "[ChessGame]") {
    ChessGame game;
    REQUIRE(!game.stalemate(WHITE));
    REQUIRE(!game.checkmate(WHITE));
}

// ============================================================================
// ChessGame rules
// ============================================================================

TEST_CASE("ChessGame: turn alternates after valid moves", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getTurn() == WHITE);
    REQUIRE(game.makeMove(ChessMove(1, 4, 2, 4)));
    REQUIRE(game.getTurn() == BLACK);
    REQUIRE(game.makeMove(ChessMove(6, 4, 5, 4)));
    REQUIRE(game.getTurn() == WHITE);
}

TEST_CASE("ChessGame: cannot move opponent's piece", "[ChessGame]") {
    ChessGame game;
    REQUIRE(!game.makeMove(ChessMove(6, 4, 5, 4)));  // black on white's turn
    REQUIRE(game.getTurn() == WHITE);
}

TEST_CASE("ChessGame: illegal move does not change turn", "[ChessGame]") {
    ChessGame game;
    REQUIRE(!game.makeMove(ChessMove(1, 4, 5, 4)));  // pawn cannot jump 4 rows
    REQUIRE(game.getTurn() == WHITE);
}

TEST_CASE("ChessGame: getMoves returns non-empty list at start", "[ChessGame]") {
    ChessGame game;
    auto moves = game.getMoves(WHITE);
    REQUIRE(!moves.empty());
}

TEST_CASE("ChessGame: getPieceBoard returns reference to board", "[ChessGame]") {
    ChessGame game;
    ChessBoard& b = game.getPieceBoard();
    // The reference should refer to the game's own board — verify via piece access
    REQUIRE(b.getPiece(0, 4) != nullptr);
    REQUIRE(b.getPiece(0, 4)->getType() == KING);
}

TEST_CASE("ChessPiece: constructed with board reference can query board", "[ChessPiece]") {
    CustomBoard cb;
    cb.place(3, 3, new Rook(WHITE, false, cb.b));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();

    const ChessPiece* rook = cb.game.getPiece(3, 3);
    REQUIRE(rook != nullptr);
    REQUIRE(rook->getType() == ROOK);
    // Piece can query its board: canMove works (which calls board.getPiece internally)
    REQUIRE(rook->canMove(3, 7));
    REQUIRE(rook->canMove(0, 3));
}

TEST_CASE("ChessGame: rules can be toggled", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getRules() == true);
    game.setRules(false);
    REQUIRE(game.getRules() == false);
    game.setRules(true);
    REQUIRE(game.getRules() == true);
}

// ============================================================================
// Pawn promotion
// ============================================================================

TEST_CASE("Pawn: getMoves emits 4 promotion moves at back rank", "[Pawn]") {
    CustomBoard cb;
    cb.place(6, 3, new Pawn(WHITE, false, cb.b, 1));  // white pawn at d7 (row 6, col 3)
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    auto moves = cb.game.getPiece(6, 3)->getMoves();
    // 4 promotion types for the forward move
    int promCount = 0;
    for (const auto& m : moves)
        if (m.getPromotion() != PAWN) promCount++;
    REQUIRE(promCount == 4);
}

TEST_CASE("Pawn: promotion move results in correct piece type on board", "[Pawn]") {
    CustomBoard cb;
    cb.place(6, 3, new Pawn(WHITE, false, cb.b, 1));  // d7
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    // Make a queen-promotion move
    REQUIRE(cb.game.makeMove(ChessMove(6, 3, 7, 3, QUEEN)));
    REQUIRE(cb.game.getPiece(7, 3) != nullptr);
    REQUIRE(cb.game.getPiece(7, 3)->getType() == QUEEN);
    REQUIRE(cb.game.getPiece(6, 3) == nullptr);
}

TEST_CASE("Pawn: promotion to rook works", "[Pawn]") {
    CustomBoard cb;
    cb.place(6, 3, new Pawn(WHITE, false, cb.b, 1));
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    REQUIRE(cb.game.makeMove(ChessMove(6, 3, 7, 3, ROOK)));
    REQUIRE(cb.game.getPiece(7, 3)->getType() == ROOK);
}

TEST_CASE("ChessMove: getPromotion returns promotion type for promotion move", "[ChessMove]") {
    ChessMove cm(6, 3, 7, 3, QUEEN);
    REQUIRE(cm.getPromotion() == QUEEN);
    REQUIRE(!cm.isEnd());
}

// ============================================================================
// Move History
// ============================================================================

TEST_CASE("ChessGame: history is empty at start", "[ChessGame][History]") {
    ChessGame game;
    REQUIRE(game.getHistory().empty());
}

TEST_CASE("ChessGame: history records a move after makeMove", "[ChessGame][History]") {
    ChessGame game;
    ChessMove e2e4(1, 4, 3, 4);  // e2-e4
    REQUIRE(game.makeMove(e2e4));
    REQUIRE(game.getHistory().size() == 1);
    const ChessMove& recorded = game.getHistory()[0];
    REQUIRE(recorded.getStartX() == 1);
    REQUIRE(recorded.getStartY() == 4);
    REQUIRE(recorded.getEndX() == 3);
    REQUIRE(recorded.getEndY() == 4);
}

TEST_CASE("ChessGame: history records multiple moves in order", "[ChessGame][History]") {
    ChessGame game;
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // e2-e4
    REQUIRE(game.makeMove(ChessMove(6, 4, 4, 4)));  // e7-e5
    REQUIRE(game.makeMove(ChessMove(0, 5, 3, 2)));  // Bf1-c4
    REQUIRE(game.getHistory().size() == 3);
    // First move is e2-e4
    REQUIRE(game.getHistory()[0].getStartX() == 1);
    REQUIRE(game.getHistory()[0].getEndX() == 3);
    // Second move is e7-e5
    REQUIRE(game.getHistory()[1].getStartX() == 6);
    REQUIRE(game.getHistory()[1].getEndX() == 4);
    // Third move is Bf1-c4
    REQUIRE(game.getHistory()[2].getStartX() == 0);
    REQUIRE(game.getHistory()[2].getStartY() == 5);
}

TEST_CASE("ChessGame: failed move is not recorded in history", "[ChessGame][History]") {
    ChessGame game;
    // Try to move black piece on white's turn — should fail
    REQUIRE_FALSE(game.makeMove(ChessMove(6, 4, 4, 4)));
    REQUIRE(game.getHistory().empty());
}

TEST_CASE("ChessGame: history records promotion moves with promotion type",
          "[ChessGame][History]") {
    CustomBoard cb;
    cb.place(6, 3, new Pawn(WHITE, false, cb.b, 1));  // white pawn at d7
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    REQUIRE(cb.game.makeMove(ChessMove(6, 3, 7, 3, QUEEN)));
    REQUIRE(cb.game.getHistory().size() == 1);
    REQUIRE(cb.game.getHistory()[0].getPromotion() == QUEEN);
}

TEST_CASE("ChessGame: rules-off moves are not recorded in history", "[ChessGame][History]") {
    ChessGame game;
    game.setRules(false);
    game.makeMove(ChessMove(1, 4, 3, 4));
    REQUIRE(game.getHistory().empty());
}

// ============================================================================
// FEN Serialization
// ============================================================================

TEST_CASE("ChessGame: initial position FEN", "[ChessGame][FEN]") {
    ChessGame game;
    REQUIRE(game.toFen() ==
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST_CASE("ChessGame: FEN after 1. e4", "[ChessGame][FEN]") {
    ChessGame game;
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // e2-e4
    REQUIRE(game.toFen() ==
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST_CASE("ChessGame: FEN after 1. e4 e5", "[ChessGame][FEN]") {
    ChessGame game;
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // e2-e4
    REQUIRE(game.makeMove(ChessMove(6, 4, 4, 4)));  // e7-e5
    REQUIRE(game.toFen() ==
            "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
}

TEST_CASE("ChessGame: FEN castling rights removed after king moves", "[ChessGame][FEN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.b.setCastlingRight(true, true, true);
    cb.b.setCastlingRight(true, false, true);
    cb.activate();
    // Move white king
    REQUIRE(cb.game.makeMove(ChessMove(0, 4, 0, 5)));
    std::string fen = cb.game.toFen();
    // After king moves, white has no castling rights; black has none either
    // FEN castling field should be "-"
    // Extract castling field (3rd space-separated field)
    int spaces = 0;
    std::string castling;
    for (char c : fen) {
        if (c == ' ') { spaces++; continue; }
        if (spaces == 2) castling += c;
        if (spaces > 2) break;
    }
    REQUIRE(castling == "-");
}

TEST_CASE("ChessGame: FEN castling rights after only kingside rook moves", "[ChessGame][FEN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.b.setCastlingRight(true, true, true);
    cb.b.setCastlingRight(true, false, true);
    cb.activate();
    // Move kingside rook
    REQUIRE(cb.game.makeMove(ChessMove(0, 7, 1, 7)));
    std::string fen = cb.game.toFen();
    // White should still have queenside castling (Q), but not kingside
    int spaces = 0;
    std::string castling;
    for (char c : fen) {
        if (c == ' ') { spaces++; continue; }
        if (spaces == 2) castling += c;
        if (spaces > 2) break;
    }
    REQUIRE(castling == "Q");
}

TEST_CASE("ChessGame: FEN with empty board except kings", "[ChessGame][FEN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.activate();
    REQUIRE(cb.game.toFen() == "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
}

TEST_CASE("ChessGame: FEN halfmove clock resets on pawn move", "[ChessGame][FEN]") {
    ChessGame game;
    // 1. Nf3 Nf6  (two non-pawn, non-capture moves → halfmove clock = 2)
    REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // Ng1-f3
    REQUIRE(game.makeMove(ChessMove(7, 6, 5, 5)));  // Ng8-f6
    std::string fen = game.toFen();
    // Extract halfmove clock (5th space-separated field)
    int spaces = 0;
    std::string halfmove;
    for (char c : fen) {
        if (c == ' ') { spaces++; continue; }
        if (spaces == 4) halfmove += c;
        if (spaces > 4) break;
    }
    REQUIRE(halfmove == "2");

    // Now a pawn move resets halfmove clock to 0
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // e2-e4
    fen = game.toFen();
    spaces = 0;
    halfmove.clear();
    for (char c : fen) {
        if (c == ' ') { spaces++; continue; }
        if (spaces == 4) halfmove += c;
        if (spaces > 4) break;
    }
    REQUIRE(halfmove == "0");
}

TEST_CASE("ChessGame: FEN halfmove clock resets on capture", "[ChessGame][FEN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(3, 3, new Knight(WHITE, false, cb.b));
    // Knight from (3,3) can reach (4,1) — place black piece there for capture.
    cb.place(4, 1, new Knight(BLACK, false, cb.b));
    cb.activate();
    // Non-capture king moves to build up the clock.
    REQUIRE(cb.game.makeMove(ChessMove(0, 4, 0, 5)));  // halfmove = 1
    REQUIRE(cb.game.makeMove(ChessMove(7, 4, 7, 5)));  // halfmove = 2
    // White knight captures black knight at (4,1): halfmove resets to 0
    REQUIRE(cb.game.makeMove(ChessMove(3, 3, 4, 1)));
    std::string fen = cb.game.toFen();
    int spaces = 0;
    std::string halfmove;
    for (char c : fen) {
        if (c == ' ') { spaces++; continue; }
        if (spaces == 4) halfmove += c;
        if (spaces > 4) break;
    }
    REQUIRE(halfmove == "0");
}

TEST_CASE("ChessGame: FEN fullmove number increments after black moves", "[ChessGame][FEN]") {
    ChessGame game;
    // Initial: fullmove = 1
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // 1. e4
    // After white's first move, still fullmove 1
    std::string fen = game.toFen();
    REQUIRE(fen.back() == '1');
    REQUIRE(game.makeMove(ChessMove(6, 4, 4, 4)));  // 1... e5
    // After black's first move, fullmove = 2
    fen = game.toFen();
    REQUIRE(fen.back() == '2');
    REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // 2. Nf3
    REQUIRE(game.makeMove(ChessMove(7, 1, 5, 2)));  // 2... Nc6
    fen = game.toFen();
    REQUIRE(fen.back() == '3');
}

// ============================================================================
// SAN (Standard Algebraic Notation) Parsing
// ============================================================================

TEST_CASE("ChessGame: parseSan pawn move e4", "[ChessGame][SAN]") {
    ChessGame game;
    ChessMove move = game.parseSan("e4");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartX() == 1);
    REQUIRE(move.getStartY() == 4);
    REQUIRE(move.getEndX() == 3);
    REQUIRE(move.getEndY() == 4);
}

TEST_CASE("ChessGame: parseSan pawn move e3 (single push)", "[ChessGame][SAN]") {
    ChessGame game;
    ChessMove move = game.parseSan("e3");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartX() == 1);
    REQUIRE(move.getStartY() == 4);
    REQUIRE(move.getEndX() == 2);
    REQUIRE(move.getEndY() == 4);
}

TEST_CASE("ChessGame: parseSan knight move Nf3", "[ChessGame][SAN]") {
    ChessGame game;
    ChessMove move = game.parseSan("Nf3");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartY() == 6);  // g-file knight
    REQUIRE(move.getEndX() == 2);
    REQUIRE(move.getEndY() == 5);
}

TEST_CASE("ChessGame: parseSan piece capture Bxd6", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(3, 5, new Bishop(WHITE, false, cb.b));  // Bishop on f4
    cb.place(5, 3, new Pawn(BLACK, false, cb.b, 1));  // Pawn on d6
    cb.activate();
    ChessMove move = cb.game.parseSan("Bxd6");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getEndX() == 5);
    REQUIRE(move.getEndY() == 3);
}

TEST_CASE("ChessGame: parseSan pawn capture exd5", "[ChessGame][SAN]") {
    ChessGame game;
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // 1. e4
    REQUIRE(game.makeMove(ChessMove(6, 3, 4, 3)));  // 1... d5
    ChessMove move = game.parseSan("exd5");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartY() == 4);  // e-file
    REQUIRE(move.getEndX() == 4);
    REQUIRE(move.getEndY() == 3);    // d-file
}

TEST_CASE("ChessGame: parseSan kingside castling O-O", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, true, true);
    cb.activate();
    ChessMove move = cb.game.parseSan("O-O");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartX() == 0);
    REQUIRE(move.getStartY() == 4);
    REQUIRE(move.getEndX() == 0);
    REQUIRE(move.getEndY() == 6);
}

TEST_CASE("ChessGame: parseSan queenside castling O-O-O", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, false, true);
    cb.activate();
    ChessMove move = cb.game.parseSan("O-O-O");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartX() == 0);
    REQUIRE(move.getStartY() == 4);
    REQUIRE(move.getEndX() == 0);
    REQUIRE(move.getEndY() == 2);
}

TEST_CASE("ChessGame: parseSan promotion e8=Q", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(6, 4, new Pawn(WHITE, false, cb.b, 1));  // white pawn on e7
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    ChessMove move = cb.game.parseSan("e8=Q");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getEndX() == 7);
    REQUIRE(move.getEndY() == 4);
    REQUIRE(move.getPromotion() == QUEEN);
}

TEST_CASE("ChessGame: parseSan promotion with capture dxe8=N", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(6, 3, new Pawn(WHITE, false, cb.b, 1));  // white pawn on d7
    cb.place(7, 4, new Knight(BLACK, false, cb.b));    // black knight on e8
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.activate();
    ChessMove move = cb.game.parseSan("dxe8=N");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartY() == 3);  // d-file
    REQUIRE(move.getEndX() == 7);
    REQUIRE(move.getEndY() == 4);    // e-file
    REQUIRE(move.getPromotion() == KNIGHT);
}

TEST_CASE("ChessGame: parseSan knight disambiguation by file Nbd2", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(2, 5, new Knight(WHITE, false, cb.b));  // Knight on f3
    cb.place(0, 1, new Knight(WHITE, false, cb.b));  // Knight on b1
    cb.activate();
    // Both knights can reach d2 (row 1, col 3). Nbd2 specifies b-file knight.
    ChessMove move = cb.game.parseSan("Nbd2");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartY() == 1);  // b-file
    REQUIRE(move.getEndX() == 1);
    REQUIRE(move.getEndY() == 3);
}

TEST_CASE("ChessGame: parseSan knight disambiguation by rank N1d2", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(2, 5, new Knight(WHITE, false, cb.b));  // Knight on f3 (rank 3)
    cb.place(0, 1, new Knight(WHITE, false, cb.b));  // Knight on b1 (rank 1)
    cb.activate();
    ChessMove move = cb.game.parseSan("N1d2");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getStartX() == 0);  // rank 1
    REQUIRE(move.getEndX() == 1);
    REQUIRE(move.getEndY() == 3);
}

TEST_CASE("ChessGame: parseSan invalid move returns end sentinel", "[ChessGame][SAN]") {
    ChessGame game;
    ChessMove move = game.parseSan("Qd4");  // Queen can't move to d4 from initial position
    REQUIRE(move.isEnd());
}

TEST_CASE("ChessGame: parseSan with check suffix Bb8+", "[ChessGame][SAN]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(3, 5, new Bishop(WHITE, false, cb.b));  // Bishop on f4
    cb.activate();
    ChessMove move = cb.game.parseSan("Bb8+");
    REQUIRE(!move.isEnd());
    REQUIRE(move.getEndX() == 7);
    REQUIRE(move.getEndY() == 1);
}

TEST_CASE("ChessGame: parseSan rejects false capture annotation", "[ChessGame][SAN]") {
    ChessGame game;
    // Nxf3 is invalid — f3 is empty, so 'x' is a false capture claim
    ChessMove move = game.parseSan("Nxf3");
    REQUIRE(move.isEnd());
    // But Nf3 (without x) works
    move = game.parseSan("Nf3");
    REQUIRE(!move.isEnd());
}

// ============================================================================
// Draw Detection
// ============================================================================

TEST_CASE("ChessGame: no draw claims at start of game", "[ChessGame][Draw]") {
    ChessGame game;
    REQUIRE_FALSE(game.canClaimDraw());
    REQUIRE_FALSE(game.isAutomaticDraw());
}

TEST_CASE("ChessGame: threefold repetition is claimable", "[ChessGame][Draw]") {
    // Move knights back and forth to repeat the starting position.
    // Position 1: initial. Nf3, Nf6 → position 2. Ng1, Ng8 → position 1 again (count=2).
    // Nf3, Nf6 → position 2 again (count=2). Ng1, Ng8 → position 1 (count=3). Claimable!
    ChessGame game;
    // Cycle 1
    REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // Nf3
    REQUIRE(game.makeMove(ChessMove(7, 6, 5, 5)));  // Nf6
    REQUIRE(game.makeMove(ChessMove(2, 5, 0, 6)));  // Ng1
    REQUIRE(game.makeMove(ChessMove(5, 5, 7, 6)));  // Ng8
    REQUIRE_FALSE(game.canClaimDraw());  // position repeated twice, not yet three
    // Cycle 2
    REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // Nf3
    REQUIRE(game.makeMove(ChessMove(7, 6, 5, 5)));  // Nf6
    REQUIRE(game.makeMove(ChessMove(2, 5, 0, 6)));  // Ng1
    REQUIRE(game.makeMove(ChessMove(5, 5, 7, 6)));  // Ng8
    // Position has now occurred 3 times — claimable
    REQUIRE(game.canClaimDraw());
    REQUIRE_FALSE(game.isAutomaticDraw());  // not 5-fold yet
}

TEST_CASE("ChessGame: fivefold repetition is automatic draw", "[ChessGame][Draw]") {
    ChessGame game;
    // Need position to occur 5 times. Initial = occurrence 1. Each full cycle adds 1.
    for (int i = 0; i < 4; i++) {
        REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // Nf3
        REQUIRE(game.makeMove(ChessMove(7, 6, 5, 5)));  // Nf6
        REQUIRE(game.makeMove(ChessMove(2, 5, 0, 6)));  // Ng1
        REQUIRE(game.makeMove(ChessMove(5, 5, 7, 6)));  // Ng8
    }
    REQUIRE(game.isAutomaticDraw());
}

TEST_CASE("ChessGame: 50-move rule claimable at 100 halfmoves", "[ChessGame][Draw]") {
    CustomBoard cb;
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    // Rooks bounce on separate rows (12-move cycles). Fivefold repetition also
    // triggers within 50 iterations, but the test verifies canClaimDraw is true
    // once the halfmove clock reaches 100. Repetition is tested separately above.
    cb.place(2, 0, new Rook(WHITE, false, cb.b));
    cb.place(5, 7, new Rook(BLACK, false, cb.b));
    cb.activate();
    int wCol = 0, wDir = 1;
    int bCol = 7, bDir = -1;
    for (int i = 0; i < 49; i++) {
        int wNext = wCol + wDir;
        if (wNext > 6 || wNext < 0) { wDir = -wDir; wNext = wCol + wDir; }
        REQUIRE(cb.game.makeMove(ChessMove(2, wCol, 2, wNext)));
        wCol = wNext;
        int bNext = bCol + bDir;
        if (bNext < 1 || bNext > 7) { bDir = -bDir; bNext = bCol + bDir; }
        REQUIRE(cb.game.makeMove(ChessMove(5, bCol, 5, bNext)));
        bCol = bNext;
    }
    // Two more moves to reach 100 halfmoves.
    {
        int wNext = wCol + wDir;
        if (wNext > 6 || wNext < 0) { wDir = -wDir; wNext = wCol + wDir; }
        REQUIRE(cb.game.makeMove(ChessMove(2, wCol, 2, wNext)));
        wCol = wNext;
        int bNext = bCol + bDir;
        if (bNext < 1 || bNext > 7) { bDir = -bDir; bNext = bCol + bDir; }
        REQUIRE(cb.game.makeMove(ChessMove(5, bCol, 5, bNext)));
    }
    // 100 halfmoves — claimable (50-move rule; fivefold repetition also applies)
    REQUIRE(cb.game.canClaimDraw());
}

TEST_CASE("ChessGame: 75-move rule is automatic draw", "[ChessGame][Draw]") {
    CustomBoard cb;
    cb.place(0, 0, new King(WHITE, cb.b));
    cb.place(7, 7, new King(BLACK, cb.b));
    cb.place(2, 0, new Rook(WHITE, false, cb.b));
    cb.place(5, 7, new Rook(BLACK, false, cb.b));
    cb.activate();
    int wCol = 0, wDir = 1;
    int bCol = 7, bDir = -1;
    for (int i = 0; i < 75; i++) {
        int wNext = wCol + wDir;
        if (wNext > 6 || wNext < 0) { wDir = -wDir; wNext = wCol + wDir; }
        REQUIRE(cb.game.makeMove(ChessMove(2, wCol, 2, wNext)));
        wCol = wNext;
        int bNext = bCol + bDir;
        if (bNext < 1 || bNext > 7) { bDir = -bDir; bNext = bCol + bDir; }
        REQUIRE(cb.game.makeMove(ChessMove(5, bCol, 5, bNext)));
        bCol = bNext;
    }
    // 150 halfmoves — automatic draw (75-move rule; fivefold repetition also applies)
    REQUIRE(cb.game.isAutomaticDraw());
}

TEST_CASE("ChessGame: pawn move resets 50-move counter for draw", "[ChessGame][Draw]") {
    ChessGame game;
    // Make some knight moves (non-pawn, non-capture)
    REQUIRE(game.makeMove(ChessMove(0, 6, 2, 5)));  // Nf3
    REQUIRE(game.makeMove(ChessMove(7, 6, 5, 5)));  // Nf6
    REQUIRE_FALSE(game.canClaimDraw());
    // Now make a pawn move — this resets the clock
    REQUIRE(game.makeMove(ChessMove(1, 4, 3, 4)));  // e4
    // After pawn move, draw definitely not claimable
    REQUIRE_FALSE(game.canClaimDraw());
}

// ============================================================================
// Position identity for repetition detection
// ============================================================================

TEST_CASE("ChessGame: position identity ignores ep square when no legal capture exists",
          "[ChessGame][Draw]") {
    // The standard initial position + 1.e4 creates an ep target on e3, but no
    // black pawn is on d4 or f4 to capture it. So for position identity, the ep
    // field should be ignored.
    //
    // Play: 1. e4 Nc6  2. Nf3 Nb8  3. Ng1 Nc6  4. Ng1 Nb8  5. ...
    // After 2. Nf3 Nb8: board = initial except e-pawn on e4, black to move
    // After 4. Ng1 Nb8: same board, black to move = second occurrence
    //
    // But we need a third occurrence. Use a different approach:
    // Start from a FEN where a double push just happened (ep target set) but
    // no enemy pawn can capture. Then cycle to repeat.

    // Use fromFen to set up: pawn already on a4, black pawn on h7,
    // black to move, ep=a3 (irrelevant since no black pawn near a-file).
    auto game = ChessGame::fromFen("4k3/7p/8/8/P7/8/8/4K3 b - a3 0 1");
    REQUIRE(game != nullptr);

    // Position 1 (from FEN): kings e1/e8, white pawn a4, black pawn h7, black to move
    // Note: ep=a3 in FEN but since no legal capture, position key should use "-"

    // 1... Kd8  2. Kd1  3... Ke8  4. Ke1 → position 2 (same board, black to move)
    REQUIRE(game->makeMove(ChessMove(7, 4, 7, 3)));  // 1... Kd8
    REQUIRE(game->makeMove(ChessMove(0, 4, 0, 3)));  // 2. Kd1
    REQUIRE(game->makeMove(ChessMove(7, 3, 7, 4)));  // 2... Ke8
    REQUIRE(game->makeMove(ChessMove(0, 3, 0, 4)));  // 3. Ke1
    // Position 2: same as 1 (black to move), no ep target
    REQUIRE(game->makeMove(ChessMove(7, 4, 7, 3)));  // 3... Kd8
    REQUIRE(game->makeMove(ChessMove(0, 4, 0, 3)));  // 4. Kd1
    REQUIRE(game->makeMove(ChessMove(7, 3, 7, 4)));  // 4... Ke8
    REQUIRE(game->makeMove(ChessMove(0, 3, 0, 4)));  // 5. Ke1
    // Position 3: same as 1 and 2

    // With correct ep handling: all three positions identical → threefold repetition
    REQUIRE(game->canClaimDraw());
}

// ============================================================================
// JSON Board State (toJson)
// ============================================================================

TEST_CASE("toJson: initial position contains correct FEN", "[ChessGame][JSON]") {
    ChessGame game;
    std::string json = game.toJson();
    REQUIRE(json.find("\"fen\":\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"") !=
            std::string::npos);
}

TEST_CASE("toJson: initial position has turn white", "[ChessGame][JSON]") {
    ChessGame game;
    std::string json = game.toJson();
    REQUIRE(json.find("\"turn\":\"white\"") != std::string::npos);
}

TEST_CASE("toJson: initial position has 20 legal moves", "[ChessGame][JSON]") {
    ChessGame game;
    std::string json = game.toJson();
    // Count occurrences of move strings in legalMoves array
    auto start = json.find("\"legalMoves\":[");
    REQUIRE(start != std::string::npos);
    auto end = json.find(']', start + 14);
    REQUIRE(end != std::string::npos);
    std::string movesStr = json.substr(start + 14, end - start - 14);
    // Count commas + 1 = number of elements (if non-empty)
    int count = movesStr.empty() ? 0 : 1;
    for (char c : movesStr) {
        if (c == ',') count++;
    }
    REQUIRE(count == 20);
}

TEST_CASE("toJson: after 1.e4 has turn black and en passant in FEN", "[ChessGame][JSON]") {
    ChessGame game;
    game.makeMove(ChessMove(1, 4, 3, 4));  // e2-e4
    std::string json = game.toJson();
    REQUIRE(json.find("\"turn\":\"black\"") != std::string::npos);
    REQUIRE(json.find("e3") != std::string::npos);  // en passant square in FEN
}

TEST_CASE("toJson: board array has correct pieces at initial position", "[ChessGame][JSON]") {
    ChessGame game;
    std::string json = game.toJson();
    // Black rook should appear
    REQUIRE(json.find("{\"type\":\"rook\",\"color\":\"black\"}") != std::string::npos);
    // White pawn should appear
    REQUIRE(json.find("{\"type\":\"pawn\",\"color\":\"white\"}") != std::string::npos);
    // White king should appear
    REQUIRE(json.find("{\"type\":\"king\",\"color\":\"white\"}") != std::string::npos);
    // Null squares (empty) should appear
    REQUIRE(json.find("null") != std::string::npos);
}

TEST_CASE("toJson: checkmate position has isCheckmate true", "[ChessGame][JSON]") {
    // Scholar's mate: 1. e4 e5 2. Bc4 Nc6 3. Qh5 Nf6 4. Qxf7#
    ChessGame game;
    game.makeMove(ChessMove(1, 4, 3, 4));  // e2-e4
    game.makeMove(ChessMove(6, 4, 4, 4));  // e7-e5
    game.makeMove(ChessMove(0, 5, 3, 2));  // Bf1-c4
    game.makeMove(ChessMove(7, 1, 5, 2));  // Nb8-c6
    game.makeMove(ChessMove(0, 3, 4, 7));  // Qd1-h5
    game.makeMove(ChessMove(7, 6, 5, 5));  // Ng8-f6
    game.makeMove(ChessMove(4, 7, 6, 5));  // Qh5xf7#
    std::string json = game.toJson();
    REQUIRE(json.find("\"isCheckmate\":true") != std::string::npos);
    REQUIRE(json.find("\"inCheck\":true") != std::string::npos);
}

TEST_CASE("toJson: moveHistory grows after moves", "[ChessGame][JSON]") {
    ChessGame game;
    std::string json = game.toJson();
    REQUIRE(json.find("\"moveHistory\":[]") != std::string::npos);

    game.makeMove(ChessMove(1, 4, 3, 4));  // e2-e4
    json = game.toJson();
    REQUIRE(json.find("\"moveHistory\":[\"e2-e4\"]") != std::string::npos);

    game.makeMove(ChessMove(6, 4, 4, 4));  // e7-e5
    json = game.toJson();
    REQUIRE(json.find("\"moveHistory\":[\"e2-e4\",\"e7-e5\"]") != std::string::npos);
}

// ============================================================================
// FEN Deserialization
// ============================================================================

TEST_CASE("ChessGame::fromFen: round-trip initial position", "[ChessGame][FEN]") {
    ChessGame game;
    std::string originalFen = game.toFen();
    auto loaded = ChessGame::fromFen(originalFen);
    REQUIRE(loaded->toFen() == originalFen);
}

TEST_CASE("ChessGame::fromFen: mid-game position pieces placed correctly", "[ChessGame][FEN]") {
    // Sicilian Defense after 1. e4 c5
    std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    auto game = ChessGame::fromFen(fen);
    // White pawn on e4 (x=3, y=4)
    const ChessPiece* wp = game->getPiece(3, 4);
    REQUIRE(wp != nullptr);
    REQUIRE(wp->getType() == PAWN);
    REQUIRE(wp->getWhite() == true);
    // Black pawn on c5 (x=4, y=2)
    const ChessPiece* bp = game->getPiece(4, 2);
    REQUIRE(bp != nullptr);
    REQUIRE(bp->getType() == PAWN);
    REQUIRE(bp->getWhite() == false);
    // Empty square at e2 (x=1, y=4) — pawn moved away
    REQUIRE(game->getPiece(1, 4) == nullptr);
    // FEN round-trips
    REQUIRE(game->toFen() == fen);
}

TEST_CASE("ChessGame::fromFen: preserves active color (black to move)", "[ChessGame][FEN]") {
    std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->getTurn() == BLACK);
    REQUIRE(game->toFen() == fen);
}

TEST_CASE("ChessGame::fromFen: preserves partial castling rights", "[ChessGame][FEN]") {
    // Only white queenside and black kingside castling available
    std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Qk - 0 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->toFen() == fen);
    ChessBoard& b = game->getPieceBoard();
    REQUIRE(!b.getCastlingRight(true, true));   // no K
    REQUIRE(b.getCastlingRight(true, false));    // Q present
    REQUIRE(b.getCastlingRight(false, true));    // k present
    REQUIRE(!b.getCastlingRight(false, false));  // no q
}

TEST_CASE("ChessGame::fromFen: preserves en passant target square", "[ChessGame][FEN]") {
    // After 1. e4, en passant target is e3
    std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->toFen() == fen);
    // The white pawn at e4 (x=3, y=4) should have enPassant flag set
    const ChessPiece* p = game->getPiece(3, 4);
    REQUIRE(p != nullptr);
    REQUIRE(p->getType() == PAWN);
    const Pawn* pawn = dynamic_cast<const Pawn*>(p);
    REQUIRE(pawn->getEnPassant());
}

TEST_CASE("ChessGame::fromFen: preserves halfmove clock", "[ChessGame][FEN]") {
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->toFen() == fen);
}

TEST_CASE("ChessGame::fromFen: preserves fullmove number via toFen", "[ChessGame][FEN]") {
    std::string fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->toFen() == fen);
}

TEST_CASE("ChessGame::fromFen: no castling rights (dash)", "[ChessGame][FEN]") {
    std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w - - 0 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game->toFen() == fen);
    // Castling flags should all be cleared
    ChessBoard& b = game->getPieceBoard();
    REQUIRE(!b.getCastlingRight(true, true));
    REQUIRE(!b.getCastlingRight(true, false));
    REQUIRE(!b.getCastlingRight(false, true));
    REQUIRE(!b.getCastlingRight(false, false));
}

TEST_CASE("ChessGame::fromFen: legal moves from loaded position work", "[ChessGame][FEN]") {
    // Scholar's mate setup: white to play Qxf7#
    std::string fen = "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 3";
    auto game = ChessGame::fromFen(fen);
    // White queen on f3 (x=2, y=5) captures f7 (x=6, y=5) — should be checkmate
    REQUIRE(game->makeMove(ChessMove(2, 5, 6, 5)));
    REQUIRE(game->checkmate(BLACK));
}

// ============================================================================
// Independent castling rights
// ============================================================================

TEST_CASE("ChessBoard: getCastlingRight returns initial rights", "[ChessBoard][Castling]") {
    ChessGame game;
    ChessBoard& b = game.getPieceBoard();
    REQUIRE(b.getCastlingRight(true, true));    // white kingside
    REQUIRE(b.getCastlingRight(true, false));   // white queenside
    REQUIRE(b.getCastlingRight(false, true));   // black kingside
    REQUIRE(b.getCastlingRight(false, false));  // black queenside
}

TEST_CASE("ChessGame: castling right lost when rook captured on starting square",
          "[ChessGame][Castling]") {
    // White rook on a1, Black rook on a8, kings on e-file.
    // White plays Rxa8 — captures the black queenside rook on its starting square.
    // Black should lose queenside castling right.
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));  // a1
    cb.place(7, 0, new Rook(BLACK, false, cb.b));   // a8
    cb.place(7, 7, new Rook(BLACK, true, cb.b));     // h8
    cb.b.setCastlingRight(false, true, true);   // black kingside
    cb.b.setCastlingRight(false, false, true);  // black queenside
    cb.activate();

    // Verify black has both castling rights initially
    REQUIRE(cb.b.getCastlingRight(false, true));   // black kingside
    REQUIRE(cb.b.getCastlingRight(false, false));  // black queenside

    // White rook captures black rook on a8 (row 7, col 0)
    REQUIRE(cb.game.makeMove(ChessMove(0, 0, 7, 0)));

    // Black queenside castling right should be gone
    REQUIRE(!cb.b.getCastlingRight(false, false));
    // Black kingside should still be available
    REQUIRE(cb.b.getCastlingRight(false, true));
}

TEST_CASE("ChessGame: FEN reflects castling right lost on rook capture",
          "[ChessGame][Castling][FEN]") {
    // Position from SPEC.md test vector 9.4.2:
    // r3k2r/8/8/8/8/8/8/R3K3 w Qkq - 0 1
    // White plays Rxa8+ — captures Black's queenside rook.
    // Expected FEN after: R3k2r/8/8/8/8/8/8/4K3 b k - 0 1
    auto game = ChessGame::fromFen("r3k2r/8/8/8/8/8/8/R3K3 w Qkq - 0 1");
    REQUIRE(game != nullptr);
    REQUIRE(game->makeMove(ChessMove(0, 0, 7, 0)));  // Rxa8
    REQUIRE(game->toFen() == "R3k2r/8/8/8/8/8/8/4K3 b k - 0 1");
}

TEST_CASE("ChessGame: king move clears both castling rights via flags",
          "[ChessGame][Castling]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, true, true);
    cb.b.setCastlingRight(true, false, true);
    cb.activate();

    REQUIRE(cb.b.getCastlingRight(true, true));
    REQUIRE(cb.b.getCastlingRight(true, false));

    // Move white king
    REQUIRE(cb.game.makeMove(ChessMove(0, 4, 0, 5)));

    REQUIRE(!cb.b.getCastlingRight(true, true));
    REQUIRE(!cb.b.getCastlingRight(true, false));
}

TEST_CASE("ChessGame: rook move from starting square clears that side's right",
          "[ChessGame][Castling]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 0, new Rook(WHITE, false, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.b.setCastlingRight(true, true, true);
    cb.b.setCastlingRight(true, false, true);
    cb.activate();

    // Move kingside rook
    REQUIRE(cb.game.makeMove(ChessMove(0, 7, 1, 7)));

    REQUIRE(!cb.b.getCastlingRight(true, true));   // kingside gone
    REQUIRE(cb.b.getCastlingRight(true, false));    // queenside still there
}

TEST_CASE("ChessGame::fromFen: castling rights set from FEN flags, not piece state",
          "[ChessGame][Castling][FEN]") {
    // FEN says only white queenside (Q) — rooks may or may not be in starting positions.
    auto game = ChessGame::fromFen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Q - 0 1");
    REQUIRE(game != nullptr);
    ChessBoard& b = game->getPieceBoard();
    REQUIRE(!b.getCastlingRight(true, true));   // white kingside: no
    REQUIRE(b.getCastlingRight(true, false));    // white queenside: yes
    REQUIRE(!b.getCastlingRight(false, true));   // black kingside: no
    REQUIRE(!b.getCastlingRight(false, false));  // black queenside: no
    // Round-trip FEN
    REQUIRE(game->toFen() == "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Q - 0 1");
}

TEST_CASE("ChessGame::fromFen: no castling rights preserved as flags",
          "[ChessGame][Castling][FEN]") {
    std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w - - 0 1";
    auto game = ChessGame::fromFen(fen);
    REQUIRE(game != nullptr);
    ChessBoard& b = game->getPieceBoard();
    REQUIRE(!b.getCastlingRight(true, true));
    REQUIRE(!b.getCastlingRight(true, false));
    REQUIRE(!b.getCastlingRight(false, true));
    REQUIRE(!b.getCastlingRight(false, false));
    REQUIRE(game->toFen() == fen);
}
