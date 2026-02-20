// Tests for the chess engine (chess.cpp / chess.h).
// Run via: make test
// Coverage: make coverage

#include <catch2/catch_test_macros.hpp>
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
    ChessBoard* b;

    CustomBoard() : b(game.getPieceBoard()) {
        game.setRules(false);
        for (int x = 0; x < 8; x++)
            for (int y = 0; y < 8; y++) game.setPiece(x, y, NULL);
    }

    void place(int x, int y, ChessPiece* piece) { game.setPiece(x, y, piece); }

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
    cb.activate();

    REQUIRE(!cb.game.getPiece(0, 4)->canMove(0, 6));
}

TEST_CASE("King: cannot castle if rook has moved", "[King]") {
    CustomBoard cb;
    cb.place(0, 4, new King(WHITE, cb.b));
    cb.place(0, 7, new Rook(WHITE, true, cb.b));
    cb.place(7, 4, new King(BLACK, cb.b));
    cb.place(7, 0, new Rook(BLACK, false, cb.b));
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

TEST_CASE("ChessGame: getPieceBoard returns valid board pointer", "[ChessGame]") {
    ChessGame game;
    REQUIRE(game.getPieceBoard() != NULL);
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

TEST_CASE("ChessMove: getPromotion returns PAWN for normal move", "[ChessMove]") {
    ChessMove cm(1, 2, 3, 4);
    REQUIRE(cm.getPromotion() == PAWN);
}

TEST_CASE("ChessMove: getPromotion returns promotion type for promotion move", "[ChessMove]") {
    ChessMove cm(6, 3, 7, 3, QUEEN);
    REQUIRE(cm.getPromotion() == QUEEN);
    REQUIRE(!cm.isEnd());
}
