// This is the chess.cpp file which contains implementations for the headers in chess.h

#include "chess.h"

#include <cassert>
#include <cstdlib>
#include <sstream>

using std::abs;

///////////
// CHESSMOVE
const ChessMove ChessMove::end = ChessMove();
const char ChessMove::fileLetters[8 + 1] = "abcdefgh";
const int ChessMove::maxLength = 1024;  // 669 should be sufficient for all moves of both sides, FYI

// Default ctor — end sentinel (sx = -1)
ChessMove::ChessMove() : sx(-1), sy(-1), ex(-1), ey(-1), promotion(PAWN) {
    repr[0] = 'E';
    repr[1] = 'N';
    repr[2] = 'D';
    repr[3] = '\0';
    repr[4] = repr[5] = repr[6] = repr[7] = '\0';
}

// 4-arg ctor (no promotion)
ChessMove::ChessMove(int sX, int sY, int eX, int eY)
    : sx(-1), sy(-1), ex(-1), ey(-1), promotion(PAWN) {
    assert(sX >= 0 && sX <= 7 && sY >= 0 && sY <= 7 && eX >= 0 && eX <= 7 && eY >= 0 &&
           eY <= 7);
    sx = (int8_t)sX;
    sy = (int8_t)sY;
    ex = (int8_t)eX;
    ey = (int8_t)eY;
    repr[0] = fileLetters[sy];
    repr[1] = ChessPiece::digits[sx + 1];
    repr[2] = '-';
    repr[3] = fileLetters[ey];
    repr[4] = ChessPiece::digits[ex + 1];
    repr[5] = '\0';
    repr[6] = repr[7] = '\0';
}

// 5-arg ctor (with promotion) — delegates to 4-arg
ChessMove::ChessMove(int sX, int sY, int eX, int eY, PieceType promo)
    : ChessMove(sX, sY, eX, eY) {
    if (!isEnd()) {
        promotion = promo;
        if (promo != PAWN) {
            // Indexed by PieceType: {PAWN=p, ROOK=r, KNIGHT=n, BISHOP=b, KING=k, QUEEN=q}.
            // Must stay in sync with the PieceType enum order in chess.h.
            static_assert(PAWN == 0 && ROOK == 1 && KNIGHT == 2 && BISHOP == 3 && QUEEN == 5,
                          "letters[] indexing assumes specific PieceType enum values");
            const char letters[] = {'p', 'r', 'n', 'b', 'k', 'q'};
            repr[5] = letters[promo];
            repr[6] = '\0';
        }
    }
}

// String ctor — delegates to 4-arg; accepts "a1-b2" or "a1b2" format.
// Argument mapping: sX=rank(str[1]-'1'), sY=file(str[0]-'a'),
//   separator at str[2] (' ' or '-') shifts end-square indices by 1.
ChessMove::ChessMove(const char* const str)
    : ChessMove((int)(str[1] - '1'),
                (int)(str[0] - 'a'),
                (str[2] == ' ' || str[2] == '-') ? (int)(str[4] - '1') : (int)(str[3] - '1'),
                (str[2] == ' ' || str[2] == '-') ? (int)(str[3] - 'a') : (int)(str[2] - 'a')) {}

// Copy ctor
ChessMove::ChessMove(const ChessMove& rcm)
    : sx(rcm.sx), sy(rcm.sy), ex(rcm.ex), ey(rcm.ey), promotion(rcm.promotion) {
    for (int i = 0; i < 8; i++) repr[i] = rcm.repr[i];
}

ChessMove::~ChessMove() {}

ChessMove& ChessMove::operator=(const ChessMove& rhs) {
    if (this == &rhs) return *this;
    sx = rhs.sx;
    sy = rhs.sy;
    ex = rhs.ex;
    ey = rhs.ey;
    promotion = rhs.promotion;
    for (int i = 0; i < 8; i++) repr[i] = rhs.repr[i];
    return *this;
}

int ChessMove::getStartX() const { return sx; }
int ChessMove::getStartY() const { return sy; }
int ChessMove::getEndX() const { return ex; }
int ChessMove::getEndY() const { return ey; }
PieceType ChessMove::getPromotion() const { return promotion; }

bool ChessMove::isEnd() const { return sx < 0; }

void ChessMove::swap(ChessMove& cm1, ChessMove& cm2) {
    ChessMove temp = cm1;
    cm1 = cm2;
    cm2 = temp;
}


const char* ChessMove::toString() const { return repr; }

////////////
// CHESSPIECE
const char ChessPiece::digits[11] = "0123456789";

ChessPiece::ChessPiece(bool isW, bool isKS, ChessBoard& b, int i)
    : isWhite(isW), isKingSide(isKS), board(b), index(i) {
    for (int j = 0; j < 4 + 1; j++) {
        id[j] = '\0';
    }

    if (isW)
        id[0] = 'W';
    else
        id[0] = 'B';

    if (isKS)
        id[2] = 'K';
    else
        id[2] = 'Q';

    if (i <= 10 && i >= 0) id[3] = digits[i];
}

ChessPiece::~ChessPiece() {}

int ChessPiece::getIndex() const { return index; }
bool ChessPiece::getWhite() const { return isWhite; }
bool ChessPiece::getKingSide() const { return isKingSide; }

int ChessPiece::getPosX() const { return posX; }  // O(1); cached by ChessBoard
int ChessPiece::getPosY() const { return posY; }

const char* ChessPiece::getID() const { return id; }

bool ChessPiece::move(int x, int y) {
    if (canMove(x, y)) {
        // movePiece returns the displaced piece (if any) as a unique_ptr;
        // it is automatically deleted when the return value is dropped.
        (void)movePiece(board, ChessMove(getPosX(), getPosY(), x, y));
        return true;
    } else
        return false;
}

std::unique_ptr<ChessPiece> ChessPiece::movePiece(ChessBoard& cb, ChessMove move) const {
    return cb.movePiece(move);
}

ChessPiece* ChessPiece::getMutablePiece(ChessBoard& cb, int x, int y) const {
    return cb.getMoveablePiece(x, y);
}

void ChessPiece::setPiece(ChessBoard& cb, int x, int y, std::unique_ptr<ChessPiece> piece) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;
    cb.place(x, y, std::move(piece));
}

int ChessPiece::getRootValue() {
    switch (getType()) {
        case PAWN:
            return 1;
        case ROOK:
            return 5;
        case KNIGHT:
        case BISHOP:
            return 3;
        case QUEEN:
            return 9;
        case KING:
        default:
            return 0;
    }
}

double ChessPiece::getValue() {
    double value = getRootValue();

    return value;
}

//////
// PAWN

Pawn::Pawn(bool isW, bool isKS, ChessBoard& b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'P';
    hasMoved = false;
    enPassant = false;
}

Pawn::~Pawn() {}

bool Pawn::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7)  // check bounds
        return false;

    int cx = getPosX();
    int cy = getPosY();

    if (y == cy && board.getPiece(x, y) == nullptr)  // forward move
    {
        // Double advance: white unmoved pawn can jump from row 1 to row 3
        // (skipping row 2); black from row 6 to row 4 (skipping row 5).
        if (hasMoved == false && isWhite == true && x == 3 && board.getPiece(2, y) == nullptr)
            ;  // Would be return true; need to check for check
        else if (hasMoved == false && isWhite == false && x == 4 &&
                 board.getPiece(5, y) == nullptr)
            ;
        else if (isWhite == true && x == cx + 1)
            ;
        else if (isWhite == false && x == cx - 1)
            ;
        else
            return false;
    } else if ((y == cy + 1 || y == cy - 1) && board.getPiece(x, y) == nullptr) {  // en passant
        // En passant: white pawn must be on row 4 (opponent's double-advance
        // landing row); black pawn on row 3. The captured pawn sits on the
        // same row as the capturing pawn, one column over.
        if (isWhite == true && cx == 4 && board.getPiece(cx, y) != nullptr &&
            board.getPiece(cx, y)->getWhite() == false &&
            board.getPiece(cx, y)->getType() == PAWN) {
            const Pawn* piece = dynamic_cast<const Pawn*>(board.getPiece(cx, y));
            if (piece && piece->getEnPassant())
                ;
            else
                return false;
        } else if (isWhite == false && cx == 3 && board.getPiece(cx, y) != nullptr &&
                   board.getPiece(cx, y)->getWhite() == true &&
                   board.getPiece(cx, y)->getType() == PAWN) {
            const Pawn* piece = dynamic_cast<const Pawn*>(board.getPiece(cx, y));
            if (piece && piece->getEnPassant())
                ;
            else
                return false;
        } else
            return false;
    } else if ((y == cy + 1 || y == cy - 1) &&
               board.getPiece(x, y) != nullptr) {  // diagonal capture
        if (isWhite == true && x == cx + 1 && board.getPiece(x, y)->getWhite() == false)
            ;
        else if (isWhite == false && x == cx - 1 && board.getPiece(x, y)->getWhite() == true)
            ;
        else
            return false;
    } else
        return false;

    if (chkchk) {
        // Temporarily execute the move to test for self-check, then undo it.
        // movePiece returns whatever piece was displaced at the destination
        // (the capture candidate). After checking, we move our piece back and
        // restore the displaced piece via setPiece.
        //
        // For en passant, we must also temporarily remove the captured pawn
        // from (cx, y) — it sits on the same rank as the capturing pawn, one
        // column over.  Without this, a horizontal pin through both pawns is
        // invisible to checkCheck because the captured pawn still blocks the
        // sliding attack.  We handle this by first moving the captured pawn to
        // the destination square (x, y), so the subsequent movePiece of the
        // capturing pawn displaces it and returns it as 'temp'.
        bool isEnPassant = (y != cy) && (board.getPiece(x, y) == nullptr);
        if (isEnPassant) {
            // Move captured pawn from (cx, y) to destination (x, y).
            // This clears it from the rank for accurate check detection.
            (void)movePiece(board, ChessMove(cx, y, x, y));
        }
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(board.checkCheck(isWhite));
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        if (isEnPassant) {
            // Restore captured pawn to its original square (cx, y).
            // temp holds the captured pawn (displaced by the capturing pawn).
            setPiece(board, cx, y, std::move(temp));
        } else {
            // temp->posX/posY is stale while temp is off the board, but
            // checkCheck only reads pieces via the grid so this is safe.
            // place() (called by setPiece) restores the cache here.
            setPiece(board, x, y, std::move(temp));
        }
        return ret;
    } else
        return true;
}

std::vector<ChessMove> Pawn::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    int sign = isWhite ? 1 : -1;
    std::vector<ChessMove> ret;

    if (x + sign == 0 || x + sign == 7) {
        // Pawn reaches back rank — emit one move per promotion type per direction
        for (PieceType p : {QUEEN, ROOK, KNIGHT, BISHOP}) {
            if (canMove(x + sign, y)) ret.emplace_back(x, y, x + sign, y, p);
            if (canMove(x + sign, y + 1)) ret.emplace_back(x, y, x + sign, y + 1, p);
            if (canMove(x + sign, y - 1)) ret.emplace_back(x, y, x + sign, y - 1, p);
        }
    } else {
        if (canMove(x + sign, y)) ret.emplace_back(x, y, x + sign, y);
        if (canMove(x + 2 * sign, y)) ret.emplace_back(x, y, x + 2 * sign, y);
        if (canMove(x + sign, y + 1)) ret.emplace_back(x, y, x + sign, y + 1);
        if (canMove(x + sign, y - 1)) ret.emplace_back(x, y, x + sign, y - 1);
    }
    return ret;
}

bool Pawn::getMoved() const { return hasMoved; }
bool Pawn::getEnPassant() const { return enPassant; }
void Pawn::setEnPassant(bool b) { enPassant = b; }

bool Pawn::move(int x, int y) {
    int ox = getPosX();
    int oy = getPosY();

    bool destEmpty = board.getPiece(x, y) == nullptr;

    bool b = ChessPiece::move(x, y);
    if (b) {
        hasMoved = true;

        if (isWhite == true && x == ox + 1 && abs(y - oy) == 1 && destEmpty) {
            // En passant: remove the captured pawn. unique_ptr in setPiece handles deletion.
            setPiece(board, ox, y, nullptr);
        } else if (isWhite == false && x == ox - 1 && abs(y - oy) == 1 && destEmpty) {
            setPiece(board, ox, y, nullptr);
        }

        if (abs(ox - x) == 2) enPassant = true;
    }
    return b;

    // promotion to be covered elsewhere
}

PieceType Pawn::getType() const { return PAWN; }

//////
// ROOK

Rook::Rook(bool isW, bool isKS, ChessBoard& b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'R';
    hasMoved = false;
}

Rook::~Rook() {}

bool Rook::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    int cx = getPosX();
    int cy = getPosY();

    if (board.getPiece(x, y) != nullptr &&
        board.getPiece(x, y)->getWhite() == isWhite)  // capturing same-side piece.
        return false;

    if (x == cx) {
        if (y == cy)  // null moves don't count
            return false;
        else  // check space in between
        {
            if (y < cy) {
                for (int i = y + 1; i < cy; i++) {
                    if (board.getPiece(x, i) != nullptr) return false;
                };
            } else {  // y > cy
                for (int i = y - 1; i > cy; i--) {
                    if (board.getPiece(x, i) != nullptr) return false;
                };
            }
        }
    } else if (y != cy) {
        return false;
    } else {  // check space in between again
        if (x < cx) {
            for (int i = x + 1; i < cx; i++) {
                if (board.getPiece(i, y) != nullptr) return false;
            };
        } else {  // x > cx
            for (int i = x - 1; i > cx; i--) {
                if (board.getPiece(i, y) != nullptr) return false;
            };
        }
    }

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(board.checkCheck(isWhite));
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        // temp->posX/posY is stale while temp is off the board, but
        // checkCheck only reads pieces via the grid so this is safe.
        // place() (called by setPiece) restores the cache here.
        setPiece(board, x, y, std::move(temp));
        return ret;
    } else
        return true;
}

std::vector<ChessMove> Rook::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    std::vector<ChessMove> ret;

    for (int i = 0; i < 8; i++)  // moves in the same row
        if (y != i && canMove(x, i)) ret.emplace_back(x, y, x, i);

    for (int i = 0; i < 8; i++)  // moves in the same column
        if (x != i && canMove(i, y)) ret.emplace_back(x, y, i, y);

    return ret;
}

bool Rook::getMoved() const { return hasMoved; }

bool Rook::move(int x, int y) {
    bool b = ChessPiece::move(x, y);
    if (b) markMoved();
    return b;
}

PieceType Rook::getType() const { return ROOK; }

void Rook::markMoved() { hasMoved = true; }

////////
// KNIGHT

const int Knight::xOffsets[8] = {1, 1, -1, -1, 2, 2, -2, -2};
const int Knight::yOffsets[8] = {2, -2, 2, -2, 1, -1, 1, -1};

Knight::Knight(bool isW, bool isKS, ChessBoard& b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'N';
}

Knight::~Knight() {}

bool Knight::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board.getPiece(x, y) != nullptr && board.getPiece(x, y)->getWhite() == isWhite)
        return false;

    int cx = getPosX();
    int cy = getPosY();

    if (abs(cx - x) == 1) {
        if (abs(cy - y) == 2)
            ;
        else
            return false;
    } else if (abs(cx - x) == 2) {
        if (abs(cy - y) == 1)
            ;
        else
            return false;
    } else
        return false;

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(board.checkCheck(isWhite));
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        // temp->posX/posY is stale while temp is off the board, but
        // checkCheck only reads pieces via the grid so this is safe.
        // place() (called by setPiece) restores the cache here.
        setPiece(board, x, y, std::move(temp));
        return ret;
    } else
        return true;
}

std::vector<ChessMove> Knight::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    std::vector<ChessMove> ret;
    for (int i = 0; i < 8; i++)
        if (canMove(x + xOffsets[i], y + yOffsets[i]))
            ret.emplace_back(x, y, x + xOffsets[i], y + yOffsets[i]);

    return ret;
}

PieceType Knight::getType() const { return KNIGHT; }

////////
// BISHOP

Bishop::Bishop(bool isW, bool isKS, ChessBoard& b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'B';
}

Bishop::~Bishop() {}

bool Bishop::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    int cx = getPosX();
    int cy = getPosY();

    int diagDiff = cy - cx;
    int diagSum = cy + cx;

    if (board.getPiece(x, y) != nullptr &&
        board.getPiece(x, y)->getWhite() == isWhite)  // can't capture same side
        return false;

    if (y - x == diagDiff) {
        if (x == cx)
            return false;
        else if (x > cx) {
            for (int i = 1; (cy + i < y && cx + i < x); i++) {
                if (board.getPiece(cx + i, cy + i) != nullptr) return false;
            };
        } else {  // x < cx
            for (int i = 1; (cy - i > y && cx - i > x); i++) {
                if (board.getPiece(cx - i, cy - i) != nullptr) return false;
            };
        }
    } else if (y + x == diagSum) {
        if (x == cx)
            return false;
        else if (x > cx) {
            for (int i = 1; (cy - i > y && cx + i < x); i++) {
                if (board.getPiece(cx + i, cy - i) != nullptr) return false;
            };
        } else {  // x < cx
            for (int i = 1; (cy + i < y && cx - i > x); i++) {
                if (board.getPiece(cx - i, cy + i) != nullptr) return false;
            };
        }
    } else
        return false;

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(board.checkCheck(isWhite));
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        // temp->posX/posY is stale while temp is off the board, but
        // checkCheck only reads pieces via the grid so this is safe.
        // place() (called by setPiece) restores the cache here.
        setPiece(board, x, y, std::move(temp));
        return ret;
    } else
        return true;
}

std::vector<ChessMove> Bishop::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    std::vector<ChessMove> ret;

    for (int j = 0; (x + j < 8 && y + j < 8); j++)
        if (canMove(x + j, y + j)) ret.emplace_back(x, y, x + j, y + j);

    for (int j = 0; (x - j >= 0 && y - j >= 0); j++)
        if (canMove(x - j, y - j)) ret.emplace_back(x, y, x - j, y - j);

    for (int j = 0; (x + j < 8 && y - j >= 0); j++)
        if (canMove(x + j, y - j)) ret.emplace_back(x, y, x + j, y - j);

    for (int j = 0; (x - j >= 0 && y + j < 8); j++)
        if (canMove(x - j, y + j)) ret.emplace_back(x, y, x - j, y + j);

    return ret;
}

PieceType Bishop::getType() const { return BISHOP; }

//////
// KING

const int King::xOffsets[10] = {-1, -1, -1, 0, 0, 1, 1, 1, 0, 0};
const int King::yOffsets[10] = {-1, 0, 1, -1, 1, -1, 0, 1, -2, 2};

King::King(bool isW, ChessBoard& b) : ChessPiece(isW, true, b, 0) {
    id[1] = 'K';
    hasMoved = false;
}

King::~King() {}

bool King::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board.getPiece(x, y) != nullptr &&
        board.getPiece(x, y)->getWhite() == isWhite)  // also disallows null move
        return false;

    int cx = getPosX();
    int cy = getPosY();

    if (abs(x - cx) <= 1 && abs(y - cy) <= 1)
        ;
    else {
        if (abs(y - cy) == 2 && x == cx && !hasMoved)  // castling
        {
            if (chkchk && inCheck())  // can't castle out of check
                return false;

            // sign: +1 = kingside (y increases), -1 = queenside (y decreases).
            // King always starts at column 4 (hard-coded assumption).
            // Rook column: (7 + 7*sign)/2 -> kingside=7, queenside=0.
            int sign = (y - cy) / 2;
            bool isKingSide = (sign == 1);

            // Check independent castling right flag.
            if (!board.getCastlingRight(isWhite, isKingSide)) return false;

            if (board.getPiece(cx, 4 + sign) == nullptr &&
                board.getPiece(cx, 4 + 2 * sign) == nullptr &&
                board.getPiece(cx, (7 + 7 * sign) / 2) != nullptr)  // check rooks
            {
                if (sign == -1 &&
                    board.getPiece(cx, 1) != nullptr)  // Queen side: b-file must also be empty
                    return false;

                if (board.getPiece(cx, (7 + 7 * sign) / 2)->getType() == ROOK &&
                    board.getPiece(cx, (7 + 7 * sign) / 2)->getWhite() == isWhite) {
                    const Rook* piece =
                        dynamic_cast<const Rook*>(board.getPiece(cx, (7 + 7 * sign) / 2));
                    if (!piece->getMoved()) {
                        // Step the king through each intermediate square and verify
                        // it is not in check at any point (castling through check is illegal).
                        (void)movePiece(board, ChessMove(cx, 4, cx, 4 + sign));
                        if (chkchk && inCheck()) {
                            (void)movePiece(board, ChessMove(cx, 4 + sign, cx, 4));
                            return false;
                        }

                        (void)movePiece(board, ChessMove(cx, 4 + sign, cx, 4 + sign * 2));
                        if (chkchk && inCheck()) {
                            (void)movePiece(board, ChessMove(cx, 4 + sign * 2, cx, 4));
                            return false;
                        }

                        (void)movePiece(board, ChessMove(cx, 4 + sign * 2, cx, 4));
                    } else
                        return false;
                } else
                    return false;
            } else
                return false;
        } else
            return false;
    }

    if (chkchk) {
        // Call inCheck() directly rather than checkCheck() to avoid an extra
        // board scan. checkCheck() would search the board for this king and then
        // call inCheck() — but since we are already executing as the king, we
        // can call inCheck() directly. Both are equivalent; inCheck() is faster.
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(inCheck());
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        // temp->posX/posY is stale while temp is off the board, but
        // checkCheck only reads pieces via the grid so this is safe.
        // place() (called by setPiece) restores the cache here.
        setPiece(board, x, y, std::move(temp));
        return ret;
    } else
        return true;
}

std::vector<ChessMove> King::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    std::vector<ChessMove> ret;
    for (int i = 0; i < 10; i++)
        if (canMove(x + xOffsets[i], y + yOffsets[i]))
            ret.emplace_back(x, y, x + xOffsets[i], y + yOffsets[i]);

    return ret;
}

bool King::move(int x, int y) {
    int ox = getPosX();
    int oy = getPosY();

    bool b = ChessPiece::move(x, y);
    if (b) {
        hasMoved = true;
        if (y == 6 && oy == 4) {
            (void)movePiece(board, ChessMove(ox, 7, ox, 5));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(board, ox, 5));
            assert(piece);
            piece->markMoved();
        }
        if (y == 2 && oy == 4) {
            (void)movePiece(board, ChessMove(ox, 0, ox, 3));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(board, ox, 3));
            assert(piece);
            piece->markMoved();
        }
    }
    return b;
}

bool King::inCheck() const {
    int cx = getPosX();
    int cy = getPosY();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // canMove(..., chkchk=false): passing false breaks the recursion cycle.
            // The full chain is: canMove(true) -> checkCheck -> inCheck -> canMove(false).
            // At this final step we only need to know if the enemy piece geometrically
            // reaches the king's square; asking whether *that* move would expose the
            // enemy's own king (chkchk=true) would restart the cycle and recurse forever.
            if (board.getPiece(i, j) != nullptr && board.getPiece(i, j)->getWhite() != isWhite &&
                board.getPiece(i, j)->canMove(cx, cy, false))
                return true;
        }
    }
    return false;
}

bool King::getMoved() const { return hasMoved; }
void King::markMoved() { hasMoved = true; }

PieceType King::getType() const { return KING; }

///////
// QUEEN

Queen::Queen(bool isW, ChessBoard& b, int i, bool isKS) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'Q';
}

Queen::~Queen() {}

bool Queen::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board.getPiece(x, y) != nullptr &&
        board.getPiece(x, y)->getWhite() == isWhite)  // also disallows null move
        return false;

    int cx = getPosX();
    int cy = getPosY();

    int diagDiff = cy - cx;
    int diagSum = cy + cx;

    if (x == cx) {
        if (y == cy)  // null moves don't count
            return false;
        else  // check space in between
        {
            if (y < cy) {
                for (int i = y + 1; i < cy; i++) {
                    if (board.getPiece(x, i) != nullptr) return false;
                };
            } else {  // y > cy
                for (int i = y - 1; i > cy; i--) {
                    if (board.getPiece(x, i) != nullptr) return false;
                };
            }
        }
    } else if (y != cy) {
        if (y - x == diagDiff) {
            if (x == cx)
                return false;
            else if (x > cx) {
                for (int i = 1; (cy + i < y && cx + i < x); i++) {
                    if (board.getPiece(cx + i, cy + i) != nullptr) return false;
                };
            } else {  // x < cx
                for (int i = 1; (cy - i > y && cx - i > x); i++) {
                    if (board.getPiece(cx - i, cy - i) != nullptr) return false;
                };
            }
        } else if (y + x == diagSum) {
            if (x == cx)
                return false;
            else if (x > cx) {
                for (int i = 1; (cy - i > y && cx + i < x); i++) {
                    if (board.getPiece(cx + i, cy - i) != nullptr) return false;
                };
            } else {  // x < cx
                for (int i = 1; (cy + i < y && cx - i > x); i++) {
                    if (board.getPiece(cx - i, cy + i) != nullptr) return false;
                };
            }
        } else
            return false;
    } else {  // check space in between again
        if (x < cx) {
            for (int i = x + 1; i < cx; i++) {
                if (board.getPiece(i, y) != nullptr) return false;
            };
        } else {  // x > cx
            for (int i = x - 1; i > cx; i--) {
                if (board.getPiece(i, y) != nullptr) return false;
            };
        }
    }

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(board, ChessMove(cx, cy, x, y));
        bool ret = !(board.checkCheck(isWhite));
        (void)movePiece(board, ChessMove(x, y, cx, cy));
        // temp->posX/posY is stale while temp is off the board, but
        // checkCheck only reads pieces via the grid so this is safe.
        // place() (called by setPiece) restores the cache here.
        setPiece(board, x, y, std::move(temp));
        return ret;
    } else
        return true;
}

std::vector<ChessMove> Queen::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    std::vector<ChessMove> ret;

    for (int i = 0; i < 8; i++)  // same row
        if (y != i && canMove(x, i)) ret.emplace_back(x, y, x, i);

    for (int i = 0; i < 8; i++)  // same column
        if (x != i && canMove(i, y)) ret.emplace_back(x, y, i, y);

    for (int j = 0; (x + j < 8 && y + j < 8); j++)
        if (canMove(x + j, y + j)) ret.emplace_back(x, y, x + j, y + j);

    for (int j = 0; (x - j >= 0 && y - j >= 0); j++)
        if (canMove(x - j, y - j)) ret.emplace_back(x, y, x - j, y - j);

    for (int j = 0; (x + j < 8 && y - j >= 0); j++)
        if (canMove(x + j, y - j)) ret.emplace_back(x, y, x + j, y - j);

    for (int j = 0; (x - j >= 0 && y + j < 8); j++)
        if (canMove(x - j, y + j)) ret.emplace_back(x, y, x - j, y + j);

    return ret;
}

PieceType Queen::getType() const { return QUEEN; }

////////////
// CHESSBOARD

ChessBoard::ChessBoard() {
    // Set up the pieces using place() which initialises posX/posY on each piece.
    place(0, 0, std::make_unique<Rook>(WHITE, false, *this, 0));
    place(0, 1, std::make_unique<Knight>(WHITE, false, *this, 0));
    place(0, 2, std::make_unique<Bishop>(WHITE, false, *this, 0));
    place(0, 3, std::make_unique<Queen>(WHITE, *this, 0));
    place(0, 4, std::make_unique<King>(WHITE, *this));
    place(0, 5, std::make_unique<Bishop>(WHITE, true, *this, 0));
    place(0, 6, std::make_unique<Knight>(WHITE, true, *this, 0));
    place(0, 7, std::make_unique<Rook>(WHITE, true, *this, 0));

    place(1, 0, std::make_unique<Pawn>(WHITE, false, *this, 4));
    place(1, 1, std::make_unique<Pawn>(WHITE, false, *this, 3));
    place(1, 2, std::make_unique<Pawn>(WHITE, false, *this, 2));
    place(1, 3, std::make_unique<Pawn>(WHITE, false, *this, 1));
    place(1, 4, std::make_unique<Pawn>(WHITE, true, *this, 1));
    place(1, 5, std::make_unique<Pawn>(WHITE, true, *this, 2));
    place(1, 6, std::make_unique<Pawn>(WHITE, true, *this, 3));
    place(1, 7, std::make_unique<Pawn>(WHITE, true, *this, 4));

    place(6, 0, std::make_unique<Pawn>(BLACK, false, *this, 4));
    place(6, 1, std::make_unique<Pawn>(BLACK, false, *this, 3));
    place(6, 2, std::make_unique<Pawn>(BLACK, false, *this, 2));
    place(6, 3, std::make_unique<Pawn>(BLACK, false, *this, 1));
    place(6, 4, std::make_unique<Pawn>(BLACK, true, *this, 1));
    place(6, 5, std::make_unique<Pawn>(BLACK, true, *this, 2));
    place(6, 6, std::make_unique<Pawn>(BLACK, true, *this, 3));
    place(6, 7, std::make_unique<Pawn>(BLACK, true, *this, 4));

    place(7, 0, std::make_unique<Rook>(BLACK, false, *this, 0));
    place(7, 1, std::make_unique<Knight>(BLACK, false, *this, 0));
    place(7, 2, std::make_unique<Bishop>(BLACK, false, *this, 0));
    place(7, 3, std::make_unique<Queen>(BLACK, *this, 0));
    place(7, 4, std::make_unique<King>(BLACK, *this));
    place(7, 5, std::make_unique<Bishop>(BLACK, true, *this, 0));
    place(7, 6, std::make_unique<Knight>(BLACK, true, *this, 0));
    place(7, 7, std::make_unique<Rook>(BLACK, true, *this, 0));
}

ChessBoard::~ChessBoard() {}  // unique_ptrs in grid[] clean up automatically

bool ChessBoard::getCastlingRight(bool isWhite, bool isKingSide) const {
    return castlingRights[isWhite ? 0 : 1][isKingSide ? 0 : 1];
}

void ChessBoard::clearCastlingRight(bool isWhite, bool isKingSide) {
    castlingRights[isWhite ? 0 : 1][isKingSide ? 0 : 1] = false;
}

void ChessBoard::setCastlingRight(bool isWhite, bool isKingSide, bool value) {
    castlingRights[isWhite ? 0 : 1][isKingSide ? 0 : 1] = value;
}

const char* ChessBoard::toString() {
    // Layout: 8 ranks x 4 display rows/rank + 1 border row = 33 rows.
    // Each row: 8 squares x 5 chars/square + 1 border col = 41 chars + newline = 42.
    // Total: 42 * 33 + 1 (null terminator) = 1387 bytes.
    repr.assign(1387, '\0');  // 1387 = 42 * 33 + 1

    bool white = false;

    repr[1386] = '\0';

    for (int i = 0; i < 33; i++)  // 33 rows
    {
        repr[42 * i + 41] = '\n';  // newlines
        if (i % 4 == 0) {
            for (int j = 0; j < 41; j++)  // 41 columns
            {
                if (j % 5 == 0)
                    repr[42 * i + j] = '+';
                else {
                    if (i == 0 || i == 32)
                        repr[42 * i + j] = ChessMove::fileLetters[j / 5];
                    else
                        repr[42 * i + j] = '-';
                }
            }
        } else if (i % 2 == 0) {
            for (int j = 0; j < 41; j++) {
                if (j % 5 == 0) {
                    if (j == 0 || j == 40)
                        repr[42 * i + j] = ChessPiece::digits[8 - (i / 4)];
                    else
                        repr[42 * i + j] = '|';
                } else if (j % 5 == 1 || j % 5 == 4) {
                    if (grid[7 - (i / 4)][j / 5] != nullptr)
                        repr[42 * i + j] = ' ';
                    else {
                        white = (i / 4) % 2 == 0;
                        if ((j / 5) % 2 != 0) white = !white;
                        if (white)
                            repr[42 * i + j] = 'X';
                        else
                            repr[42 * i + j] = ' ';
                    }
                } else {
                    if (grid[7 - (i / 4)][j / 5] != nullptr) {
                        if (j % 5 == 2)
                            repr[42 * i + j] = *((grid[7 - (i / 4)][j / 5])->getID());
                        else
                            repr[42 * i + j] = *((grid[7 - (i / 4)][j / 5])->getID() + 1);
                    } else {
                        white = (i / 4) % 2 == 0;
                        if ((j / 5) % 2 != 0) white = !white;
                        if (white)
                            repr[42 * i + j] = 'X';
                        else
                            repr[42 * i + j] = ' ';
                    }
                }
            }
        } else {
            for (int j = 0; j < 41; j++) {
                if (j % 5 == 0) {
                    if (j == 0 || j == 40)
                        repr[42 * i + j] = ChessPiece::digits[8 - (i / 4)];
                    else
                        repr[42 * i + j] = '|';
                } else if (j % 5 == 1 || j % 5 == 4) {
                    white = (i / 4) % 2 == 0;
                    if ((j / 5) % 2 != 0) white = !white;
                    if (white)
                        repr[42 * i + j] = 'X';
                    else
                        repr[42 * i + j] = ' ';
                } else {
                    if (grid[7 - (i / 4)][j / 5] != nullptr)
                        repr[42 * i + j] = ' ';
                    else {
                        white = (i / 4) % 2 == 0;
                        if ((j / 5) % 2 != 0) white = !white;
                        if (white)
                            repr[42 * i + j] = 'X';
                        else
                            repr[42 * i + j] = ' ';
                    }
                }
            }
        }
    }
    return repr.c_str();
}

ChessPiece* ChessBoard::getMoveablePiece(int x, int y) {
    if (x >= 0 && x <= 7 && y >= 0 && y <= 7)
        return grid[x][y].get();
    else
        return nullptr;
}

void ChessBoard::place(int x, int y, std::unique_ptr<ChessPiece> p) {
    assert(x >= 0 && x <= 7 && y >= 0 && y <= 7);
    if (p) {
        p->posX = x;
        p->posY = y;
    }
    grid[x][y] = std::move(p);
}

std::unique_ptr<ChessPiece> ChessBoard::movePiece(ChessMove move) {
    if (!move.isEnd() && grid[move.getStartX()][move.getStartY()] != nullptr) {
        auto displaced = std::move(grid[move.getEndX()][move.getEndY()]);
        grid[move.getEndX()][move.getEndY()] =
            std::move(grid[move.getStartX()][move.getStartY()]);
        grid[move.getStartX()][move.getStartY()] = nullptr;
        // Update position cache for the piece that just moved.
        // The piece is always non-null here: the outer guard confirmed
        // the source was non-null, and it is what we just moved to dest.
        assert(grid[move.getEndX()][move.getEndY()] != nullptr);
        grid[move.getEndX()][move.getEndY()]->posX = move.getEndX();
        grid[move.getEndX()][move.getEndY()]->posY = move.getEndY();
        return displaced;
    } else
        return nullptr;
}

const ChessPiece* ChessBoard::getPiece(int x, int y) const {
    if (x >= 0 && x <= 7 && y >= 0 && y <= 7)
        return grid[x][y].get();
    else
        return nullptr;
}

bool ChessBoard::checkCheck(bool isW) const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (grid[i][j] != nullptr && (grid[i][j])->getType() == KING &&
                (grid[i][j])->getWhite() == isW) {
                const King* piece = dynamic_cast<const King*>(getPiece(i, j));
                if (piece && piece->inCheck())
                    return true;
                else if (piece)
                    return false;
                else
                    return true;  // dynamic_cast failed: king pointer is wrong type; treat as error
                                  // (in check)
            }
        }
    }
    // No king of the requested color found on the board. This should not happen
    // during normal play (a king is always present), but can occur when the board
    // is configured manually via ChessGame::setPiece() with rules disabled — for
    // example, during pawn promotion or in test fixtures that clear the board
    // without placing a king. Returning true (in check) is the safest sentinel:
    // it causes getMoves() to return an empty list, preventing any moves.
    return true;
}

///////////
// CHESSGAME

ChessGame::ChessGame() : rulesOn(true), whiteTurn(true), board(ChessBoard()) {
    positionHistory.push_back(positionKey());
}

void ChessGame::setRules(bool on) { rulesOn = on; }
bool ChessGame::getRules() const { return rulesOn; }

ChessGame::~ChessGame() {}

bool ChessGame::checkmate(bool white) const {
    if (!board.checkCheck(white)) return false;
    return getMoves(white).empty();
}

bool ChessGame::stalemate(bool turn) const {
    if (board.checkCheck(turn)) return false;
    return getMoves(turn).empty();
}

std::vector<ChessMove> ChessGame::getMoves(bool white) const {
    std::vector<ChessMove> all;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            const ChessPiece* p = board.getPiece(i, j);
            if (p && p->getWhite() == white) {
                auto m = p->getMoves();
                all.insert(all.end(), m.begin(), m.end());
            }
        }
    }
    return all;
}

std::unique_ptr<ChessPiece> ChessGame::makePiece(PieceType type, bool white, int y) {
    bool ks = (y > 3);
    int idx = white ? whiteProms : blackProms;
    switch (type) {
        case QUEEN:
            return std::make_unique<Queen>(white, board, idx, ks);
        case ROOK:
            return std::make_unique<Rook>(white, ks, board, idx);
        case KNIGHT:
            return std::make_unique<Knight>(white, ks, board, idx);
        case BISHOP:
            return std::make_unique<Bishop>(white, ks, board, idx);
        default:
            fprintf(stderr, "makePiece: invalid promotion type %d\n", static_cast<int>(type));
            std::abort();
    }
}

bool ChessGame::getTurn() const { return whiteTurn; }

const std::vector<ChessMove>& ChessGame::getHistory() const { return history; }

bool ChessGame::makeMove(const ChessMove& cm) {
    if (rulesOn) {
        ChessPiece* piece = board.getMoveablePiece(cm.getStartX(), cm.getStartY());
        if (piece == nullptr || piece->getWhite() != whiteTurn) return false;
        bool isPawnMove = (piece->getType() == PAWN);
        bool isCapture = (board.getPiece(cm.getEndX(), cm.getEndY()) != nullptr);
        // En passant is also a capture (destination is empty but a pawn is taken).
        if (isPawnMove && !isCapture && cm.getStartY() != cm.getEndY()) isCapture = true;
        bool b = piece->move(cm.getEndX(), cm.getEndY());
        if (b) {
            history.push_back(cm);
            if (isPawnMove || isCapture)
                halfmoveClock = 0;
            else
                halfmoveClock++;
            bool movedColor = whiteTurn;  // capture before flip

            // Update independent castling rights.
            // King move: clear both rights for that color.
            if (piece->getType() == KING) {
                board.clearCastlingRight(movedColor, true);
                board.clearCastlingRight(movedColor, false);
            }
            // Rook move from starting square: clear that side's right.
            if (piece->getType() == ROOK) {
                int sx = cm.getStartX(), sy = cm.getStartY();
                int homeRank = movedColor ? 0 : 7;
                if (sx == homeRank && sy == 7) board.clearCastlingRight(movedColor, true);
                if (sx == homeRank && sy == 0) board.clearCastlingRight(movedColor, false);
            }
            // Capture on a rook's starting square: clear that side's right.
            int ex = cm.getEndX(), ey = cm.getEndY();
            if (ex == 0 && ey == 7) board.clearCastlingRight(true, true);
            if (ex == 0 && ey == 0) board.clearCastlingRight(true, false);
            if (ex == 7 && ey == 7) board.clearCastlingRight(false, true);
            if (ex == 7 && ey == 0) board.clearCastlingRight(false, false);
            // Handle pawn promotion: replace pawn with requested piece type.
            // Write directly to board.grid (ChessGame is a friend of ChessBoard)
            // rather than routing through setPiece() to avoid the rulesOn guard.
            if (cm.getPromotion() != PAWN) {
                int endX = cm.getEndX(), endY = cm.getEndY();
                board.place(endX, endY, makePiece(cm.getPromotion(), movedColor, endY));
                if (movedColor) whiteProms++;
                else blackProms++;
            }
            whiteTurn = !whiteTurn;
            // Clear en passant flags for the side whose turn it now is.
            // After the flip, whiteTurn is the color that did NOT just move —
            // their en passant window (set when they double-advanced last turn)
            // has now expired because the opponent completed a move.
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (board.getPiece(i, j) != nullptr &&
                        board.getPiece(i, j)->getWhite() == whiteTurn) {
                        ChessPiece* p = board.getMoveablePiece(i, j);
                        if (p->getType() == PAWN) {
                            Pawn* pawn = dynamic_cast<Pawn*>(p);
                            pawn->setEnPassant(false);
                        }
                    }
                }
            }
            positionHistory.push_back(positionKey());
        }
        return b;
    } else {
        (void)board.movePiece(cm);  // displaced piece auto-deleted by unique_ptr
        return true;
    }
}

const char* ChessGame::getBoard() { return board.toString(); }

const ChessPiece* ChessGame::getPiece(int x, int y) const { return board.getPiece(x, y); }

void ChessGame::setPiece(int x, int y, std::unique_ptr<ChessPiece> piece) {
    if (rulesOn) return;
    if (x < 0 || x > 7 || y < 0 || y > 7) return;
    board.place(x, y, std::move(piece));
}

std::string ChessGame::toFen() const {
    std::string fen;

    // 1. Piece placement (rank 8 down to rank 1 = x=7 down to x=0)
    for (int x = 7; x >= 0; x--) {
        int empty = 0;
        for (int y = 0; y < 8; y++) {
            const ChessPiece* p = board.getPiece(x, y);
            if (p == nullptr) {
                empty++;
            } else {
                if (empty > 0) {
                    fen += std::to_string(empty);
                    empty = 0;
                }
                char c;
                switch (p->getType()) {
                    case PAWN: c = 'p'; break;
                    case ROOK: c = 'r'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case QUEEN: c = 'q'; break;
                    case KING: c = 'k'; break;
                }
                if (p->getWhite()) c = c - 32;  // uppercase for white
                fen += c;
            }
        }
        if (empty > 0) fen += std::to_string(empty);
        if (x > 0) fen += '/';
    }

    // 2. Active color
    fen += whiteTurn ? " w" : " b";

    // 3. Castling availability — uses independent flags on ChessBoard.
    std::string castling;
    if (board.getCastlingRight(true, true)) castling += 'K';
    if (board.getCastlingRight(true, false)) castling += 'Q';
    if (board.getCastlingRight(false, true)) castling += 'k';
    if (board.getCastlingRight(false, false)) castling += 'q';
    fen += ' ';
    fen += castling.empty() ? "-" : castling;

    // 4. En passant target square
    std::string ep = "-";
    for (int y = 0; y < 8; y++) {
        // En passant target is on the square "behind" the pawn that just double-advanced.
        // Check pawns that have the enPassant flag set.
        // White pawn with enPassant is at row 3 (just moved from row 1 to 3); target = row 2.
        // Black pawn with enPassant is at row 4 (just moved from row 6 to 4); target = row 5.
        for (int x = 0; x < 8; x++) {
            const ChessPiece* p = board.getPiece(x, y);
            if (p != nullptr && p->getType() == PAWN) {
                const Pawn* pawn = dynamic_cast<const Pawn*>(p);
                if (pawn->getEnPassant()) {
                    int targetX = pawn->getWhite() ? (x - 1) : (x + 1);
                    ep = "";
                    ep += ChessMove::fileLetters[y];
                    ep += std::to_string(targetX + 1);
                }
            }
        }
    }
    fen += ' ';
    fen += ep;

    // 5. Halfmove clock
    fen += ' ';
    fen += std::to_string(halfmoveClock);

    // 6. Fullmove number
    fen += ' ';
    fen += std::to_string(1 + (int)history.size() / 2);

    return fen;
}

ChessMove ChessGame::parseSan(const std::string& san) const {
    if (san.empty()) return ChessMove();

    // Strip and record check/checkmate suffixes (+, #).
    std::string s = san;
    bool hasCheck = false, hasCheckmate = false;
    while (!s.empty() && (s.back() == '+' || s.back() == '#')) {
        if (s.back() == '#') hasCheckmate = true;
        else hasCheck = true;
        s.pop_back();
    }
    if (s.empty()) return ChessMove();

    // Helper: validate check/checkmate suffix against the actual move result.
    auto validateSuffix = [&](const ChessMove& m) -> ChessMove {
        if (!hasCheck && !hasCheckmate) return m;
        auto copy = ChessGame::fromFen(toFen());
        if (!copy) return ChessMove();
        if (!copy->makeMove(ChessMove(m.getStartX(), m.getStartY(),
                                       m.getEndX(), m.getEndY(), m.getPromotion())))
            return ChessMove();
        bool opponent = !whiteTurn;  // the side being checked is the one not moving
        bool givesCheck = copy->board.checkCheck(opponent);
        bool givesCheckmate = copy->checkmate(opponent);
        if (hasCheckmate && !givesCheckmate) return ChessMove();
        if (hasCheck && !givesCheck) return ChessMove();
        return m;
    };

    // Castling.
    if (s == "O-O") {
        int rank = whiteTurn ? 0 : 7;
        ChessMove cm(rank, 4, rank, 6);
        auto moves = getMoves(whiteTurn);
        for (const auto& m : moves) {
            if (m.getStartX() == cm.getStartX() && m.getStartY() == cm.getStartY() &&
                m.getEndX() == cm.getEndX() && m.getEndY() == cm.getEndY())
                return validateSuffix(m);
        }
        return ChessMove();
    }
    if (s == "O-O-O") {
        int rank = whiteTurn ? 0 : 7;
        ChessMove cm(rank, 4, rank, 2);
        auto moves = getMoves(whiteTurn);
        for (const auto& m : moves) {
            if (m.getStartX() == cm.getStartX() && m.getStartY() == cm.getStartY() &&
                m.getEndX() == cm.getEndX() && m.getEndY() == cm.getEndY())
                return validateSuffix(m);
        }
        return ChessMove();
    }

    // Parse promotion suffix (e.g., "=Q", "=N").
    PieceType promo = PAWN;
    {
        size_t eq = s.find('=');
        if (eq != std::string::npos && eq + 1 < s.size()) {
            char pc = s[eq + 1];
            switch (pc) {
                case 'Q': promo = QUEEN; break;
                case 'R': promo = ROOK; break;
                case 'B': promo = BISHOP; break;
                case 'N': promo = KNIGHT; break;
                default: return ChessMove();
            }
            s = s.substr(0, eq);
        }
    }

    // Determine piece type and strip the piece letter.
    PieceType pieceType = PAWN;
    if (!s.empty() && s[0] >= 'A' && s[0] <= 'Z') {
        switch (s[0]) {
            case 'K': pieceType = KING; break;
            case 'Q': pieceType = QUEEN; break;
            case 'R': pieceType = ROOK; break;
            case 'B': pieceType = BISHOP; break;
            case 'N': pieceType = KNIGHT; break;
            default: return ChessMove();
        }
        s = s.substr(1);
    }

    // Remove and record 'x' capture indicator.
    bool hasCapture = false;
    std::string cleaned;
    for (char c : s) {
        if (c == 'x') hasCapture = true;
        else cleaned += c;
    }
    s = cleaned;

    // Now s should be one of:
    //   "e4"       — destination only (pawn or piece)
    //   "fe4"      — file disambiguation + destination (pawn capture or piece)
    //   "1e4"      — rank disambiguation + destination
    //   "f1e4"     — full disambiguation (file+rank) + destination

    if (s.size() < 2) return ChessMove();

    // Destination is always the last two characters.
    int endY = s[s.size() - 2] - 'a';
    int endX = s[s.size() - 1] - '1';
    if (endX < 0 || endX > 7 || endY < 0 || endY > 7) return ChessMove();

    // Disambiguation from remaining characters.
    int disambigFile = -1;  // 0-7 if specified
    int disambigRank = -1;  // 0-7 if specified
    std::string prefix = s.substr(0, s.size() - 2);
    for (char c : prefix) {
        if (c >= 'a' && c <= 'h') disambigFile = c - 'a';
        else if (c >= '1' && c <= '8') disambigRank = c - '1';
        else return ChessMove();
    }

    // Find matching legal move.
    auto moves = getMoves(whiteTurn);
    ChessMove match;
    int matchCount = 0;
    for (const auto& m : moves) {
        if (m.getEndX() != endX || m.getEndY() != endY) continue;
        const ChessPiece* p = board.getPiece(m.getStartX(), m.getStartY());
        if (p == nullptr || p->getType() != pieceType) continue;
        if (disambigFile >= 0 && m.getStartY() != disambigFile) continue;
        if (disambigRank >= 0 && m.getStartX() != disambigRank) continue;
        // For promotion moves, check that the promotion type matches.
        if (promo != PAWN) {
            if (m.getPromotion() != promo) continue;
        } else {
            // If no promotion specified, skip promotion moves.
            if (m.getPromotion() != PAWN) continue;
        }
        match = m;
        matchCount++;
    }

    if (matchCount != 1) return ChessMove();  // ambiguous or no match

    // Validate capture annotation: 'x' must correspond to an actual capture.
    // A capture occurs when the destination is occupied, or for en passant
    // (pawn moves diagonally to an empty square).
    if (hasCapture) {
        bool isCapture = (board.getPiece(endX, endY) != nullptr);
        if (!isCapture && pieceType == PAWN && match.getStartY() != match.getEndY())
            isCapture = true;  // en passant
        if (!isCapture) return ChessMove();
    }

    return validateSuffix(match);
}

std::string ChessGame::toSan(const ChessMove& move) const {
    if (move.isEnd()) return "";

    int sx = move.getStartX(), sy = move.getStartY();
    int ex = move.getEndX(),   ey = move.getEndY();
    const ChessPiece* piece = board.getPiece(sx, sy);
    if (piece == nullptr) return "";

    PieceType type = piece->getType();
    bool isWhite = piece->getWhite();

    std::string san;

    // Castling.
    if (type == KING && std::abs(sy - ey) == 2) {
        san = (ey > sy) ? "O-O" : "O-O-O";
    } else {
        // Piece letter prefix (not for pawns).
        const char pieceLetters[] = {'?', 'R', 'N', 'B', 'K', 'Q'};
        if (type != PAWN) {
            san += pieceLetters[type];

            // Disambiguation: find all same-type, same-color pieces that can
            // geometrically reach the destination (canMove with chkchk=false).
            bool needFile = false, needRank = false;
            bool sameFile = false, sameRank = false;
            int ambigCount = 0;
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    if (x == sx && y == sy) continue;
                    const ChessPiece* other = board.getPiece(x, y);
                    if (other == nullptr) continue;
                    if (other->getType() != type || other->getWhite() != isWhite) continue;
                    if (!other->canMove(ex, ey, false)) continue;
                    ambigCount++;
                    if (y == sy) sameFile = true;
                    if (x == sx) sameRank = true;
                }
            }
            if (ambigCount > 0) {
                if (!sameFile) {
                    needFile = true;
                } else if (!sameRank) {
                    needRank = true;
                } else {
                    needFile = true;
                    needRank = true;
                }
            }
            if (needFile) san += static_cast<char>('a' + sy);
            if (needRank) san += static_cast<char>('1' + sx);
        }

        // Capture indicator.
        bool isCapture = (board.getPiece(ex, ey) != nullptr);
        // En passant: pawn moves diagonally to empty square.
        if (type == PAWN && sy != ey && !isCapture) isCapture = true;
        if (isCapture) {
            if (type == PAWN) san += static_cast<char>('a' + sy);  // pawn source file
            san += 'x';
        }

        // Destination square.
        san += static_cast<char>('a' + ey);
        san += static_cast<char>('1' + ex);

        // Promotion.
        if (move.getPromotion() != PAWN) {
            san += '=';
            san += pieceLetters[move.getPromotion()];
        }
    }

    // Check/checkmate suffix: simulate the move to test.
    // Make a copy via FEN round-trip to avoid mutating this game.
    auto copy = ChessGame::fromFen(toFen());
    if (copy && copy->makeMove(ChessMove(sx, sy, ex, ey, move.getPromotion()))) {
        bool opponentColor = !isWhite;
        if (copy->checkmate(opponentColor)) {
            san += '#';
        } else if (copy->board.checkCheck(opponentColor)) {
            san += '+';
        }
    }

    return san;
}

std::string ChessGame::normalizeSan(const std::string& san) {
    std::string s = san;

    // 0. Strip leading/trailing whitespace first so other steps work on clean input.
    {
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\n\r");
        s = s.substr(start, end - start + 1);
    }

    // 1. Strip NAGs: '$' followed by digits.
    {
        std::string result;
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '$') {
                // Skip $ and following digits.
                i++;
                while (i < s.size() && s[i] >= '0' && s[i] <= '9') i++;
                i--;  // loop increment
            } else {
                result += s[i];
            }
        }
        s = result;
    }

    // 2. Strip move assessment glyphs (longest first): !!, ??, !?, ?!, !, ?
    {
        // Strip from the end, after any check/checkmate suffix.
        // First, save and strip check/checkmate suffixes.
        std::string suffix;
        while (!s.empty() && (s.back() == '+' || s.back() == '#')) {
            suffix = s.back() + suffix;
            s.pop_back();
        }
        // Now strip assessment glyphs from the end.
        bool changed = true;
        while (changed && !s.empty()) {
            changed = false;
            for (const char* glyph : {"!!", "??", "!?", "?!", "!", "?"}) {
                size_t glen = strlen(glyph);
                if (s.size() >= glen && s.substr(s.size() - glen) == glyph) {
                    s.resize(s.size() - glen);
                    changed = true;
                    break;
                }
            }
        }
        s += suffix;
    }

    // 3. Convert digit-zero castling to letter-O.
    if (s == "0-0" || s == "0-0+" || s == "0-0#") {
        s = "O-O" + s.substr(3);
    } else if (s == "0-0-0" || s == "0-0-0+" || s == "0-0-0#") {
        s = "O-O-O" + s.substr(5);
    }

    // 4. Strip leading/trailing whitespace.
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    s = s.substr(start, end - start + 1);

    return s;
}

std::unique_ptr<ChessGame> ChessGame::fromFen(const std::string& fen) {
    // Split into exactly 6 space-separated fields.
    std::vector<std::string> fields;
    {
        std::istringstream ss(fen);
        std::string token;
        while (ss >> token) fields.push_back(token);
    }
    if (fields.size() != 6) return nullptr;

    const std::string& placement = fields[0];
    const std::string& activeColor = fields[1];
    const std::string& castling = fields[2];
    const std::string& enPassant = fields[3];

    // Validate halfmove clock and fullmove number (must be non-negative integers).
    int halfmove, fullmove;
    try {
        size_t pos;
        halfmove = std::stoi(fields[4], &pos);
        if (pos != fields[4].size()) return nullptr;  // trailing chars
        fullmove = std::stoi(fields[5], &pos);
        if (pos != fields[5].size()) return nullptr;
    } catch (...) {
        return nullptr;
    }
    if (halfmove < 0) return nullptr;
    if (fullmove < 1) return nullptr;

    // Validate active color.
    if (activeColor != "w" && activeColor != "b") return nullptr;

    // Validate castling field: must be "-" or a subset of "KQkq" in order.
    if (castling != "-") {
        const std::string allowed = "KQkq";
        size_t prev = 0;
        for (char c : castling) {
            size_t idx = allowed.find(c);
            if (idx == std::string::npos) return nullptr;
            if (idx < prev) return nullptr;  // wrong order
            prev = idx + 1;
        }
    }

    // Validate en passant field.
    if (enPassant != "-") {
        if (enPassant.size() != 2) return nullptr;
        if (enPassant[0] < 'a' || enPassant[0] > 'h') return nullptr;
        if (enPassant[1] < '1' || enPassant[1] > '8') return nullptr;
        // ep target must be on rank 3 or 6
        if (enPassant[1] != '3' && enPassant[1] != '6') return nullptr;
    }

    // Validate and parse piece placement.
    // Split by '/' — must have exactly 8 ranks.
    std::vector<std::string> ranks;
    {
        std::string rank;
        for (char c : placement) {
            if (c == '/') {
                ranks.push_back(rank);
                rank.clear();
            } else {
                rank += c;
            }
        }
        ranks.push_back(rank);
    }
    if (ranks.size() != 8) return nullptr;

    // Validate each rank: no consecutive digits, exactly 8 squares, valid piece chars.
    // Also count kings and check for pawns on back ranks.
    int whiteKings = 0, blackKings = 0;
    int whiteKingX = -1, whiteKingY = -1, blackKingX = -1, blackKingY = -1;

    // Build a piece map: ranks[0] = rank 8 (x=7), ranks[7] = rank 1 (x=0).
    struct PlacedPiece { int x; int y; char letter; bool isWhite; };
    std::vector<PlacedPiece> pieces;

    for (int ri = 0; ri < 8; ri++) {
        int boardX = 7 - ri;  // rank 8 → x=7, rank 1 → x=0
        int squares = 0;
        bool lastWasDigit = false;
        for (char c : ranks[ri]) {
            if (c >= '1' && c <= '8') {
                if (lastWasDigit) return nullptr;  // consecutive digits
                squares += (c - '0');
                lastWasDigit = true;
            } else {
                lastWasDigit = false;
                bool isWhite = (c >= 'A' && c <= 'Z');
                char lower = isWhite ? static_cast<char>(c + 32) : c;
                if (lower != 'p' && lower != 'r' && lower != 'n' &&
                    lower != 'b' && lower != 'q' && lower != 'k')
                    return nullptr;  // invalid piece letter

                // Pawn on back rank check
                if (lower == 'p' && (boardX == 0 || boardX == 7))
                    return nullptr;

                // Count kings
                if (lower == 'k') {
                    if (isWhite) { whiteKings++; whiteKingX = boardX; whiteKingY = squares; }
                    else         { blackKings++; blackKingX = boardX; blackKingY = squares; }
                }

                pieces.push_back({boardX, squares, lower, isWhite});
                squares++;
            }
        }
        if (squares != 8) return nullptr;  // rank doesn't have exactly 8 squares
    }

    // Exactly one king per side.
    if (whiteKings != 1 || blackKings != 1) return nullptr;

    // Kings must not be adjacent.
    if (std::abs(whiteKingX - blackKingX) <= 1 && std::abs(whiteKingY - blackKingY) <= 1)
        return nullptr;

    // --- All validation passed; build the game. ---

    auto game = std::unique_ptr<ChessGame>(new ChessGame());
    game->setRules(false);

    // Clear the board
    for (int rx = 0; rx < 8; rx++)
        for (int ry = 0; ry < 8; ry++)
            game->setPiece(rx, ry, nullptr);

    // Place pieces.
    for (const auto& pp : pieces) {
        bool isKingSide = (pp.y > 3);
        int pawnIdx = isKingSide ? (pp.y - 3) : (4 - pp.y);
        std::unique_ptr<ChessPiece> piece;
        switch (pp.letter) {
            case 'p': piece = std::make_unique<Pawn>(pp.isWhite, isKingSide, game->board, pawnIdx); break;
            case 'r': piece = std::make_unique<Rook>(pp.isWhite, isKingSide, game->board, 0); break;
            case 'n': piece = std::make_unique<Knight>(pp.isWhite, isKingSide, game->board, 0); break;
            case 'b': piece = std::make_unique<Bishop>(pp.isWhite, isKingSide, game->board, 0); break;
            case 'q': piece = std::make_unique<Queen>(pp.isWhite, game->board, 0, isKingSide); break;
            case 'k': piece = std::make_unique<King>(pp.isWhite, game->board); break;
        }
        game->board.place(pp.x, pp.y, std::move(piece));
    }

    // Active color.
    game->whiteTurn = (activeColor == "w");

    // Castling rights — default is all true; clear the ones not present.
    if (castling.find('K') == std::string::npos)
        game->board.clearCastlingRight(true, true);
    if (castling.find('Q') == std::string::npos)
        game->board.clearCastlingRight(true, false);
    if (castling.find('k') == std::string::npos)
        game->board.clearCastlingRight(false, true);
    if (castling.find('q') == std::string::npos)
        game->board.clearCastlingRight(false, false);

    // En passant target square.
    if (enPassant != "-") {
        int epY = enPassant[0] - 'a';
        int epX = enPassant[1] - '1';
        int pawnX = (activeColor == "b") ? (epX + 1) : (epX - 1);
        ChessPiece* p = game->board.getMoveablePiece(pawnX, epY);
        if (p != nullptr && p->getType() == PAWN)
            dynamic_cast<Pawn*>(p)->setEnPassant(true);
    }

    // Halfmove clock.
    game->halfmoveClock = halfmove;

    // Fullmove number — derive history size so toFen() reproduces it.
    int histSize = (fullmove - 1) * 2 + (activeColor == "b" ? 1 : 0);
    game->history.clear();
    for (int i = 0; i < histSize; i++)
        game->history.push_back(ChessMove());

    // Side not to move must not be in check.
    game->setRules(true);
    bool notToMove = !game->whiteTurn;
    if (game->board.checkCheck(notToMove)) return nullptr;

    // Initialize position history with the current position.
    game->positionHistory.clear();
    game->positionHistory.push_back(game->positionKey());

    return game;
}

std::string ChessGame::positionKey() const {
    std::string fen = toFen();
    // Extract first 4 space-separated fields (piece placement, active color,
    // castling, en passant) — halfmove clock and fullmove number are not part
    // of position identity.
    int spaces = 0;
    size_t end = fen.size();
    for (size_t i = 0; i < fen.size(); i++) {
        if (fen[i] == ' ') {
            spaces++;
            if (spaces == 4) { end = i; break; }
        }
    }
    std::string key = fen.substr(0, end);

    // Per SPEC 4.3: the en passant target is only part of position identity
    // when a fully legal en passant capture exists. If no legal capture is
    // available, replace the ep field with "-" so positions match.
    // Find the ep field (after 3rd space).
    size_t epStart = 0;
    int sp = 0;
    for (size_t i = 0; i < key.size(); i++) {
        if (key[i] == ' ') {
            sp++;
            if (sp == 3) { epStart = i + 1; break; }
        }
    }
    std::string ep = key.substr(epStart);
    if (ep != "-") {
        // Check if any legal en passant capture exists.
        bool hasLegalEp = false;
        int epY = ep[0] - 'a';  // file
        int epX = ep[1] - '1';  // rank (0-indexed)
        // The capturing pawn must be on the same rank as the pawn that double-pushed,
        // i.e., one rank "behind" the ep target from the capturing side's perspective.
        // If white to move, ep target is on rank 5 (epX=5), capturing pawn on rank 4.
        // If black to move, ep target is on rank 2 (epX=2), capturing pawn on rank 3.
        int captRank = whiteTurn ? (epX - 1) : (epX + 1);
        for (int dy : {-1, 1}) {
            int adjFile = epY + dy;
            if (adjFile < 0 || adjFile > 7) continue;
            const ChessPiece* p = board.getPiece(captRank, adjFile);
            if (p != nullptr && p->getType() == PAWN && p->getWhite() == whiteTurn) {
                // Check if the en passant capture is legal (doesn't leave king in check).
                if (p->canMove(epX, epY)) {
                    hasLegalEp = true;
                    break;
                }
            }
        }
        if (!hasLegalEp) {
            key = key.substr(0, epStart) + "-";
        }
    }
    return key;
}

// O(n) scan over full history. Could be improved by only scanning back
// halfmoveClock entries (positions can't repeat across pawn moves or captures)
// or by maintaining an unordered_map<string, int> as a running counter.
int ChessGame::positionCount() const {
    if (positionHistory.empty()) return 0;
    const std::string& current = positionHistory.back();
    int count = 0;
    for (const auto& pos : positionHistory) {
        if (pos == current) count++;
    }
    return count;
}

bool ChessGame::canClaimDraw() const {
    return halfmoveClock >= 100 || positionCount() >= 3;
}

bool ChessGame::isAutomaticDraw() const {
    return halfmoveClock >= 150 || positionCount() >= 5;
}

bool ChessGame::insufficientMaterial() const {
    // SPEC 4.4: Exactly these material combinations are insufficient:
    // 1. K vs K
    // 2. K+B vs K
    // 3. K+N vs K
    // 4. K+B vs K+B (same color square bishops)

    int whiteBishops = 0, blackBishops = 0;
    int whiteKnights = 0, blackKnights = 0;
    int whiteOther = 0, blackOther = 0;  // pawns, rooks, queens
    int whiteBishopColor = -1, blackBishopColor = -1;  // 0=dark, 1=light

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            const ChessPiece* p = board.getPiece(x, y);
            if (p == nullptr) continue;
            PieceType t = p->getType();
            if (t == KING) continue;
            bool w = p->getWhite();
            if (t == BISHOP) {
                if (w) { whiteBishops++; whiteBishopColor = (x + y) % 2; }
                else   { blackBishops++; blackBishopColor = (x + y) % 2; }
            } else if (t == KNIGHT) {
                if (w) whiteKnights++; else blackKnights++;
            } else {
                if (w) whiteOther++; else blackOther++;
            }
        }
    }

    // Any pawns, rooks, or queens → sufficient
    if (whiteOther > 0 || blackOther > 0) return false;

    int wMinor = whiteBishops + whiteKnights;
    int bMinor = blackBishops + blackKnights;

    // K vs K
    if (wMinor == 0 && bMinor == 0) return true;
    // K+B vs K or K+N vs K (either side)
    if (wMinor == 1 && bMinor == 0) return true;
    if (wMinor == 0 && bMinor == 1) return true;
    // K+B vs K+B same color
    if (whiteBishops == 1 && blackBishops == 1 &&
        whiteKnights == 0 && blackKnights == 0 &&
        whiteBishopColor == blackBishopColor) return true;

    return false;
}

std::string ChessGame::toJson() const {
    std::string json = "{";

    // fen
    json += "\"fen\":\"" + toFen() + "\"";

    // turn
    json += ",\"turn\":\"";
    json += whiteTurn ? "white" : "black";
    json += "\"";

    // board: 8x8 array, rank 8 (x=7) to rank 1 (x=0)
    json += ",\"board\":[";
    for (int x = 7; x >= 0; x--) {
        json += "[";
        for (int y = 0; y < 8; y++) {
            const ChessPiece* p = board.getPiece(x, y);
            if (p == nullptr) {
                json += "null";
            } else {
                json += "{\"type\":\"";
                switch (p->getType()) {
                    case PAWN: json += "pawn"; break;
                    case ROOK: json += "rook"; break;
                    case KNIGHT: json += "knight"; break;
                    case BISHOP: json += "bishop"; break;
                    case QUEEN: json += "queen"; break;
                    case KING: json += "king"; break;
                }
                json += "\",\"color\":\"";
                json += p->getWhite() ? "white" : "black";
                json += "\"}";
            }
            if (y < 7) json += ",";
        }
        json += "]";
        if (x > 0) json += ",";
    }
    json += "]";

    // legalMoves
    bool currentTurn = whiteTurn;
    std::vector<ChessMove> moves = getMoves(currentTurn);
    json += ",\"legalMoves\":[";
    for (size_t i = 0; i < moves.size(); i++) {
        json += "\"";
        json += moves[i].toString();
        json += "\"";
        if (i + 1 < moves.size()) json += ",";
    }
    json += "]";

    // inCheck
    bool inChk = board.checkCheck(currentTurn);
    json += ",\"inCheck\":";
    json += inChk ? "true" : "false";

    // isCheckmate
    json += ",\"isCheckmate\":";
    json += checkmate(currentTurn) ? "true" : "false";

    // isStalemate
    json += ",\"isStalemate\":";
    json += stalemate(currentTurn) ? "true" : "false";

    // canClaimDraw
    json += ",\"canClaimDraw\":";
    json += canClaimDraw() ? "true" : "false";

    // isAutomaticDraw
    json += ",\"isAutomaticDraw\":";
    json += isAutomaticDraw() ? "true" : "false";

    // halfmoveClock
    json += ",\"halfmoveClock\":" + std::to_string(halfmoveClock);

    // fullmoveNumber
    json += ",\"fullmoveNumber\":" + std::to_string(1 + static_cast<int>(history.size()) / 2);

    // moveHistory
    json += ",\"moveHistory\":[";
    for (size_t i = 0; i < history.size(); i++) {
        json += "\"";
        json += history[i].toString();
        json += "\"";
        if (i + 1 < history.size()) json += ",";
    }
    json += "]";

    json += "}";
    return json;
}

ChessBoard& ChessGame::getPieceBoard() { return board; }
