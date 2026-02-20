// This is the chess.cpp file which contains implementations for the headers in chess.h

#include "chess.h"

#include <cstdlib>

using std::abs;

///////////
// CHESSMOVE
const ChessMove ChessMove::end = ChessMove();
const char ChessMove::fileLetters[8 + 1] = "abcdefgh";
const int ChessMove::maxLength = 1024;  // 669 should be sufficient for all moves of both sides, FYI

ChessMove::ChessMove() : data(0) {
    repr[3] = '\0';
    repr[0] = 'E';
    repr[1] = 'N';
    repr[2] = 'D';
    repr[4] = '\0';
    repr[5] = '\0';
}

ChessMove::ChessMove(int sX, int sY, int eX, int eY) : data(0) { init(sX, sY, eX, eY); }

ChessMove::ChessMove(const char* const str) : data(0) {
    int sY = (int)(str[0] - 'a');
    int sX = (int)(str[1] - '1');

    int eX, eY;

    if (str[2] == ' ' || str[2] == '-') {
        eY = (int)(str[3] - 'a');
        eX = (int)(str[4] - '1');
    } else {
        eY = (int)(str[2] - 'a');
        eX = (int)(str[3] - '1');
    }
    init(sX, sY, eX, eY);
}

ChessMove::ChessMove(const ChessMove& rcm) : data(rcm.data) {
    for (int i = 0; i < 6; i++) {
        repr[i] = rcm.repr[i];
    }
}

ChessMove::~ChessMove() {}

ChessMove& ChessMove::operator=(const ChessMove& rhs) {
    if (this == &rhs) return *this;

    data = rhs.data;
    for (int i = 0; i < 6; i++) {
        repr[i] = rhs.repr[i];
    }
    return *this;
}

int ChessMove::getStartX() const { return data & 0x7; }
int ChessMove::getStartY() const { return (data & 0x38) / 0x8; }
int ChessMove::getEndX() const { return (data & 0x1c0) / 0x40; }
int ChessMove::getEndY() const { return (data & 0xe00) / 0x200; }

bool ChessMove::isEnd() const { return (data == 0); }

void ChessMove::swap(ChessMove& cm1, ChessMove& cm2) {
    ChessMove temp = cm1;
    cm1 = cm2;
    cm2 = temp;
}

void ChessMove::sort(ChessMove* acm, int size) {
    for (int j, i = 0; i < size - 1; i++)  // sort the ::ends to the end ( ::end like \0 in string )
    {
        if (acm[i].isEnd()) {
            for (j = i + 1; j < size; j++) {
                if (!acm[j].isEnd()) {
                    ChessMove::swap(acm[i], acm[j]);
                    break;
                }
            }
            if (j == size)  // nothing to swap with?
                break;
        }
    }
}

int ChessMove::length(ChessMove const* cm) {
    if (cm == nullptr) return 0;
    int l;
    for (l = 0; !(cm[l].isEnd()) && l < maxLength; l++);
    return l;
}

void ChessMove::copy(ChessMove* cm1, const ChessMove* cm2) {
    int l = length(cm2);
    for (int i = 0; i <= l; i++) {
        cm1[i] = cm2[i];
    }
}

void ChessMove::concat(ChessMove* cm1, const ChessMove* cm2) {
    int l = length(cm1);
    if (cm1[l].isEnd()) copy(cm1 + l, cm2);
}

const char* ChessMove::toString() const { return repr; }

void ChessMove::init(int sX, int sY, int eX, int eY) {
    if (sX < 0 || sX > 7 || sY < 0 || sY > 7 || eX < 0 || eX > 7 || eY < 0 || eY > 7)
        ;
    else {
        short int startX = (short int)sX;
        short int startY = (short int)sY;
        short int endX = (short int)eX;
        short int endY = (short int)eY;
        data += startX;
        data += startY * 8;
        data += endX * 8 * 8;
        data += endY * 8 * 8 * 8;
    }

    if (!isEnd()) {
        repr[5] = '\0';
        repr[2] = '-';
        repr[0] = fileLetters[getStartY()];
        repr[1] = ChessPiece::digits[getStartX() + 1];
        repr[3] = fileLetters[getEndY()];
        repr[4] = ChessPiece::digits[getEndX() + 1];
    } else {
        repr[3] = '\0';
        repr[0] = 'E';
        repr[1] = 'N';
        repr[2] = 'D';
        repr[4] = '\0';
        repr[5] = '\0';
    }
}

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

int ChessPiece::getPosX() const  // O(64) scan; position is not stored on the piece
{
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->getPiece(i, j) == this) return i;
        }
    }
    return -1;
}

int ChessPiece::getPosY() const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->getPiece(i, j) == this) return j;
        }
    }
    return -1;
}

const char* ChessPiece::getID() const { return id; }

bool ChessPiece::move(int x, int y) {
    if (canMove(x, y) &&
        board !=
            nullptr)  // not sure what happens when *(nullptr) used as reference; can't be good.
    {
        delete getMutablePiece(*board, x, y);  // check pointers !
        movePiece(*board,
                  ChessMove(getPosX(), getPosY(), x, y));  // and this will overwrite the pointer
        return true;
    } else
        return false;
}

ChessPiece* ChessPiece::movePiece(ChessBoard& cb, ChessMove move) const {
    return cb.movePiece(move);
}

ChessPiece* ChessPiece::getMutablePiece(ChessBoard& cb, int x, int y) const {
    return cb.getMoveablePiece(x, y);
}

ChessPiece* ChessPiece::setPiece(ChessBoard& cb, int x, int y, ChessPiece* piece) const {
    if (x < 0 || x > 7 || y < 0 || y > 7)
        return nullptr;
    else {
        ChessPiece* temp = cb.grid[x][y];
        cb.grid[x][y] = piece;
        return temp;  // function caller's responsibility to prevent memory holes.
    }
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* Pawn::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    int sign = 1;
    if (!isWhite) sign -= 2;

    ChessMove* ret = new ChessMove[4 + 1];
    if (!ret) {
        return nullptr;
    }

    ret[4] = ChessMove::end;

    if (canMove(x + sign, y))
        ret[0] = ChessMove(x, y, x + sign, y);
    else
        ret[0] = ChessMove::end;

    if (canMove(x + 2 * sign, y))
        ret[1] = ChessMove(x, y, x + 2 * sign, y);
    else
        ret[1] = ChessMove::end;

    if (canMove(x + sign, y + 1))
        ret[2] = ChessMove(x, y, x + sign, y + 1);
    else
        ret[2] = ChessMove::end;

    if (canMove(x + sign, y - 1))
        ret[3] = ChessMove(x, y, x + sign, y - 1);
    else
        ret[3] = ChessMove::end;

    ChessMove::sort(ret, 5);  // sort the ::ends to the end
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
            ChessPiece* capturedPawn = getMutablePiece(*board, ox, y);
            delete capturedPawn;
            setPiece(*board, ox, y, nullptr);
        } else if (isWhite == false && x == ox - 1 && abs(y - oy) == 1 && destEmpty) {
            ChessPiece* capturedPawn = getMutablePiece(*board, ox, y);
            delete capturedPawn;
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* Rook::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    ChessMove* ret = new ChessMove[14 + 1];
    if (!ret) return nullptr;

    for (int i = 0; i < 15; i++)  // all blank to start
    {
        ret[i] = ChessMove::end;
    }

    for (int i = 0, j = 0; i < 8; i++)  // add move in the same row
    {
        if (y != i) {
            if (canMove(x, i)) ret[j] = ChessMove(x, y, x, i);
            j++;
        }
    }

    for (int i = 0, j = 7; i < 8; i++)  // add moves in the same column
    {
        if (x != i) {
            if (canMove(i, y)) ret[j] = ChessMove(x, y, i, y);
            j++;
        }
    }

    ChessMove::sort(ret, 15);
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* Knight::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    ChessMove* ret = new ChessMove[8 + 1];
    if (!ret) return nullptr;

    for (int i = 0; i < 9; i++) {
        ret[i] = ChessMove::end;
    }

    for (int i = 0; i < 8; i++) {
        if (canMove(x + xOffsets[i], y + yOffsets[i]))
            ret[i] = ChessMove(x, y, x + xOffsets[i], y + yOffsets[i]);
    }

    ChessMove::sort(ret, 9);
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* Bishop::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    ChessMove* ret = new ChessMove[13 + 1];
    if (!ret) return nullptr;

    for (int i = 0; i < 14; i++) {
        ret[i] = ChessMove::end;
    }

    int i = 0;
    for (int j = 0; (x + j < 8 && y + j < 8); j++) {
        if (canMove(x + j, y + j)) {
            ret[i] = ChessMove(x, y, x + j, y + j);
            i++;
        }
    }

    for (int j = 0; (x - j >= 0 && y - j >= 0); j++) {
        if (canMove(x - j, y - j)) {
            ret[i] = ChessMove(x, y, x - j, y - j);
            i++;
        }
    }

    for (int j = 0; (x + j < 8 && y - j >= 0); j++) {
        if (canMove(x + j, y - j)) {
            ret[i] = ChessMove(x, y, x + j, y - j);
            i++;
        }
    }

    for (int j = 0; (x - j >= 0 && y + j < 8); j++) {
        if (canMove(x - j, y + j)) {
            ret[i] = ChessMove(x, y, x - j, y + j);
            i++;
        }
    }

    ChessMove::sort(ret, 14);  // shouldn't be necessary; if so returns quickly.
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
                        movePiece(*board, ChessMove(cx, 4, cx, 4 + sign));
                        if (chkchk && inCheck()) {
                            movePiece(*board, ChessMove(cx, 4 + sign, cx, 4));
                            return false;
                        }

                        movePiece(*board, ChessMove(cx, 4 + sign, cx, 4 + sign * 2));
                        if (chkchk && inCheck()) {
                            movePiece(*board, ChessMove(cx, 4 + sign * 2, cx, 4));
                            return false;
                        }

                        movePiece(*board, ChessMove(cx, 4 + sign * 2, cx, 4));
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(inCheck());
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* King::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    ChessMove* ret = new ChessMove[10 + 1];
    if (!ret) return nullptr;

    for (int i = 0; i < 11; i++) {
        ret[i] = ChessMove::end;
    }

    for (int i = 0; i < 10; i++) {
        if (canMove(x + xOffsets[i], y + yOffsets[i]))
            ret[i] = ChessMove(x, y, x + xOffsets[i], y + yOffsets[i]);
    }

    ChessMove::sort(ret, 11);
    return ret;
}

bool King::move(int x, int y) {
    int ox = getPosX();
    int oy = getPosY();

    bool b = ChessPiece::move(x, y);
    if (b) {
        hasMoved = true;
        if (y == 6 && oy == 4) {
            movePiece(*board, ChessMove(ox, 7, ox, 5));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(*board, ox, 5));
            piece->markMoved();
        }
        if (y == 2 && oy == 4) {
            movePiece(*board, ChessMove(ox, 0, ox, 3));
            Rook* piece = dynamic_cast<Rook*>(getMutablePiece(*board, ox, 3));
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
        ChessPiece* temp = movePiece(*board, ChessMove(cx, cy, x, y));
        bool ret = !(board->checkCheck(isWhite));
        movePiece(*board, ChessMove(x, y, cx, cy));
        setPiece(*board, x, y, temp);
        return ret;
    } else
        return true;
}

ChessMove* Queen::getMoves() const {
    int x = getPosX();
    int y = getPosY();

    ChessMove* ret = new ChessMove[27 + 1];
    if (!ret) return nullptr;

    for (int i = 0; i < 28; i++)  // all blank to start
    {
        ret[i] = ChessMove::end;
    }

    for (int i = 0, j = 0; i < 8; i++)  // add move in the same row
    {
        if (y != i) {
            if (canMove(x, i)) ret[j] = ChessMove(x, y, x, i);
            j++;
        }
    }

    for (int i = 0, j = 7; i < 8; i++)  // add moves in the same column
    {
        if (x != i) {
            if (canMove(i, y)) ret[j] = ChessMove(x, y, i, y);
            j++;
        }
    }

    int i = 14;
    for (int j = 0; (x + j < 8 && y + j < 8); j++) {
        if (canMove(x + j, y + j)) {
            ret[i] = ChessMove(x, y, x + j, y + j);
            i++;
        }
    }

    for (int j = 0; (x - j >= 0 && y - j >= 0); j++) {
        if (canMove(x - j, y - j)) {
            ret[i] = ChessMove(x, y, x - j, y - j);
            i++;
        }
    }

    for (int j = 0; (x + j < 8 && y - j >= 0); j++) {
        if (canMove(x + j, y - j)) {
            ret[i] = ChessMove(x, y, x + j, y - j);
            i++;
        }
    }

    for (int j = 0; (x - j >= 0 && y + j < 8); j++) {
        if (canMove(x - j, y + j)) {
            ret[i] = ChessMove(x, y, x - j, y + j);
            i++;
        }
    }

    ChessMove::sort(ret, 28);
    return ret;
}

PieceType Queen::getType() const { return QUEEN; }

////////////
// CHESSBOARD

ChessBoard::ChessBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            grid[i][j] = nullptr;
        }
    }

    // Set up the pieces

    grid[0][0] = new Rook(WHITE, false, this, 0);
    grid[0][1] = new Knight(WHITE, false, this, 0);
    grid[0][2] = new Bishop(WHITE, false, this, 0);
    grid[0][3] = new Queen(WHITE, this, 0);
    grid[0][4] = new King(WHITE, this);
    grid[0][5] = new Bishop(WHITE, true, this, 0);
    grid[0][6] = new Knight(WHITE, true, this, 0);
    grid[0][7] = new Rook(WHITE, true, this, 0);

    grid[1][0] = new Pawn(WHITE, false, this, 4);
    grid[1][1] = new Pawn(WHITE, false, this, 3);
    grid[1][2] = new Pawn(WHITE, false, this, 2);
    grid[1][3] = new Pawn(WHITE, false, this, 1);
    grid[1][4] = new Pawn(WHITE, true, this, 1);
    grid[1][5] = new Pawn(WHITE, true, this, 2);
    grid[1][6] = new Pawn(WHITE, true, this, 3);
    grid[1][7] = new Pawn(WHITE, true, this, 4);

    grid[6][0] = new Pawn(BLACK, false, this, 4);
    grid[6][1] = new Pawn(BLACK, false, this, 3);
    grid[6][2] = new Pawn(BLACK, false, this, 2);
    grid[6][3] = new Pawn(BLACK, false, this, 1);
    grid[6][4] = new Pawn(BLACK, true, this, 1);
    grid[6][5] = new Pawn(BLACK, true, this, 2);
    grid[6][6] = new Pawn(BLACK, true, this, 3);
    grid[6][7] = new Pawn(BLACK, true, this, 4);

    grid[7][0] = new Rook(BLACK, false, this, 0);
    grid[7][1] = new Knight(BLACK, false, this, 0);
    grid[7][2] = new Bishop(BLACK, false, this, 0);
    grid[7][3] = new Queen(BLACK, this, 0);
    grid[7][4] = new King(BLACK, this);
    grid[7][5] = new Bishop(BLACK, true, this, 0);
    grid[7][6] = new Knight(BLACK, true, this, 0);
    grid[7][7] = new Rook(BLACK, true, this, 0);
}

ChessBoard::~ChessBoard()  // do not keep pointers to pieces in this grid after the board is
                           // destroyed
{
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (grid[i][j] != nullptr) {
                delete grid[i][j];
                grid[i][j] = nullptr;
            }
        }
    }
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
        return grid[x][y];
    else
        return nullptr;
}

ChessPiece* ChessBoard::movePiece(ChessMove move) {
    if (!move.isEnd() && grid[move.getStartX()][move.getStartY()] != nullptr) {
        ChessPiece* temp = grid[move.getEndX()][move.getEndY()];
        grid[move.getEndX()][move.getEndY()] =
            grid[move.getStartX()][move.getStartY()];        // replaces destination
        grid[move.getStartX()][move.getStartY()] = nullptr;  // leaves behind nothing
        return temp;
    } else
        return nullptr;
}

const ChessPiece* ChessBoard::getPiece(int x, int y) const {
    if (x >= 0 && x <= 7 && y >= 0 && y <= 7)
        return grid[x][y];
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

    ChessMove* moves = getMoves(white);
    if (moves && moves->isEnd()) {
        delete[] moves;
        return true;
    } else {
        delete[] moves;
        return false;
    }
}

bool ChessGame::stalemate(bool turn) const {
    if (board.checkCheck(turn)) return false;

    ChessMove* moves = getMoves(turn);
    if (moves && moves->isEnd()) {
        delete[] moves;
        return true;
    } else {
        delete[] moves;
        return false;
    }
}

ChessMove* ChessGame::getMoves(bool white) const {
    int num = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board.getPiece(i, j) != nullptr && board.getPiece(i, j)->getWhite() == white) num++;
        }
    }

    if (num == 0) return nullptr;

    ChessMove** moveSets = new ChessMove*[num];
    for (int i = 0, k = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board.getPiece(i, j) != nullptr && board.getPiece(i, j)->getWhite() == white) {
                moveSets[k] = board.getPiece(i, j)->getMoves();
                k++;
            }
        }
    }

    int numMoves = 0;
    for (int i = 0; i < num; i++) {
        numMoves += ChessMove::length(moveSets[i]);
    }

    if (numMoves == 0) {
        ChessMove* ret = new ChessMove[1];
        ret[0] = ChessMove::end;
        return ret;
    }

    ChessMove* ret = new ChessMove[numMoves + 1];
    ret[numMoves] = ChessMove::end;

    ChessMove::copy(ret, moveSets[0]);

    for (int i = 1; i < num; i++) {
        ChessMove::concat(ret, moveSets[i]);
    }

    for (int i = 0; i < num; i++) {
        delete[] moveSets[i];
        moveSets[i] = nullptr;
    }
    delete[] moveSets;
    moveSets = nullptr;

    return ret;
}

bool ChessGame::getTurn() const { return whiteTurn; }

bool ChessGame::makeMove(const ChessMove& cm) {
    if (rulesOn) {
        ChessPiece* piece = board.getMoveablePiece(cm.getStartX(), cm.getStartY());
        if (piece == nullptr || piece->getWhite() != whiteTurn) return false;
        bool b = piece->move(cm.getEndX(), cm.getEndY());
        if (b) {
            whiteTurn = !whiteTurn;
            // Clear en passant flags for the side whose turn it now is.
            // After the flip, whiteTurn is the color that did NOT just move —
            // their en passant window (set when they double-advanced last turn)
            // has now expired because the opponent completed a move.
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (board.getPiece(i, j) != nullptr &&
                        board.getPiece(i, j)->getWhite() == whiteTurn) {
                        ChessPiece* piece = board.getMoveablePiece(i, j);
                        if (piece->getType() == PAWN) {
                            Pawn* pawn = dynamic_cast<Pawn*>(piece);
                            pawn->setEnPassant(false);
                        }
                    }
                }
            }
        }
        return b;
    } else {
        ChessPiece* piece = board.movePiece(cm);
        delete piece;
        piece = nullptr;
        return true;
    }
}

const char* ChessGame::getBoard() { return board.toString(); }

const ChessPiece* ChessGame::getPiece(int x, int y) const { return board.getPiece(x, y); }

void ChessGame::setPiece(int x, int y, ChessPiece* piece) {
    if (rulesOn) return;

    if (x < 0 || x > 7 || y < 0 || y > 7) return;

    delete board.grid[x][y];
    board.grid[x][y] = piece;
}

ChessBoard* ChessGame::getPieceBoard() { return &board; }
