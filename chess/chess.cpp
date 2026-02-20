// This is the chess.cpp file which contains implementations for the headers in chess.h

#include "chess.h"

#include <cassert>
#include <cstdlib>

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
    if (sX < 0 || sX > 7 || sY < 0 || sY > 7 || eX < 0 || eX > 7 || eY < 0 || eY > 7) {
        repr[0] = 'E';
        repr[1] = 'N';
        repr[2] = 'D';
        repr[3] = '\0';
        repr[4] = repr[5] = repr[6] = repr[7] = '\0';
    } else {
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
}

// 5-arg ctor (with promotion) — delegates to 4-arg
ChessMove::ChessMove(int sX, int sY, int eX, int eY, PieceType promo)
    : ChessMove(sX, sY, eX, eY) {
    if (!isEnd()) {
        promotion = promo;
        if (promo != PAWN) {
            // Append promotion letter: p/r/n/b/k/q indexed by PieceType enum
            const char letters[] = {'p', 'r', 'n', 'b', 'k', 'q'};
            repr[5] = letters[promo];
            repr[6] = '\0';
        }
    }
}

// String ctor — delegates to 4-arg; accepts "a1-b2" or "a1b2" format
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

ChessPiece::ChessPiece(bool isW, bool isKS, ChessBoard* b, int i)
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
    if (canMove(x, y) &&
        board !=
            nullptr)  // not sure what happens when *(nullptr) used as reference; can't be good.
    {
        // movePiece returns the displaced piece (if any) as a unique_ptr;
        // it is automatically deleted when the return value is dropped.
        (void)movePiece(*board, ChessMove(getPosX(), getPosY(), x, y));
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

Pawn::Pawn(bool isW, bool isKS, ChessBoard* b, int i) : ChessPiece(isW, isKS, b, i) {
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

    if (y == cy && board->getPiece(x, y) == nullptr)  // forward move
    {
        // Double advance: white unmoved pawn can jump from row 1 to row 3
        // (skipping row 2); black from row 6 to row 4 (skipping row 5).
        if (hasMoved == false && isWhite == true && x == 3 && board->getPiece(2, y) == nullptr)
            ;  // Would be return true; need to check for check
        else if (hasMoved == false && isWhite == false && x == 4 &&
                 board->getPiece(5, y) == nullptr)
            ;
        else if (isWhite == true && x == cx + 1)
            ;
        else if (isWhite == false && x == cx - 1)
            ;
        else
            return false;
    } else if ((y == cy + 1 || y == cy - 1) && board->getPiece(x, y) == nullptr) {  // en passant
        // En passant: white pawn must be on row 4 (opponent's double-advance
        // landing row); black pawn on row 3. The captured pawn sits on the
        // same row as the capturing pawn, one column over.
        if (isWhite == true && cx == 4 && board->getPiece(cx, y) != nullptr &&
            board->getPiece(cx, y)->getWhite() == false &&
            board->getPiece(cx, y)->getType() == PAWN) {
            const Pawn* piece = dynamic_cast<const Pawn*>(board->getPiece(cx, y));
            if (piece && piece->getEnPassant())
                ;
            else
                return false;
        } else if (isWhite == false && cx == 3 && board->getPiece(cx, y) != nullptr &&
                   board->getPiece(cx, y)->getWhite() == true &&
                   board->getPiece(cx, y)->getType() == PAWN) {
            const Pawn* piece = dynamic_cast<const Pawn*>(board->getPiece(cx, y));
            if (piece && piece->getEnPassant())
                ;
            else
                return false;
        } else
            return false;
    } else if ((y == cy + 1 || y == cy - 1) &&
               board->getPiece(x, y) != nullptr) {  // diagonal capture
        if (isWhite == true && x == cx + 1 && board->getPiece(x, y)->getWhite() == false)
            ;
        else if (isWhite == false && x == cx - 1 && board->getPiece(x, y)->getWhite() == true)
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
        // FIXME: en passant temp-undo doesn't remove the captured pawn, so
        // check detection is wrong for a horizontally-pinned en passant capture.
        // Tracked for a later PR.
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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

    bool destEmpty = board->getPiece(x, y) == nullptr;

    bool b = ChessPiece::move(x, y);
    if (b) {
        hasMoved = true;

        if (isWhite == true && x == ox + 1 && abs(y - oy) == 1 && destEmpty) {
            // En passant: remove the captured pawn. unique_ptr in setPiece handles deletion.
            setPiece(*board, ox, y, nullptr);
        } else if (isWhite == false && x == ox - 1 && abs(y - oy) == 1 && destEmpty) {
            setPiece(*board, ox, y, nullptr);
        }

        if (abs(ox - x) == 2) enPassant = true;
    }
    return b;

    // promotion to be covered elsewhere
}

PieceType Pawn::getType() const { return PAWN; }

//////
// ROOK

Rook::Rook(bool isW, bool isKS, ChessBoard* b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'R';
    hasMoved = false;
}

Rook::~Rook() {}

bool Rook::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    int cx = getPosX();
    int cy = getPosY();

    if (board->getPiece(x, y) != nullptr &&
        board->getPiece(x, y)->getWhite() == isWhite)  // capturing same-side piece.
        return false;

    if (x == cx) {
        if (y == cy)  // null moves don't count
            return false;
        else  // check space in between
        {
            if (y < cy) {
                for (int i = y + 1; i < cy; i++) {
                    if (board->getPiece(x, i) != nullptr) return false;
                };
            } else {  // y > cy
                for (int i = y - 1; i > cy; i--) {
                    if (board->getPiece(x, i) != nullptr) return false;
                };
            }
        }
    } else if (y != cy) {
        return false;
    } else {  // check space in between again
        if (x < cx) {
            for (int i = x + 1; i < cx; i++) {
                if (board->getPiece(i, y) != nullptr) return false;
            };
        } else {  // x > cx
            for (int i = x - 1; i > cx; i--) {
                if (board->getPiece(i, y) != nullptr) return false;
            };
        }
    }

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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

Knight::Knight(bool isW, bool isKS, ChessBoard* b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'N';
}

Knight::~Knight() {}

bool Knight::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board->getPiece(x, y) != nullptr && board->getPiece(x, y)->getWhite() == isWhite)
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
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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

Bishop::Bishop(bool isW, bool isKS, ChessBoard* b, int i) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'B';
}

Bishop::~Bishop() {}

bool Bishop::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    int cx = getPosX();
    int cy = getPosY();

    int diagDiff = cy - cx;
    int diagSum = cy + cx;

    if (board->getPiece(x, y) != nullptr &&
        board->getPiece(x, y)->getWhite() == isWhite)  // can't capture same side
        return false;

    if (y - x == diagDiff) {
        if (x == cx)
            return false;
        else if (x > cx) {
            for (int i = 1; (cy + i < y && cx + i < x); i++) {
                if (board->getPiece(cx + i, cy + i) != nullptr) return false;
            };
        } else {  // x < cx
            for (int i = 1; (cy - i > y && cx - i > x); i++) {
                if (board->getPiece(cx - i, cy - i) != nullptr) return false;
            };
        }
    } else if (y + x == diagSum) {
        if (x == cx)
            return false;
        else if (x > cx) {
            for (int i = 1; (cy - i > y && cx + i < x); i++) {
                if (board->getPiece(cx + i, cy - i) != nullptr) return false;
            };
        } else {  // x < cx
            for (int i = 1; (cy + i < y && cx - i > x); i++) {
                if (board->getPiece(cx - i, cy + i) != nullptr) return false;
            };
        }
    } else
        return false;

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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

King::King(bool isW, ChessBoard* b) : ChessPiece(isW, true, b, 0) {
    id[1] = 'K';
    hasMoved = false;
}

King::~King() {}

bool King::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board->getPiece(x, y) != nullptr &&
        board->getPiece(x, y)->getWhite() == isWhite)  // also disallows null move
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

            if (board->getPiece(cx, 4 + sign) == nullptr &&
                board->getPiece(cx, 4 + 2 * sign) == nullptr &&
                board->getPiece(cx, (7 + 7 * sign) / 2) != nullptr)  // check rooks
            {
                if (sign == -1 &&
                    board->getPiece(cx, 1) != nullptr)  // Queen side: b-file must also be empty
                    return false;

                if (board->getPiece(cx, (7 + 7 * sign) / 2)->getType() == ROOK &&
                    board->getPiece(cx, (7 + 7 * sign) / 2)->getWhite() == isWhite) {
                    const Rook* piece =
                        dynamic_cast<const Rook*>(board->getPiece(cx, (7 + 7 * sign) / 2));
                    if (!piece->getMoved()) {
                        // Step the king through each intermediate square and verify
                        // it is not in check at any point (castling through check is illegal).
                        (void)movePiece(*board, ChessMove(cx, 4, cx, 4 + sign));
                        if (chkchk && inCheck()) {
                            (void)movePiece(*board, ChessMove(cx, 4 + sign, cx, 4));
                            return false;
                        }

                        (void)movePiece(*board, ChessMove(cx, 4 + sign, cx, 4 + sign * 2));
                        if (chkchk && inCheck()) {
                            (void)movePiece(*board, ChessMove(cx, 4 + sign * 2, cx, 4));
                            return false;
                        }

                        (void)movePiece(*board, ChessMove(cx, 4 + sign * 2, cx, 4));
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
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(inCheck());
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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
            (void)movePiece(*board, ChessMove(ox, 7, ox, 5));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(*board, ox, 5));
            assert(piece);
            piece->markMoved();
        }
        if (y == 2 && oy == 4) {
            (void)movePiece(*board, ChessMove(ox, 0, ox, 3));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(*board, ox, 3));
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
            if (board->getPiece(i, j) != nullptr && board->getPiece(i, j)->getWhite() != isWhite &&
                board->getPiece(i, j)->canMove(cx, cy, false))
                return true;
        }
    }
    return false;
}

PieceType King::getType() const { return KING; }

///////
// QUEEN

Queen::Queen(bool isW, ChessBoard* b, int i, bool isKS) : ChessPiece(isW, isKS, b, i) {
    id[1] = 'Q';
}

Queen::~Queen() {}

bool Queen::canMove(int x, int y, bool chkchk) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return false;

    if (board->getPiece(x, y) != nullptr &&
        board->getPiece(x, y)->getWhite() == isWhite)  // also disallows null move
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
                    if (board->getPiece(x, i) != nullptr) return false;
                };
            } else {  // y > cy
                for (int i = y - 1; i > cy; i--) {
                    if (board->getPiece(x, i) != nullptr) return false;
                };
            }
        }
    } else if (y != cy) {
        if (y - x == diagDiff) {
            if (x == cx)
                return false;
            else if (x > cx) {
                for (int i = 1; (cy + i < y && cx + i < x); i++) {
                    if (board->getPiece(cx + i, cy + i) != nullptr) return false;
                };
            } else {  // x < cx
                for (int i = 1; (cy - i > y && cx - i > x); i++) {
                    if (board->getPiece(cx - i, cy - i) != nullptr) return false;
                };
            }
        } else if (y + x == diagSum) {
            if (x == cx)
                return false;
            else if (x > cx) {
                for (int i = 1; (cy - i > y && cx + i < x); i++) {
                    if (board->getPiece(cx + i, cy - i) != nullptr) return false;
                };
            } else {  // x < cx
                for (int i = 1; (cy + i < y && cx - i > x); i++) {
                    if (board->getPiece(cx - i, cy + i) != nullptr) return false;
                };
            }
        } else
            return false;
    } else {  // check space in between again
        if (x < cx) {
            for (int i = x + 1; i < cx; i++) {
                if (board->getPiece(i, y) != nullptr) return false;
            };
        } else {  // x > cx
            for (int i = x - 1; i > cx; i--) {
                if (board->getPiece(i, y) != nullptr) return false;
            };
        }
    }

    if (chkchk) {
        // Temporarily execute and undo the move to test for self-check (see Pawn::canMove).
        auto temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        (void)movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, std::move(temp));
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
    place(0, 0, std::make_unique<Rook>(WHITE, false, this, 0));
    place(0, 1, std::make_unique<Knight>(WHITE, false, this, 0));
    place(0, 2, std::make_unique<Bishop>(WHITE, false, this, 0));
    place(0, 3, std::make_unique<Queen>(WHITE, this, 0));
    place(0, 4, std::make_unique<King>(WHITE, this));
    place(0, 5, std::make_unique<Bishop>(WHITE, true, this, 0));
    place(0, 6, std::make_unique<Knight>(WHITE, true, this, 0));
    place(0, 7, std::make_unique<Rook>(WHITE, true, this, 0));

    place(1, 0, std::make_unique<Pawn>(WHITE, false, this, 4));
    place(1, 1, std::make_unique<Pawn>(WHITE, false, this, 3));
    place(1, 2, std::make_unique<Pawn>(WHITE, false, this, 2));
    place(1, 3, std::make_unique<Pawn>(WHITE, false, this, 1));
    place(1, 4, std::make_unique<Pawn>(WHITE, true, this, 1));
    place(1, 5, std::make_unique<Pawn>(WHITE, true, this, 2));
    place(1, 6, std::make_unique<Pawn>(WHITE, true, this, 3));
    place(1, 7, std::make_unique<Pawn>(WHITE, true, this, 4));

    place(6, 0, std::make_unique<Pawn>(BLACK, false, this, 4));
    place(6, 1, std::make_unique<Pawn>(BLACK, false, this, 3));
    place(6, 2, std::make_unique<Pawn>(BLACK, false, this, 2));
    place(6, 3, std::make_unique<Pawn>(BLACK, false, this, 1));
    place(6, 4, std::make_unique<Pawn>(BLACK, true, this, 1));
    place(6, 5, std::make_unique<Pawn>(BLACK, true, this, 2));
    place(6, 6, std::make_unique<Pawn>(BLACK, true, this, 3));
    place(6, 7, std::make_unique<Pawn>(BLACK, true, this, 4));

    place(7, 0, std::make_unique<Rook>(BLACK, false, this, 0));
    place(7, 1, std::make_unique<Knight>(BLACK, false, this, 0));
    place(7, 2, std::make_unique<Bishop>(BLACK, false, this, 0));
    place(7, 3, std::make_unique<Queen>(BLACK, this, 0));
    place(7, 4, std::make_unique<King>(BLACK, this));
    place(7, 5, std::make_unique<Bishop>(BLACK, true, this, 0));
    place(7, 6, std::make_unique<Knight>(BLACK, true, this, 0));
    place(7, 7, std::make_unique<Rook>(BLACK, true, this, 0));
}

ChessBoard::~ChessBoard() {}  // unique_ptrs in grid[] clean up automatically

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
        // Update position cache for the piece that just moved
        if (grid[move.getEndX()][move.getEndY()]) {
            grid[move.getEndX()][move.getEndY()]->posX = move.getEndX();
            grid[move.getEndX()][move.getEndY()]->posY = move.getEndY();
        }
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

ChessGame::ChessGame() : rulesOn(true), whiteTurn(true), board(ChessBoard()) {}

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

std::unique_ptr<ChessPiece> ChessGame::makePiece(PieceType type, bool white, int y,
                                                   ChessBoard* b) {
    bool ks = (y > 3);
    int idx = white ? whiteProms : blackProms;
    switch (type) {
        case QUEEN:
            return std::make_unique<Queen>(white, b, idx, ks);
        case ROOK:
            return std::make_unique<Rook>(white, ks, b, idx);
        case KNIGHT:
            return std::make_unique<Knight>(white, ks, b, idx);
        case BISHOP:
            return std::make_unique<Bishop>(white, ks, b, idx);
        default:
            assert(false);
            return nullptr;
    }
}

bool ChessGame::getTurn() const { return whiteTurn; }

bool ChessGame::makeMove(const ChessMove& cm) {
    if (rulesOn) {
        ChessPiece* piece = board.getMoveablePiece(cm.getStartX(), cm.getStartY());
        if (piece == nullptr || piece->getWhite() != whiteTurn) return false;
        bool b = piece->move(cm.getEndX(), cm.getEndY());
        if (b) {
            bool movedColor = whiteTurn;  // capture before flip
            // Handle pawn promotion: replace pawn with requested piece type.
            if (cm.getPromotion() != PAWN) {
                int endX = cm.getEndX(), endY = cm.getEndY();
                rulesOn = false;
                setPiece(endX, endY,
                         makePiece(cm.getPromotion(), movedColor, endY, &board));
                rulesOn = true;
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

ChessBoard* ChessGame::getPieceBoard() { return &board; }
