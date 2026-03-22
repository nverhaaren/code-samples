// This is the Main.cpp  file which holds the main() funcion.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include "bridge.h"
#include "chess.h"

void printMoves(bool color, const ChessGame& game);
void printMoveList(const std::vector<ChessMove>& moves);

int main(int argc, char* argv[]) {
    // Check for --json-bridge flag
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--json-bridge") == 0) {
            runBridgeLoop();
            return 0;
        }
    }

    srand(time(nullptr));  // initialize random number generator
    bool print = true;
    ChessGame game = ChessGame();
    std::string input;

    printf("Chess Version 1.0\n\n");
    printf("Commands:\nNf3, e4, O-O\tSAN move\nx#-x#\t\tLAN move\nend\t\texit\n");
    printf("moves\t\tshow moves\nmoves x#\tshow moves at x#\nrand\t\trandom move\n\n");

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
            // Try SAN first (e.g., "e4", "Nf3", "O-O", "exd5", "e8=Q").
            // Falls back to LAN (e.g., "e2e4") if SAN doesn't match.
            ChessMove move = game.parseSan(input);
            if (move.isEnd()) {
                // SAN didn't match — try LAN. For LAN promotion, prompt the user.
                move = ChessMove(input.c_str());
                const ChessPiece* piece = game.getPiece(move.getStartX(), move.getStartY());
                if (piece != nullptr && piece->getType() == PAWN) {
                    int endRank = move.getEndX();
                    bool isPromoRank =
                        (piece->getWhite() == WHITE) ? endRank == 7 : endRank == 0;
                    if (isPromoRank) {
                        printf("Promote to? (q=queen, r=rook, b=bishop, n=knight): ");
                        std::string promo;
                        if (std::getline(std::cin, promo)) {
                            PieceType pt = QUEEN;
                            if (promo == "r") pt = ROOK;
                            else if (promo == "b") pt = BISHOP;
                            else if (promo == "n") pt = KNIGHT;
                            move = ChessMove(move.getStartX(), move.getStartY(), endRank,
                                             move.getEndY(), pt);
                        }
                    }
                }
            }
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
