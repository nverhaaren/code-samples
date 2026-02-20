// This is the Main.cpp  file which holds the main() funcion.

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include "chess.h"

void printMoves(bool color, const ChessGame& game);
void printMoveList(const std::vector<ChessMove>& moves);
// TODO: complete algebraic notation parsing (Phase 6)
// ChessMove parseAlgMove( const char * szMove, const ChessBoard & board );
// int stdMoves( const char * szMove, const ChessBoard & board );

int main(int argc, char* argv[]) {
    srand(time(nullptr));  // initialize random number generator
    bool print = true;
    ChessGame game = ChessGame();
    std::string input;

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
                auto moves = game.getPiece(x, y)->getMoves();
                printMoveList(moves);
                printf("\n");
            }
        } else if (input == "rand") {
            auto moves = game.getMoves(game.getTurn());
            int l = (int)moves.size();
            if (l != 0) {
                int move = rand() % l;
                game.makeMove(moves[move]);
                print = true;
            }
        } else {
            ChessMove move = ChessMove(input.c_str());
            if (game.makeMove(move)) {
                print = true;
            }
        }
    }

    return 0;
}

void printMoves(bool color, const ChessGame& game) {
    auto moves = game.getMoves(color);
    printMoveList(moves);
}

void printMoveList(const std::vector<ChessMove>& moves) {
    for (const auto& m : moves) {
        printf("%s\n", m.toString());
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
