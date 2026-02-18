// This is the Main.cpp  file which holds the main() funcion.

#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include <string.h>
#include <time.h>

void printDelete( char * sz );
void printMoves( bool color, const ChessGame & game );
void printMoveList( const ChessMove * paMoves );
// TODO: complete algebraic notation parsing (Phase 6)
// ChessMove parseAlgMove( const char * szMove, const ChessBoard & board );
// int stdMoves( const char * szMove, const ChessBoard & board );

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

// TODO (Phase 6): implement full algebraic notation parsing.
// The function below is incomplete and currently unused. Commented out to
// suppress compiler warnings until the implementation is ready.
//
// ChessMove parseAlgMove( const char * szMove, const ChessGame & game )
// {
//     ... (incomplete stub — O-O/O-O-O castling partially handled,
//          pawn moves and piece moves not implemented)
// }
