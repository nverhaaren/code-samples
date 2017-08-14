// This is the chess.cpp file which contains implementations for the headers in chess.h


#include "chess.h"

/*                                                                                                 1
         1         2         3         4         5         6         7         8         9         0         1         2
1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
*/

int abs(int x) { return x < 0 ? -x : x; }

///////////
//CHESSMOVE
const ChessMove ChessMove::end = ChessMove();
const char ChessMove::szlcLetters[8 + 1] = "abcdefgh";
const int ChessMove::MAXCMLENGTH = 1024;                // 669 should be sufficient for all moves of both sides, FYI

ChessMove::ChessMove():
data(0) 
{
    szForm[3] = '\0';
    szForm[0] = 'E';
    szForm[1] = 'N';
    szForm[2] = 'D';
    szForm[4] = '\0';
    szForm[5] = '\0';
}

ChessMove::ChessMove( int sX, int sY, int eX, int eY ):
data(0)
{
    init( sX, sY, eX, eY );

}

ChessMove::ChessMove( const char * const pszMove ):
data(0)
{
    int sY = (int)(pszMove[0] - 'a');
    int sX = (int)(pszMove[1] - '1');
    
    int eX, eY;
    
    if ( pszMove[2] == ' '  ||  pszMove[2] == '-' )
    {
        eY = (int)(pszMove[3] - 'a');
        eX = (int)(pszMove[4] - '1');
    } else {
        eY = (int)(pszMove[2] - 'a');
        eX = (int)(pszMove[3] - '1');
    }
    init( sX, sY, eX, eY );
}

ChessMove::ChessMove( const ChessMove & rcm ):
data( rcm.data )
{
    for( int i = 0; i < 6; i++ )
    {
        szForm[i] = rcm.szForm[i];
    }
}


ChessMove::~ChessMove()
{}

ChessMove & ChessMove::operator=( const ChessMove & rhs )
{
    if ( this == &rhs )
        return *this;
    
    data = rhs.data;
    for( int i = 0; i < 6; i++ )
    {
        szForm[i] = rhs.szForm[i];
    }
    return *this;
}

int ChessMove::getStartX() const { return data & 0x7; }
int ChessMove::getStartY() const { return ( data & 0x38 ) / 0x8 ; }
int ChessMove::getEndX() const { return ( data & 0x1c0 ) / 0x40; }
int ChessMove::getEndY() const { return ( data & 0xe00 ) / 0x200 ; }

bool ChessMove::isEnd() const
{
    return ( data == 0 );
}

void ChessMove::swap( ChessMove & cm1, ChessMove & cm2 )
{
    ChessMove temp = cm1;
    cm1 = cm2;
    cm2 = temp;
}

void ChessMove::sort( ChessMove * acm, int size )
{
    for( int j, i = 0; i < size - 1; i++ )            // sort the ::ends to the end ( ::end like \0 in string )
    {
        if ( acm[i].isEnd() )
        {
            for( j = i + 1; j < size; j++ )
            {
                if ( !acm[j].isEnd() )
                {
                    ChessMove::swap( acm[i], acm[j] );
                    break;
                }
            }
            if ( j == size )       // nothing to swap with?
                break;
        }
    }
}

int ChessMove::length( ChessMove const * cm )
{
    if ( cm == NULL )
        return 0;
    int l;
    for( l = 0; !(cm[l].isEnd())  &&  l < MAXCMLENGTH; l++ );
    return l;
}

void ChessMove::copy( ChessMove * cm1, const ChessMove * cm2 )
{
    int l = length( cm2 );
    for( int i = 0; i <= l; i++ )
    {
        cm1[i] = cm2[i];
    }
}

void ChessMove::concat( ChessMove * cm1, const ChessMove * cm2 )
{
    int l = length( cm1 );
    if ( cm1[l].isEnd() )
        copy( cm1 + l, cm2 );
}

const char * ChessMove::toString() const
{
    return szForm;
}

void ChessMove::init( int sX, int sY, int eX, int eY )
{
    if ( sX < 0  ||  sX > 7  || sY < 0  ||  sY > 7  ||
         eX < 0  ||  eX > 7  || eY < 0  ||  eY > 7 )
        ;
    else
    {
        short int startX = (short int)sX;
        short int startY = (short int)sY;
        short int endX = (short int)eX;
        short int endY = (short int)eY;
        data += startX;
        data += startY * 8;
        data += endX * 8 * 8;
        data += endY * 8 * 8 * 8;
    }
    
    if ( !isEnd() )
    {
        szForm[5] = '\0';
        szForm[2] = '-';
        szForm[0] = szlcLetters[getStartY()];
        szForm[1] = ChessPiece::szDigits[getStartX()+1];
        szForm[3] = szlcLetters[getEndY()];
        szForm[4] = ChessPiece::szDigits[getEndX()+1];
    } else {
        szForm[3] = '\0';
        szForm[0] = 'E';
        szForm[1] = 'N';
        szForm[2] = 'D';
        szForm[4] = '\0';
        szForm[5] = '\0';
    }
}

////////////
//CHESSPIECE
const char ChessPiece::szDigits[11] = "0123456789";

ChessPiece::ChessPiece( bool isW, bool isKS, ChessBoard * b, int i ):
isWhite(isW), isKingSide(isKS), board(b), index(i)
{
    for( int j = 0; j < 4 + 1; j++ )
    {
        szID[j] = '\0';
    }
    
    if ( isW )
        szID[0] = 'W';
    else
        szID[0] = 'B';
        
    if ( isKS )
        szID[2] = 'K';
    else
        szID[2] = 'Q';
    
    if ( i <= 10  &&  i >= 0 )
        szID[3] = szDigits[i];
}

ChessPiece::~ChessPiece() {}

int ChessPiece::getIndex() const { return index; }
bool ChessPiece::getWhite() const { return isWhite; }
bool ChessPiece::getKingSide() const { return isKingSide; }

int ChessPiece::getPosX() const             // calculate positions on the fly
{
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( board->getPiece( i, j ) == this )
                return i;
        }
    }
    return -1;
}

int ChessPiece::getPosY() const
{
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( board->getPiece( i, j ) == this )
                return j;
        }
    }
    return -1;
}

const char * ChessPiece::getID() const { return szID; }

bool ChessPiece::move( int x, int y )
{
    if ( canMove( x, y )  &&  board != NULL )   // not sure what happens when *(NULL) used as reference; can't be good.
    {
        delete f_getMoveablePiece( *board, x, y );                          // check pointers !
        f_movePiece( *board, ChessMove( getPosX(), getPosY(), x, y ) );     // and this will overwrite the pointer
        return true;
    } else
        return false;
}

ChessPiece * ChessPiece::f_movePiece( ChessBoard & cb, ChessMove move ) const
{
    return cb.movePiece( move );
}

ChessPiece * ChessPiece::f_getMoveablePiece( ChessBoard & cb, int x, int y ) const
{
    return cb.getMoveablePiece( x, y );
}

ChessPiece * ChessPiece::f_setPiece( ChessBoard & cb, int x, int y, ChessPiece * piece ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return NULL;
    else
    {
        ChessPiece * temp = cb.aapGrid[x][y];
        cb.aapGrid[x][y] = piece;
        return temp;         //function caller's responsibility to prevent memory holes.
    }
}

int ChessPiece::getRootValue()
{
    switch( getType() )
    {
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

double ChessPiece::getValue()
{
    double value = getRootValue();
    
    return value;
}


//////
//PAWN

Pawn::Pawn( bool isW, bool isKS, ChessBoard * b, int i ):
ChessPiece(isW,isKS,b,i)
{   
    szID[1] = 'P';
    hasMoved = false;
    enPassant = false;
}

Pawn::~Pawn() {}

bool Pawn::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )       // check bounds
        return false;
    
    int xNow = getPosX();
    int yNow = getPosY();
    
    if ( y == yNow  &&  board->getPiece( x, y ) == NULL )         // forward move
    {
        if ( hasMoved == false  &&  isWhite == true  &&  x == 3  &&  board->getPiece( 2, y ) == NULL )
            ;                                                       // Would be return true; need to check for check
        else if ( hasMoved == false  &&  isWhite == false  &&  x == 4  &&  board->getPiece( 5, y ) == NULL )
            ;
        else if ( isWhite == true  &&  x == xNow + 1 )
            ;
        else if ( isWhite == false  &&  x == xNow - 1 )
            ;
        else
            return false;
    } else if ( ( y == yNow + 1  ||  y == yNow - 1 )  &&  board->getPiece( x, y ) == NULL ) {     // en passant
        if ( isWhite == true  &&  xNow == 4  &&  board->getPiece( xNow, y ) != NULL  &&
                board->getPiece( xNow, y )->getWhite() == false  &&  board->getPiece( xNow, y )->getType() == PAWN )
        {
            const Pawn * piece = dynamic_cast<const Pawn *>(board->getPiece( xNow, y ));
            if ( piece  &&  piece->getEnPassant() )
                ;
            else
                return false;
        } else if ( isWhite == false  &&  xNow == 3  &&  board->getPiece( xNow, y ) != NULL  &&
                    board->getPiece( xNow, y )->getWhite() == true  &&  board->getPiece( xNow, y )->getType() == PAWN )
        {
            const Pawn * piece = dynamic_cast<const Pawn *>(board->getPiece( xNow, y ));
            if ( piece  &&  piece->getEnPassant() )
                ;
            else
                return false;
        } else
            return false;
    } else if ( ( y == yNow + 1  ||  y == yNow - 1 )  &&  board->getPiece( x, y ) != NULL ) {      // diagonal capture
        if ( isWhite == true  &&  x == xNow + 1  &&  board->getPiece( x, y )->getWhite() == false )
            ;
        else if ( isWhite == false  &&  x == xNow - 1  &&  board->getPiece( x, y )->getWhite() == true )
            ;
        else
            return false;
    } else
        return false;
    
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(board->checkCheck( isWhite ));
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * Pawn::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    int sign = 1;
    if ( !isWhite )
        sign -= 2;
    
    ChessMove * ret = new ChessMove[4 + 1];
    if ( !ret )
    {
        return NULL;
    }
    
    ret[4] = ChessMove::end;
    
    if ( canMove( x + sign, y ) )
        ret[0] = ChessMove( x, y, x + sign, y );
    else
        ret[0] = ChessMove::end;
    
    if ( canMove( x + 2 * sign, y ) )
        ret[1] = ChessMove( x, y, x + 2 * sign, y );
    else
        ret[1] = ChessMove::end;
    
    if ( canMove( x + sign, y + 1 ) )
        ret[2] = ChessMove( x, y, x + sign, y + 1 );
    else
        ret[2] = ChessMove::end;
    
    if ( canMove( x + sign, y - 1 ) )
        ret[3] = ChessMove( x, y, x + sign, y - 1 );
    else
        ret[3] = ChessMove::end;
    
    ChessMove::sort( ret, 5 );          // sort the ::ends to the end
    return ret;
}

bool Pawn::getMoved() const { return hasMoved; }
bool Pawn::getEnPassant() const { return enPassant; }
void Pawn::setEnPassant( bool b ) { enPassant = b; }

bool Pawn::move( int x, int y )
{
    int xPrev = getPosX();
    int yPrev = getPosY();
    
    bool destEmpty = board->getPiece( x, y ) == NULL;
    
    bool b = ChessPiece::move( x, y );
    if ( b )
    {
        hasMoved = true;
        
        if ( isWhite == true  &&  x == xPrev + 1  &&  abs( y - yPrev ) == 1  &&  destEmpty )
        {
            ChessPiece * ePawn = f_getMoveablePiece( *board, xPrev, y );
            delete ePawn;
            f_setPiece( *board, xPrev, y, NULL );
        } else if ( isWhite == false  &&  x == xPrev - 1  &&  abs( y - yPrev ) == 1  &&  destEmpty ) {
            ChessPiece * ePawn = f_getMoveablePiece( *board, xPrev, y );
            delete ePawn;
            f_setPiece( *board, xPrev, y, NULL );
        }
        
        if ( abs( xPrev - x ) == 2 )
            enPassant = true;
    }
    return b;
    
    //promotion to be covered elsewhere
}

PieceType Pawn::getType() const { return PAWN; }

//////
//ROOK

Rook::Rook( bool isW, bool isKS, ChessBoard * b, int i ):
ChessPiece(isW,isKS,b,i)
{
    szID[1] = 'R';
    hasMoved = false;
}

Rook::~Rook() {}

bool Rook::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return false;
    
    int xNow = getPosX();
    int yNow = getPosY();
    
    if ( board->getPiece( x, y ) != NULL  &&  board->getPiece( x, y )->getWhite() == isWhite )  // capturing same-side piece.
        return false;
    
    if ( x == xNow )
    {
        if ( y == yNow )                // null moves don't count
            return false;
        else                            // check space in between
        {
            if ( y < yNow )
            {
                for( int i = y + 1; i < yNow; i++ )
                {
                    if ( board->getPiece( x, i ) != NULL )
                        return false;
                }
                ;
            } else {     // y > yNow
                for( int i = y - 1; i > yNow; i-- )
                {
                    if ( board->getPiece( x, i ) != NULL )
                        return false;
                }
                ;
            }
        }
    } else if ( y != yNow ) {
        return false;
    } else {                // check space in between again
        if ( x < xNow )
        {
            for( int i = x + 1; i < xNow; i++ )
            {
                if ( board->getPiece( i, y ) != NULL )
                    return false;
            }
            ;
        } else {        // x > xNow
            for( int i = x - 1; i > xNow; i-- )
            {
                if ( board->getPiece( i, y ) != NULL )
                    return false;
            }
            ;
        }
    }
    
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(board->checkCheck( isWhite ));
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * Rook::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    ChessMove * ret = new ChessMove[14 + 1];
    if ( !ret )
        return NULL;
    
    for( int i = 0; i < 15; i++ )           // all blank to start
    {
        ret[i] = ChessMove::end;
    }
    
    for( int i = 0, j = 0; i < 8; i++ )     // add move in the same row
    {
        if ( y != i )
        {
            if ( canMove( x, i ) )
                ret[j] = ChessMove( x, y, x, i );
            j++;
        }
    }
    
    for( int i = 0, j = 7; i < 8; i++ )     // add moves in the same column
    {
        if ( x != i )
        {
            if ( canMove( i, y ) )
                ret[j] = ChessMove( x, y, i, y );
            j++;
        }
    }
    
    ChessMove::sort( ret, 15 );
    return ret;
}

bool Rook::getMoved() const { return hasMoved; }

bool Rook::move( int x, int y )
{
    bool b = ChessPiece::move( x, y );
    if ( b )
        markMoved();
    return b;
}

PieceType Rook::getType() const { return ROOK; }

void Rook::markMoved() { hasMoved = true; }

////////
//KNIGHT

const int Knight::xOffsets[8] = { 1,  1, -1, -1, 2,  2, -2, -2 };
const int Knight::yOffsets[8] = { 2, -2,  2, -2, 1, -1,  1, -1 };

Knight::Knight( bool isW, bool isKS, ChessBoard * b, int i ):
ChessPiece(isW,isKS,b,i)
{
    szID[1] = 'N';
}

Knight::~Knight() {}

bool Knight::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return false;
    
    if ( board->getPiece( x, y ) != NULL  &&  board->getPiece( x, y )->getWhite() == isWhite )
        return false;
    
    int xNow = getPosX();
    int yNow = getPosY();
    
    if ( abs( xNow - x ) == 1 )
    {
        if ( abs( yNow - y ) == 2 )
            ;
        else
            return false;
    } else if ( abs( xNow - x ) == 2 ) {
        if ( abs( yNow - y ) == 1 )
            ;
        else
            return false;
    } else
        return false;
        
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(board->checkCheck( isWhite ));
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * Knight::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    ChessMove * ret = new ChessMove[8 + 1];
    if ( !ret )
        return NULL;
    
    for( int i = 0; i < 9; i++ )
    {
        ret[i] = ChessMove::end;
    }
    
    for( int i = 0; i < 8; i++ )
    {
        if ( canMove( x + xOffsets[i], y + yOffsets[i] ) )
            ret[i] = ChessMove( x, y, x + xOffsets[i], y + yOffsets[i] );
    }
    
    ChessMove::sort( ret, 9 );
    return ret;
}

PieceType Knight::getType() const { return KNIGHT; }

////////
//BISHOP

Bishop::Bishop( bool isW, bool isKS, ChessBoard * b, int i ):
ChessPiece(isW,isKS,b,i)
{
    szID[1] = 'B';
}

Bishop::~Bishop() {}

bool Bishop::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return false;
    
    int xNow = getPosX();
    int yNow = getPosY();
    
    int pyInt = yNow - xNow;
    int nyInt = yNow + xNow;
    
    if ( board->getPiece( x, y ) != NULL  &&  board->getPiece( x, y )->getWhite() == isWhite )    // can't capture same side
        return false;
    
    if ( y - x == pyInt )
    {
        if ( x == xNow )
            return false;
        else if ( x > xNow )
        {
            for( int i = 1; ( yNow + i < y  &&  xNow + i < x ); i++ )
            {
                if ( board->getPiece( xNow + i, yNow + i ) != NULL )
                    return false;
            }
            ;
        } else {    // x < xNow
            for( int i = 1; ( yNow - i > y  &&  xNow - i > x ); i++ )
            {
                if ( board->getPiece( xNow - i, yNow - i ) != NULL )
                    return false;
            }
            ;
        }
    } else if ( y + x == nyInt ) {
        if ( x == xNow )
            return false;
        else if ( x > xNow )
        {
            for( int i = 1; ( yNow - i > y  &&  xNow + i < x ); i++ )
            {
                if ( board->getPiece( xNow + i, yNow - i ) != NULL )
                    return false;
            }
            ;
        } else {    // x < xNow
            for( int i = 1; ( yNow + i < y  &&  xNow - i > x ); i++ )
            {
                if ( board->getPiece( xNow - i, yNow + i ) != NULL )
                    return false;
            }
            ;
        }
    } else
        return false;
        
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(board->checkCheck( isWhite ));
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * Bishop::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    ChessMove * ret = new ChessMove[13 + 1];
    if ( !ret )
        return NULL;
    
    for( int i = 0; i < 14; i++ )
    {
        ret[i] = ChessMove::end;
    }
    
    int i = 0;
    for( int j = 0;( x + j < 8  &&  y + j < 8 ); j++ )
    {
        if ( canMove( x + j, y + j ) )
        {
            ret[i] = ChessMove( x, y, x + j, y + j );
            i++;
        }
    }
    
    for( int j = 0;( x - j >= 0  &&  y - j >= 0 ); j++ )
    {
        if ( canMove( x - j, y - j ) )
        {
            ret[i] = ChessMove( x, y, x - j, y - j );
            i++;
        }
    }
    
    for( int j = 0;( x + j < 8  &&  y - j >= 0 ); j++ )
    {
        if ( canMove( x + j, y - j ) )
        {
            ret[i] = ChessMove( x, y, x + j, y - j );
            i++;
        }
    }
    
    for( int j = 0; ( x - j >= 0  &&  y + j < 8 ); j++ )
    {
        if ( canMove( x - j, y + j ) )
        {
            ret[i] = ChessMove( x, y, x - j, y + j );
            i++;
        }
    }
    
    ChessMove::sort( ret, 14 );         // shouldn't be necessary; if so returns quickly.
    return ret;
}

PieceType Bishop::getType() const { return BISHOP; }

//////
//KING

const int King::xOffsets[10] = { -1, -1, -1,  0, 0,  1, 1, 1,  0, 0 };
const int King::yOffsets[10] = { -1,  0,  1, -1, 1, -1, 0, 1, -2, 2 };

King::King( bool isW, ChessBoard * b ):
ChessPiece(isW,true,b,0)
{
    szID[1] = 'K';
    hasMoved = false;
}

King::~King() {}

bool King::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return false;
    
    if ( board->getPiece( x, y ) != NULL  &&  board->getPiece( x, y )->getWhite() == isWhite )  // also disallows null move
        return false;
    
    int xNow = getPosX();
    int yNow = getPosY();
    
    if ( abs( x - xNow ) <= 1  &&  abs( y - yNow ) <= 1 )
    ;
    else
    {
        if ( abs( y - yNow ) == 2  &&  x == xNow  &&  !hasMoved )         // castling
        {
            if ( chkchk  &&  inCheck() )                // prevent infinite loop
                return false;
            
            int sign = ( y - yNow ) / 2;
            
            if ( board->getPiece( xNow, 4 + sign ) == NULL  &&  board->getPiece( xNow, 4 + 2 * sign ) == NULL  &&
                    board->getPiece( xNow, ( 7 + 7 * sign ) / 2 ) != NULL )       // check rooks
            {
	        if ( sign == -1  &&  board->getPiece( xNow, 1 ) != NULL )   // Queen side castle
		    return false;
		
                if ( board->getPiece( xNow, ( 7 + 7 * sign ) / 2 )->getType() == ROOK  &&
                        board->getPiece( xNow, ( 7 + 7 * sign ) / 2 )->getWhite() == isWhite )
                {
                    const Rook * piece = dynamic_cast<const Rook *>(board->getPiece( xNow, ( 7 + 7 * sign ) / 2 ));
                    if ( !piece->getMoved() )
                    {
                        f_movePiece( *board, ChessMove( xNow, 4, xNow, 4 + sign ) );
                        if ( chkchk  &&  inCheck() )
                        {
                            f_movePiece( *board, ChessMove( xNow, 4 + sign, xNow, 4 ) );
                            return false;
                        }
                        
                        f_movePiece( *board, ChessMove( xNow, 4 + sign, xNow, 4 + sign * 2 ) );
                        if ( chkchk  &&  inCheck() )
                        {
                            f_movePiece( *board, ChessMove( xNow, 4 + sign * 2, xNow, 4 ) );
                            return false;
                        }
                        
                        f_movePiece( *board, ChessMove( xNow, 4 + sign * 2, xNow, 4 ) );
                    } else
                        return false;
                } else
                    return false;
            } else
                return false;
        } else
            return false;
    }
    
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(inCheck());
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * King::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    ChessMove * ret = new ChessMove[10 + 1];
    if ( !ret )
        return NULL;
    
    for( int i = 0; i < 11; i++ )
    {
        ret[i] = ChessMove::end;
    }
    
    for( int i = 0; i < 10; i++ )
    {
        if ( canMove( x + xOffsets[i], y + yOffsets[i] ) )
            ret[i] = ChessMove( x, y, x + xOffsets[i], y + yOffsets[i] );
    }
    
    ChessMove::sort( ret, 11 );
    return ret;
}

bool King::move( int x, int y )
{
    int xPrev = getPosX();
    int yPrev = getPosY();
    
    bool b = ChessPiece::move( x, y );
    if ( b )
    {
        hasMoved = true;
        if ( y == 6  &&  yPrev == 4 )
        {
            f_movePiece( *board, ChessMove( xPrev, 7, xPrev, 5 ) );
            Rook * piece = dynamic_cast<Rook *>(f_getMoveablePiece( *board, xPrev, 5 ));
            piece->markMoved();
        }
        if ( y == 2  &&  yPrev == 4 )
        {
            f_movePiece( *board, ChessMove( xPrev, 0, xPrev, 3 ) );
            Rook * piece = dynamic_cast<Rook *>(f_getMoveablePiece( *board, xPrev, 3 ));
            piece->markMoved();
        }
    }
    return b;
}

bool King::inCheck() const
{
    int xNow = getPosX();
    int yNow = getPosY();
    
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( board->getPiece( i, j ) != NULL  &&  board->getPiece( i, j )->getWhite() != isWhite  &&
                    board->getPiece( i, j )->canMove( xNow, yNow, false ) )
                return true;
        }
    }
    return false;
}

PieceType King::getType() const { return KING; }

///////
//QUEEN

Queen::Queen( bool isW, ChessBoard * b, int i, bool isKS ):
ChessPiece(isW,isKS,b,i)
{
    szID[1] = 'Q';
}

Queen::~Queen() {}

bool Queen::canMove( int x, int y, bool chkchk ) const
{
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return false;
    
    if ( board->getPiece( x, y ) != NULL  &&  board->getPiece( x, y )->getWhite() == isWhite )  // also disallows null move
        return false;
        
    int xNow = getPosX();
    int yNow = getPosY();
    
    int pyInt = yNow - xNow;
    int nyInt = yNow + xNow;
    
    if ( x == xNow )
    {
        if ( y == yNow )                // null moves don't count
            return false;
        else                            // check space in between
        {
            if ( y < yNow )
            {
                for( int i = y + 1; i < yNow; i++ )
                {
                    if ( board->getPiece( x, i ) != NULL )
                        return false;
                }
                ;
            } else {     // y > yNow
                for( int i = y - 1; i > yNow; i-- )
                {
                    if ( board->getPiece( x, i ) != NULL )
                        return false;
                }
                ;
            }
        }
    } else if ( y != yNow ) {
        if ( y - x == pyInt )
        {
            if ( x == xNow )
                return false;
            else if ( x > xNow )
            {
                for( int i = 1; ( yNow + i < y  &&  xNow + i < x ); i++ )
                {
                    if ( board->getPiece( xNow + i, yNow + i ) != NULL )
                        return false;
                }
                ;
            } else {    // x < xNow
                for( int i = 1; ( yNow - i > y  &&  xNow - i > x ); i++ )
                {
                    if ( board->getPiece( xNow - i, yNow - i ) != NULL )
                        return false;
                }
                ;
            }
        } else if ( y + x == nyInt ) {
            if ( x == xNow )
                return false;
            else if ( x > xNow )
            {
                for( int i = 1; ( yNow - i > y  &&  xNow + i < x ); i++ )
                {
                    if ( board->getPiece( xNow + i, yNow - i ) != NULL )
                        return false;
                }
                ;
            } else {    // x < xNow
                for( int i = 1; ( yNow + i < y  &&  xNow - i > x ); i++ )
                {
                    if ( board->getPiece( xNow - i, yNow + i ) != NULL )
                        return false;
                }
                ;
            }
        } else
            return false;
    } else {                // check space in between again
        if ( x < xNow )
        {
            for( int i = x + 1; i < xNow; i++ )
            {
                if ( board->getPiece( i, y ) != NULL )
                    return false;
            }
            ;
        } else {        // x > xNow
            for( int i = x - 1; i > xNow; i-- )
            {
                if ( board->getPiece( i, y ) != NULL )
                    return false;
            }
            ;
        }
    }
    
    if ( chkchk )
    {
        ChessPiece * temp = f_movePiece( *board, ChessMove( xNow, yNow, x, y ) );     // these pass by reference
        bool ret = !(board->checkCheck( isWhite ));
        f_movePiece( *board, ChessMove( x, y, xNow, yNow ) );
        f_setPiece( *board, x, y, temp );
        return ret;
    } else
        return true;
}

ChessMove * Queen::getMoves() const
{
    int x = getPosX();
    int y = getPosY();
    
    ChessMove * ret = new ChessMove[27 + 1];
    if ( !ret )
        return NULL;
    
    for( int i = 0; i < 28; i++ )           // all blank to start
    {
        ret[i] = ChessMove::end;
    }
    
    for( int i = 0, j = 0; i < 8; i++ )     // add move in the same row
    {
        if ( y != i )
        {
            if ( canMove( x, i ) )
                ret[j] = ChessMove( x, y, x, i );
            j++;
        }
    }
    
    for( int i = 0, j = 7; i < 8; i++ )     // add moves in the same column
    {
        if ( x != i )
        {
            if ( canMove( i, y ) )
                ret[j] = ChessMove( x, y, i, y );
            j++;
        }
    }
    
    int i = 14;
    for( int j = 0;( x + j < 8  &&  y + j < 8 ); j++ )
    {
        if ( canMove( x + j, y + j ) )
        {
            ret[i] = ChessMove( x, y, x + j, y + j );
            i++;
        }
    }
    
    for( int j = 0;( x - j >= 0  &&  y - j >= 0 ); j++ )
    {
        if ( canMove( x - j, y - j ) )
        {
            ret[i] = ChessMove( x, y, x - j, y - j );
            i++;
        }
    }
    
    for( int j = 0;( x + j < 8  &&  y - j >= 0 ); j++ )
    {
        if ( canMove( x + j, y - j ) )
        {
            ret[i] = ChessMove( x, y, x + j, y - j );
            i++;
        }
    }
    
    for( int j = 0; ( x - j >= 0  &&  y + j < 8 ); j++ )
    {
        if ( canMove( x - j, y + j ) )
        {
            ret[i] = ChessMove( x, y, x - j, y + j );
            i++;
        }
    }
    
    ChessMove::sort( ret, 28 );
    return ret;
}

PieceType Queen::getType() const { return QUEEN; }

////////////
//CHESSBOARD


ChessBoard::ChessBoard():
szForm(NULL)
{
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            aapGrid[i][j] = NULL;
        }
    }
    
    //Set up the pieces
    
    aapGrid[0][0] = new Rook( WHITE, false, this, 0 );
    aapGrid[0][1] = new Knight( WHITE, false, this, 0 );
    aapGrid[0][2] = new Bishop( WHITE, false, this, 0 );
    aapGrid[0][3] = new Queen( WHITE, this, 0 );
    aapGrid[0][4] = new King( WHITE, this );
    aapGrid[0][5] = new Bishop( WHITE, true, this, 0 );
    aapGrid[0][6] = new Knight( WHITE, true, this, 0 );
    aapGrid[0][7] = new Rook( WHITE, true, this, 0 );
    
    aapGrid[1][0] = new Pawn( WHITE, false, this, 4 );
    aapGrid[1][1] = new Pawn( WHITE, false, this, 3 );
    aapGrid[1][2] = new Pawn( WHITE, false, this, 2 );
    aapGrid[1][3] = new Pawn( WHITE, false, this, 1 );
    aapGrid[1][4] = new Pawn( WHITE, true, this, 1 );
    aapGrid[1][5] = new Pawn( WHITE, true, this, 2 );
    aapGrid[1][6] = new Pawn( WHITE, true, this, 3 );
    aapGrid[1][7] = new Pawn( WHITE, true, this, 4 );
    
    aapGrid[6][0] = new Pawn( BLACK, false, this, 4 );
    aapGrid[6][1] = new Pawn( BLACK, false, this, 3 );
    aapGrid[6][2] = new Pawn( BLACK, false, this, 2 );
    aapGrid[6][3] = new Pawn( BLACK, false, this, 1 );
    aapGrid[6][4] = new Pawn( BLACK, true, this, 1 );
    aapGrid[6][5] = new Pawn( BLACK, true, this, 2 );
    aapGrid[6][6] = new Pawn( BLACK, true, this, 3 );
    aapGrid[6][7] = new Pawn( BLACK, true, this, 4 );
    
    aapGrid[7][0] = new Rook( BLACK, false, this, 0 );
    aapGrid[7][1] = new Knight( BLACK, false, this, 0 );
    aapGrid[7][2] = new Bishop( BLACK, false, this, 0 );
    aapGrid[7][3] = new Queen( BLACK, this, 0 );
    aapGrid[7][4] = new King( BLACK, this );
    aapGrid[7][5] = new Bishop( BLACK, true, this, 0 );
    aapGrid[7][6] = new Knight( BLACK, true, this, 0 );
    aapGrid[7][7] = new Rook( BLACK, true, this, 0 );
}

ChessBoard::~ChessBoard()               // do not keep pointers to pieces in this grid after the board is destroyed
{
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( aapGrid[i][j] != NULL )
            {
                delete aapGrid[i][j];
                aapGrid[i][j] = NULL;
            }
        }
    }
    
    delete[] szForm;
    szForm = NULL;
}

const char * ChessBoard::toString()
{
    if ( szForm == NULL )
        szForm = new char[1387];        // 1387 = 42 * 33 + 1
    if ( szForm == NULL )
        return NULL;
    
    bool white = false;
    
    szForm[1386] = '\0';
    
    for( int i = 0; i < 33; i++ )      // 33 rows
    {
        szForm[42 * i + 41] = '\n';      // newlines
        if ( i % 4 == 0 )
        {
            for( int j = 0; j < 41; j++ )       // 41 columns
            {
                if ( j % 5 == 0 )
                    szForm[42 * i + j] = '+';
                else
                {
                    if ( i == 0  ||  i == 32 )
                        szForm[42 * i + j] = ChessMove::szlcLetters[j / 5];
                    else
                        szForm[42 * i + j] = '-';
                }
            }
        } else if ( i % 2 == 0 ) {
            for( int j = 0; j < 41; j++ )
            {
                if ( j % 5 == 0 )
                {
                    if ( j == 0  ||  j == 40 )
                        szForm[42 * i + j] = ChessPiece::szDigits[8 - ( i / 4 )];
                    else
                        szForm[42 * i + j] = '|';
                } else if ( j % 5 == 1  ||  j % 5 == 4 ) {
                    if ( aapGrid[7 - ( i / 4 )][j / 5] != NULL )
                        szForm[42 * i + j] = ' ';
                    else
                    {
                        white = ( i / 4 ) % 2 == 0;
                        if ( ( j / 5 ) % 2 != 0 )
                            white = !white;
                        if ( white )
                            szForm[42 * i + j] = 'X';
                        else
                            szForm[42 * i + j] = ' ';
                    }
                } else {
                    if ( aapGrid[7 - ( i / 4 )][j / 5] != NULL )
                    {
                        if ( j % 5 == 2 )
                            szForm[42 * i + j] = *((aapGrid[7 - ( i / 4 )][j / 5])->getID());
                        else
                            szForm[42 * i + j] = *((aapGrid[7 - ( i / 4 )][j / 5])->getID() + 1);
                    } else {
                        white = ( i / 4 ) % 2 == 0;
                        if ( ( j / 5 ) % 2 != 0 )
                            white = !white;
                        if ( white )
                            szForm[42 * i + j] = 'X';
                        else
                            szForm[42 * i + j] = ' ';
                    }
                }
            }
        } else {
            for( int j = 0; j < 41; j++ )
            {
                if ( j % 5 == 0 )
                {
                    if ( j == 0  ||  j == 40 )
                        szForm[42 * i + j] = ChessPiece::szDigits[8 - ( i / 4 )];
                    else
                        szForm[42 * i + j] = '|';
                } else if ( j % 5 == 1  ||  j % 5 == 4 ) {
                    white = ( i / 4 ) % 2 == 0;
                    if ( ( j / 5 ) % 2 != 0 )
                        white = !white;
                    if ( white )
                        szForm[42 * i + j] = 'X';
                    else
                        szForm[42 * i + j] = ' ';
                } else {
                    if ( aapGrid[7 - ( i / 4)][j / 5] != NULL )
                        szForm[42 * i + j] = ' ';
                    else
                    {
                        white = ( i / 4 ) % 2 == 0;
                        if ( ( j / 5 ) % 2 != 0 )
                            white = !white;
                        if ( white )
                            szForm[42 * i + j] = 'X';
                        else
                            szForm[42 * i + j] = ' ';
                    }
                }
            }
        }
    }
    return szForm;
}

ChessPiece * ChessBoard::getMoveablePiece( int x, int y )
{
    if ( x >= 0  &&  x <= 7  &&  y >= 0  &&  y <= 7 )
        return aapGrid[x][y];
    else
        return NULL;
}

ChessPiece * ChessBoard::movePiece( ChessMove move )
{
    if ( !move.isEnd()  &&  aapGrid[move.getStartX()][move.getStartY()] != NULL )
    {
       ChessPiece * temp = aapGrid[move.getEndX()][move.getEndY()]; 
       aapGrid[move.getEndX()][move.getEndY()] = aapGrid[move.getStartX()][move.getStartY()];   // replaces destination
       aapGrid[move.getStartX()][move.getStartY()] = NULL;                                      // leaves behind nothing
       return temp;
    } else
        return NULL;
}

const ChessPiece * ChessBoard::getPiece( int x, int y ) const
{
    if ( x >= 0  &&  x <= 7  &&  y >= 0  &&  y <= 7 )
        return aapGrid[x][y];
    else
        return NULL;
}

bool ChessBoard::checkCheck( bool isW ) const
{
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( aapGrid[i][j] != NULL  &&  (aapGrid[i][j])->getType() == KING  &&  (aapGrid[i][j])->getWhite() == isW )
            {
                const King * piece = dynamic_cast<const King *>(getPiece( i, j ));
                if ( piece  &&  piece->inCheck() )
                    return true;
                else if ( piece )
                    return false;
                else
                    return true;        // will never happen if constructors work right. The king is fake...? Error, essentially.
            }
        }
    }
    return true;        // then game is over; no moves should be made
}

///////////
//CHESSGAME

ChessGame::ChessGame():
rulesOn(true), whiteTurn(true), board(ChessBoard())
{}

void ChessGame::setRules( bool on ) { rulesOn = on; }
bool ChessGame::getRules() const { return rulesOn; }

ChessGame::~ChessGame() {}

bool ChessGame::checkmate( bool white ) const
{
    if ( !board.checkCheck( white ) )
        return false;
    
    ChessMove * moves = getMoves( white );
    if ( moves  &&  moves->isEnd() )
    {
        delete[] moves;
        return true;
    } else {
        delete[] moves;
        return false;
    }
}

bool ChessGame::stalemate( bool turn ) const
{
    if ( board.checkCheck( turn ) )
        return false;
    
    ChessMove * moves = getMoves( turn );
    if ( moves  &&  moves->isEnd() )
    {
        delete[] moves;
        return true;
    } else {
        delete[] moves;
        return false;
    }
}

ChessMove * ChessGame::getMoves( bool white ) const
{
    int num = 0;
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( board.getPiece( i, j ) != NULL  &&  board.getPiece( i, j )->getWhite() == white )
                num++;
        }
    }
    
    if ( num == 0 )
        return NULL;
    
    ChessMove ** moveSets = new ChessMove* [num];
    for( int i = 0, k = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            if ( board.getPiece( i, j ) != NULL  &&  board.getPiece( i, j )->getWhite() == white )
            {
                moveSets[k] = board.getPiece( i, j )->getMoves();
                k++;
            }
        }
    }
    
    int numMoves = 0;
    for( int i = 0; i < num; i++ )
    {
        numMoves += ChessMove::length( moveSets[i] );
    }
    
    if ( numMoves == 0 )
    {
        ChessMove * ret = new ChessMove[1];
        ret[0] = ChessMove::end;
        return ret;
    }
    
    ChessMove * ret = new ChessMove[numMoves + 1];
    ret[numMoves] = ChessMove::end;
    
    ChessMove::copy( ret, moveSets[0] );
    
    for( int i = 1; i < num; i++ )
    {
        ChessMove::concat( ret, moveSets[i] );
    }
    
    for( int i = 0; i < num; i++ )
    {
        delete[] moveSets[i];
        moveSets[i] = NULL;
    }
    delete[] moveSets;
    moveSets = NULL;
    
    return ret;
}

bool ChessGame::getTurn() const { return whiteTurn; }

bool ChessGame::makeMove( const ChessMove & cm )
{
    if ( rulesOn )
    {
        ChessPiece * piece = board.getMoveablePiece( cm.getStartX(), cm.getStartY() );
        if ( piece == NULL  ||  piece->getWhite() != whiteTurn )
            return false;
        bool b = piece->move( cm.getEndX(), cm.getEndY() );
        if ( b )
        {
            whiteTurn = !whiteTurn;
            for( int i = 0; i < 8; i++ )                // remove en passants that no longer apply
            {
                for( int j = 0; j < 8; j++ )
                {
                    if ( board.getPiece( i, j ) != NULL  &&  board.getPiece( i, j )->getWhite() == whiteTurn )
                    {
                        ChessPiece * piece = board.getMoveablePiece( i, j );
                        if ( piece->getType() == PAWN )
                        {
                            Pawn * pawn = dynamic_cast<Pawn *>(piece);
                            pawn->setEnPassant( false );
                        }
                    }
                }
            }
        }
        return b;
    } else {
        ChessPiece * piece = board.movePiece( cm );
        delete piece;
        piece = NULL;
        return true;
    }
}

const char * ChessGame::getBoard() { return board.toString(); }

const ChessPiece * ChessGame::getPiece( int x, int y ) const { return board.getPiece( x, y ); }

void ChessGame::setPiece( int x, int y, ChessPiece * piece )
{
    if ( rulesOn )
        return;
    
    if ( x < 0  ||  x > 7  ||  y < 0  ||  y > 7 )
        return;
    
    delete board.aapGrid[x][y];
    board.aapGrid[x][y] = piece;
}

ChessBoard * ChessGame::getPieceBoard() { return &board; }

