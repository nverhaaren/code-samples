// This is the Main.cpp  file which holds the main() funcion.

#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include <string.h>
#include <time.h>

void printDelete( char * sz );
void printMoves( bool color, const ChessGame & game );
void printMoveList( const ChessMove * paMoves );
ChessMove parseAlgMove( const char * szMove, const ChessBoard & board );
int stdMoves( const char * szMove, const ChessBoard & board );

#ifndef _STRING_H
bool strcmp( const char * sz1, const char * sz2 )
{
    // If STRING_H include removed, this method must be modified
    if ( sz1[0] == '\0'  &&  sz2[0] == '\0' )
        return true;
    else if ( sz1[0] == '\0'  ||  sz2[0] == '\0' )
        return false;
    
    for( int i = 0; sz1[i] != '\0'  &&  sz2[i] != '\0'; i++ )
        if ( sz1[i] != sz2[i] )
            return false;
    return true;
}

int strlen( const char * sz )
{
    int l;
    for( l = 0; sz[l] != '\0'  &&  l < 128; l++ );
    return l;
}
#endif

int main( int argc, char * argv[] )
{
    srand( time( NULL ) );          // initialize random number generator
    bool print = true;
    ChessGame game = ChessGame();
    char szInput[128];
    szInput[0] = '\0';
    
    int blackProms = 0;             // promotions
    int whiteProms = 0;
    
    printf("Chess Version 1.0\n\n");
    printf("Commands:\nx#-x#\t\tmove\nend\t\texit\nmoves\t\tshow moves\nmoves x#\tshow moves at x#\n");
    printf("rand\t\tmake a random move\n\n");
    
    while( true )
    {
        if ( print )
        {
            printf( "%s\n\n", game.getBoard() );
            if ( game.checkmate( game.getTurn() ) )
            {
                printf( "Checkmate! " );
                if ( game.getTurn() )
                    printf( "Black" );
                else
                    printf( "White" );
                printf( " wins!\n" );
                system( "PAUSE" );
                return 0;
            } else if ( game.stalemate( game.getTurn() ) ) {
                printf( "Game Over! Stalemate.\n" );
                system( "PAUSE" );
                return 0;
            } else {
                printf( "Enter move for " );
                if ( game.getTurn() )
                    printf( "white:\n" );
                else
                    printf( "black:\n" );
                print = false;
            }
        }
        szInput[0] = szInput[1] = szInput[2] = szInput[3] = szInput[4] = szInput[5] = '\0';
        fgets( szInput, 128, stdin );
        
        szInput[strlen( szInput ) - 1] = '\0';
        
        if ( strcmp( szInput, "end" ) == 0 )
        {
            if ( game.getTurn() )
                printf( "\nWhite resigns.\n\n" );
            else
                printf( "\nBlack resigns.\n\n" );
            
            system( "PAUSE" );
            return 0;
        }
        
        if ( strcmp( szInput, "moves" ) == 0 )
        {
            printMoves( game.getTurn(), game );
            printf( "\n" );
        } else if ( strncmp( szInput, "moves ", 6 ) == 0 ) {
            int x = szInput[7] - '1';
            int y = szInput[6] - 'a';
            if ( game.getPiece( x, y ) != NULL )
            {
                ChessMove * paMoves = game.getPiece( x, y )->getMoves();
                printMoveList( paMoves );
                printf( "\n" );
                delete[] paMoves;
            }
        } else if ( strcmp( szInput, "rand" ) == 0 ) {
            ChessMove * paMoves = game.getMoves( game.getTurn() );
            int l = ChessMove::length( paMoves );
            int move;
            if ( l != 0 )
            {
                move = rand() % l;
                game.makeMove( paMoves[move] );
                print = true;
                if ( ( paMoves[move].getEndX() == 0  ||  paMoves[move].getEndX() == 7 ) &&          // pawn promotion
                       game.getPiece( paMoves[move].getEndX(), paMoves[move].getEndY() )->getType() == PAWN )
                {
                    ChessPiece * piece = NULL;
                    bool KingSide = false;
                    if ( paMoves[move].getEndY() > 3 )
                        KingSide = true;
                    
                    int index = 0;
                    if ( game.getTurn() )
                    {
                        index = blackProms;
                        blackProms++;
                    } else {
                        index = whiteProms;
                        whiteProms++;
                    }
                    
                    piece = new Queen( !game.getTurn(), game.getPieceBoard(), index, KingSide );
                    game.setRules( false );
                    game.setPiece( paMoves[move].getEndX(), paMoves[move].getEndY(), piece );
                    game.setRules( true );
                }
            }
            delete[] paMoves;
        } else {
            ChessMove move = ChessMove( szInput );
            if ( game.makeMove( move ) )
            {
                print = true;
                if ( ( move.getEndX() == 0  ||  move.getEndX() == 7 ) &&  game.getPiece( move.getEndX(), move.getEndY() )->getType() == PAWN )
                {
                    char pieceType = '\0';
                    ChessPiece * piece = NULL;
                    bool KingSide = false;
                    if ( move.getEndY() > 3 )
                        KingSide = true;
                    
                    int index = 0;
                    if ( game.getTurn() )
                    {
                        index = blackProms;
                        blackProms++;
                    } else {
                        index = whiteProms;
                        whiteProms++;
                    }
                    
                    printf( "You have moved a pawn to the end of the board!\n" );
                    printf( "What do you want to promote it to? (Q, R, N, B)\n" );
                    scanf( "%c", &pieceType );
                    switch( pieceType )
                    {
                        case 'R':
                        case 'r':
                        {
                            piece = new Rook( !game.getTurn(), KingSide, game.getPieceBoard(), index );
                            break;
                        }
                        case 'N':
                        case 'n':
                        {
                            piece = new Knight( !game.getTurn(), KingSide, game.getPieceBoard(), index );
                            break;
                        }
                        case 'B':
                        case 'b':
                        {
                            piece = new Bishop( !game.getTurn(), KingSide, game.getPieceBoard(), index );
                            break;
                        }
                        case 'Q':
                        case 'q':
                        default:
                        {
                            piece = new Queen( !game.getTurn(), game.getPieceBoard(), index, KingSide );
                            break;
                        }
                    }
                    game.setRules( false );
                    game.setPiece( move.getEndX(), move.getEndY(), piece );
                    game.setRules( true );
                }
            }
        }
    }
    
    return 0;
}

void printDelete( char * sz )
{
    printf( "%s", sz );
    delete[] sz;
}

void printMoves( bool color, const ChessGame & game )
{
    ChessMove * moves = game.getMoves( color );
    printMoveList( moves );
    delete[] moves;
}

void printMoveList( const ChessMove * paMoves )
{
    int l = ChessMove::length( paMoves );
    for( int i = 0; i < l; i++ )
    {
        printf( "%s\n", paMoves[i].toString() );
    }
}

ChessMove parseAlgMove( const char * szMove, const ChessGame & game )
{
    char c = szMove[0];
    
    bool checkNoted = false;
    bool checkmateNoted = false;
    bool epNoted = false;
    
    bool turn = game.getTurn();
    
    int xDest = 0;
    int yDest = 0;
    
    int moves = 0;
    
    if ( strcmp( szMove, "O-O" )  ||  strcmp( szMove, "0-0" ) )     // kingside castle
    {
        if ( turn == WHITE )
        {
            if ( game.getPiece( 0, 4 )->getType() == KING  &&  game.getPiece( 0, 4 )->canMove( 0, 6 ) )
                return ChessMove( 0, 4, 0, 6 );
            else
            {
                printf( "Illegal move.\n" );
                return ChessMove::end;
            }
        } else {
            if ( game.getPiece( 7, 4 )->getType() == KING  &&  game.getPiece( 7, 4 )->canMove( 7, 6 ) )
                return ChessMove( 7, 4, 7, 6 );
            else
            {
                printf( "Illegal move.\n" );
                return ChessMove::end;
            }
        }
    } else if ( strcmp( szMove, "O-O-O" )  ||  strcmp( szMove, "0-0-0" ) ) {            // Queenside castling
        if ( turn == WHITE )
        {
            if ( game.getPiece( 0, 4 )->getType() == KING  &&  game.getPiece( 0, 4 )->canMove( 0, 2 ) )
                return ChessMove( 0, 4, 0, 2 );
            else
            {
                printf( "Illegal move.\n" );
                return ChessMove::end;
            }
        } else {
            if ( game.getPiece( 7, 4 )->getType() == KING  &&  game.getPiece( 7, 4 )->canMove( 7, 2 ) )
                return ChessMove( 7, 4, 7, 2 );
            else
            {
                printf( "Illegal move.\n" );
                return ChessMove::end;
            }
        }
    } else if ( strlen( szMove ) >= 2 ) {
        if ( strlen( szMove ) == 2 )            // a pawn move
        {
            if ( szMove[0] - 'a' >= 0 )
            {
                if ( szMove[1] - 'a' >= 0 )         // capture by files
                {
                    moves;                          // TODO
                } else {                            // forward move
                    xDest = szMove[1] - '1';
                    yDest = szMove[0] - 'a';
                    
                }
            } else {
                printf( "Illegal move.\n" );
                return ChessMove::end;
            }
        }
    } else
        return ChessMove::end;
}
