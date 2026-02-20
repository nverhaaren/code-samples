// This is the chess.h file which is
// the header file for the chess classes which ?

/*                                                                                                 1
         1         2         3         4         5         6         7         8         9         0         1         2
1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
*/

#ifndef _CHESS_H_
 #define _CHESS_H_

#include <string.h>

#ifdef NULL
 #if (NULL != 0 )
  #warning NULL set to a value other than 0
 #endif
#endif

#ifndef NULL
 #define NULL 0
#endif

///////////
//CONSTANTS
const bool WHITE = true;
const bool BLACK = false;

/////////////////////
//DATA TYPES DECLARED

class ChessPiece;    //ADT
 class Pawn;
 class Rook;
 class Knight;
 class Bishop;
 class King;
 class Queen;

class ChessBoard;
class ChessMove;
class ChessGame;

// Board coordinate convention:
//   x = row  (0 = white back rank, 7 = black back rank)
//   y = column (0 = a-file/queenside, 7 = h-file/kingside)
// Algebraic notation maps as: "a1" -> y=0, x=0

enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, KING, QUEEN };

///////////////////////
//CLASS HEADERS IN FULL

/**
 * Abstract base class for all chess pieces.
 *
 * Pieces do not cache their board position; getPosX/getPosY scan the
 * full 8x8 grid on every call (O(64)).
 *
 * Piece ID string (szID): [color W/B][type P/R/N/B/K/Q][side K/Q][index digit]
 */
class ChessPiece    // ADT
{
 public:
    ChessPiece( bool isW, bool isKS, ChessBoard * b, int i = 0 );
    virtual ~ChessPiece();
    bool getWhite() const;
    bool getKingSide() const;
    int getIndex() const;
    // Position is not cached; each call scans the full 8x8 board (O(64)).
    int getPosX() const;
    int getPosY() const;
    const char * getID() const;

    /**
     * Returns true if this piece can legally move to (x, y).
     *
     * The chkchk parameter controls self-check detection. The three methods
     * involved — canMove, ChessBoard::checkCheck, and King::inCheck — form a
     * mutually recursive cycle that chkchk is used to break:
     *
     *   canMove(chkchk=true)
     *     Temporarily executes the move on the board, then calls
     *     checkCheck (or King::inCheck directly for the King) to test
     *     whether the moving side's king is now in check. Restores the
     *     board afterward regardless of outcome.
     *
     *   ChessBoard::checkCheck(isW)
     *     Finds the king of color isW on the board and calls King::inCheck().
     *
     *   King::inCheck()
     *     Scans all enemy pieces and calls canMove(king_x, king_y, chkchk=false)
     *     on each. chkchk=false is essential here: if it were true, each enemy
     *     piece would call checkCheck, which calls inCheck, which calls canMove
     *     again — infinite recursion. With chkchk=false we only check whether
     *     the enemy piece geometrically reaches the king, not whether doing so
     *     would expose the enemy's own king.
     */
    virtual bool canMove( int x, int y, bool chkchk = true ) const = 0;

    /**
     * Returns a heap-allocated, ChessMove::end-terminated array of all legal
     * moves for this piece. The caller is responsible for delete[]-ing it.
     */
    virtual ChessMove * getMoves() const = 0;
    virtual bool move( int x, int y );
    virtual PieceType getType() const = 0;

    virtual double getValue();
    virtual int getRootValue();

    const static char szDigits[10 + 1];

    friend class ChessBoard;

 protected:
    const bool isWhite;
    const bool isKingSide;
    char szID[4 + 1];
    ChessBoard * board;
    const int index;

    // Virtual functions cannot be friended directly. These non-virtual
    // forwarding methods give piece subclasses access to ChessBoard's private
    // movePiece, getMoveablePiece, and setPiece methods.
    ChessPiece * f_movePiece( ChessBoard & cb, ChessMove move ) const;
    ChessPiece * f_getMoveablePiece( ChessBoard & cb, int x, int y ) const;
    ChessPiece * f_setPiece( ChessBoard & cb, int x, int y, ChessPiece * piece ) const;
};

class Pawn : public ChessPiece
{
 public:
    Pawn( bool isW, bool isKS, ChessBoard * b, int i );
    ~Pawn();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    bool getMoved () const;
    bool getEnPassant() const;
    void setEnPassant( bool b );
    bool move( int x, int y );
    PieceType getType() const;
 private:
    bool hasMoved;
    bool enPassant;
};

class Rook : public ChessPiece
{
 public:
    Rook( bool isW, bool isKS, ChessBoard * b, int i = 0 );
    ~Rook();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    bool getMoved() const;
    bool move( int x, int y );
    void markMoved();
    PieceType getType() const;
 private:
    bool hasMoved;
};

class Knight : public ChessPiece
{
 public:
    Knight( bool isW, bool isKS, ChessBoard * b, int i = 0 );
    ~Knight();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    PieceType getType() const;
    const static int xOffsets[8];
    const static int yOffsets[8];
};

class Bishop : public ChessPiece
{
 public:
    Bishop( bool isW, bool isKS, ChessBoard * b, int i = 0 );
    ~Bishop();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    PieceType getType() const;
};

class King : public ChessPiece
{
 public:
    King( bool isW, ChessBoard * b );
    ~King();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    bool getMoved() const;
    bool move( int x, int y );
    bool inCheck() const;
    PieceType getType() const;
    const static int xOffsets[10];
    const static int yOffsets[10];
 protected:
    bool hasMoved;
};

class Queen : public ChessPiece
{
 public:
    Queen( bool isW, ChessBoard * b, int i = 0, bool isKS = false );
    ~Queen();
    bool canMove( int x, int y, bool chkchk = true ) const;
    ChessMove * getMoves() const;
    PieceType getType() const;
};
 
/** Owns and manages the 8x8 grid of pieces. */
class ChessBoard
{
 public:
    ChessBoard();
    ~ChessBoard();
    const ChessPiece * getPiece( int x, int y ) const;

    /**
     * Returns true if the king of the given color is in check.
     *
     * If no king of the given color is found on the board, returns true as a
     * safe error sentinel. This halts the game (getMoves returns empty, no
     * moves can be made) rather than allowing moves in an invalid state.
     * A missing king can arise when the board is configured manually via
     * ChessGame::setPiece() with rules disabled — for example during pawn
     * promotion or in test setups — without placing a king.
     */
    bool checkCheck( bool isW ) const;

    const char * toString();

    friend class ChessGame;
 private:
    ChessPiece * aapGrid[8][8];
    ChessPiece * movePiece( ChessMove move );
    ChessPiece * getMoveablePiece( int x, int y );
    friend ChessPiece * ChessPiece::f_movePiece( ChessBoard & cb, ChessMove move ) const;   // virtual functions cannot be friends; these functions
    friend ChessPiece * ChessPiece::f_getMoveablePiece( ChessBoard & cb, int x, int y ) const;  // should allow ChessPiece to get what it needs
    friend ChessPiece * ChessPiece::f_setPiece( ChessBoard & cb, int x, int y, ChessPiece * piece ) const;
    
    char * szForm;
};

/**
 * Encodes a chess move as a packed short int: four 3-bit fields for
 * startX, startY, endX, endY (12 bits total).
 *
 * The default-constructed move (data==0) serves as a list sentinel
 * (ChessMove::end), analogous to '\0' in a C string. Move arrays
 * returned by getMoves() are heap-allocated and terminated by this
 * sentinel; callers must delete[] them.
 *
 * String constructor accepts "a1-b2" or "a1b2" format.
 */
class ChessMove
{
 public:
    ChessMove();
    ChessMove( int sX, int sY, int eX, int eY );
    ChessMove( const char * const pszMove );
    ChessMove( const ChessMove & rcm );
    ~ChessMove();
    ChessMove & operator=( const ChessMove & rhs );
    int getStartX() const;
    int getStartY() const;
    int getEndX() const;
    int getEndY() const;
    static const ChessMove end;
    bool isEnd() const;
    
    const static char szlcLetters[8 + 1];
    const static int MAXCMLENGTH;
    
    static void swap( ChessMove & cm1, ChessMove & cm2 );
    static void sort( ChessMove * acm, int size );
    static void concat( ChessMove * acm1, const ChessMove * acm2 );
    static int length( const ChessMove * acm );
    static void copy( ChessMove * acm1, const ChessMove * acm2 );
    
    const char * toString() const;
 private:
    short int data;
    char szForm[6];
    void init( int sX, int sY, int eX, int eY );
};

/**
 * Top-level game controller. Enforces turn order, manages en passant
 * expiry after each move, and detects checkmate/stalemate.
 *
 * Pawn promotion is intentionally handled outside this class (in Main.cpp)
 * because it requires player input to select the promoted piece type.
 */
class ChessGame
{
 public:
    ChessGame();
    ~ChessGame();

    void setRules( bool on );
    bool getRules() const;
    
    bool checkmate( bool white ) const;
    bool stalemate( bool turn ) const;
    
    ChessMove * getMoves( bool white ) const;
    
    bool makeMove( const ChessMove & cm );
    
    const char * getBoard();
    const ChessPiece * getPiece( int x, int y ) const;
    void setPiece( int x, int y, ChessPiece * piece );
    
    bool getTurn() const;
    
    ChessBoard * getPieceBoard();
 private:
    bool rulesOn;
    bool whiteTurn;
    ChessBoard board;
};

#endif //def _CHESS_H
