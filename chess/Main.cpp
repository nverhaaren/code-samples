// This is the Main.cpp  file which holds the main() funcion.

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "chess.h"

void printDelete(char* sz);
void printMoves(bool color, const ChessGame& game);
void printMoveList(const ChessMove* moves);
// TODO: complete algebraic notation parsing (Phase 6)
// ChessMove parseAlgMove( const char * szMove, const ChessBoard & board );
// int stdMoves( const char * szMove, const ChessBoard & board );

int main(int argc, char* argv[]) {
    srand(time(nullptr));  // initialize random number generator
    bool print = true;
    ChessGame game = ChessGame();
    std::string input;

    int blackProms = 0;  // promotions
    int whiteProms = 0;

    printf("Chess Version 1.0\n\n");
    printf(
        "Commands:\nx#-x#\t\tmove\nend\t\texit\nmoves\t\tshow moves\nmoves x#\tshow moves at x#\n");
    printf("rand\t\tmake a random move\n\n");

    while (true) {
        if (print) {
            printf("%s\n\n", game.getBoard());
            if (game.checkmate(game.getTurn())) {
                printf("Checkmate! ");
                if (game.getTurn())
                    printf("Black");
                else
                    printf("White");
                printf(" wins!\n");
                return 0;
            } else if (game.stalemate(game.getTurn())) {
                printf("Game Over! Stalemate.\n");
                return 0;
            } else {
                printf("Enter move for ");
                if (game.getTurn())
                    printf("white:\n");
                else
                    printf("black:\n");
                print = false;
            }
        }
        if (!std::getline(std::cin, input)) break;

        if (input == "end") {
            if (game.getTurn())
                printf("\nWhite resigns.\n\n");
            else
                printf("\nBlack resigns.\n\n");

            return 0;
        }

        if (input == "moves") {
            printMoves(game.getTurn(), game);
            printf("\n");
        } else if (input.substr(0, 6) == "moves ") {
            int x = input[7] - '1';
            int y = input[6] - 'a';
            if (game.getPiece(x, y) != nullptr) {
                ChessMove* moves = game.getPiece(x, y)->getMoves();
                printMoveList(moves);
                printf("\n");
                delete[] moves;
            }
        } else if (input == "rand") {
            ChessMove* moves = game.getMoves(game.getTurn());
            int l = ChessMove::length(moves);
            int move;
            if (l != 0) {
                move = rand() % l;
                game.makeMove(moves[move]);
                print = true;
                if ((moves[move].getEndX() == 0 || moves[move].getEndX() == 7) &&  // pawn promotion
                    game.getPiece(moves[move].getEndX(), moves[move].getEndY())->getType() ==
                        PAWN) {
                    ChessPiece* piece = nullptr;
                    bool kingSide = false;
                    if (moves[move].getEndY() > 3) kingSide = true;

                    int index = 0;
                    if (game.getTurn()) {
                        index = blackProms;
                        blackProms++;
                    } else {
                        index = whiteProms;
                        whiteProms++;
                    }

                    piece = new Queen(!game.getTurn(), game.getPieceBoard(), index, kingSide);
                    game.setRules(false);
                    game.setPiece(moves[move].getEndX(), moves[move].getEndY(), piece);
                    game.setRules(true);
                }
            }
            delete[] moves;
        } else {
            ChessMove move = ChessMove(input.c_str());
            if (game.makeMove(move)) {
                print = true;
                if ((move.getEndX() == 0 || move.getEndX() == 7) &&
                    game.getPiece(move.getEndX(), move.getEndY())->getType() == PAWN) {
                    char pieceType = '\0';
                    ChessPiece* piece = nullptr;
                    bool kingSide = false;
                    if (move.getEndY() > 3) kingSide = true;

                    int index = 0;
                    if (game.getTurn()) {
                        index = blackProms;
                        blackProms++;
                    } else {
                        index = whiteProms;
                        whiteProms++;
                    }

                    printf("You have moved a pawn to the end of the board!\n");
                    printf("What do you want to promote it to? (Q, R, N, B)\n");
                    scanf("%c", &pieceType);
                    switch (pieceType) {
                        case 'R':
                        case 'r': {
                            piece =
                                new Rook(!game.getTurn(), kingSide, game.getPieceBoard(), index);
                            break;
                        }
                        case 'N':
                        case 'n': {
                            piece =
                                new Knight(!game.getTurn(), kingSide, game.getPieceBoard(), index);
                            break;
                        }
                        case 'B':
                        case 'b': {
                            piece =
                                new Bishop(!game.getTurn(), kingSide, game.getPieceBoard(), index);
                            break;
                        }
                        case 'Q':
                        case 'q':
                        default: {
                            piece =
                                new Queen(!game.getTurn(), game.getPieceBoard(), index, kingSide);
                            break;
                        }
                    }
                    game.setRules(false);
                    game.setPiece(move.getEndX(), move.getEndY(), piece);
                    game.setRules(true);
                }
            }
        }
    }

    return 0;
}

void printDelete(char* sz) {
    printf("%s", sz);
    delete[] sz;
}

void printMoves(bool color, const ChessGame& game) {
    ChessMove* moves = game.getMoves(color);
    printMoveList(moves);
    delete[] moves;
}

void printMoveList(const ChessMove* moves) {
    int l = ChessMove::length(moves);
    for (int i = 0; i < l; i++) {
        printf("%s\n", moves[i].toString());
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
