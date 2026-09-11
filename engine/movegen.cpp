#include "movegen.h"
#include <vector>
using namespace std;
#define u64 uint64_t

void MoveGen::GenPsuedoPawn(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares;

    int direction;
    int startingRank;

    if (color == WHITE) {
        squares = BBtoSq(position.whitePawn);
        direction = 1;
        startingRank = 1;
    } else {
        squares = BBtoSq(position.blackPawn);
        direction = -1;
        startingRank = 6;
    }

    for (auto i : squares) {
        int rank = i % 8;
        int file = i / 8;

        int futureRank = rank;
        int futureFile = file + direction;

        if (futureFile >= 0 && futureFile < 8 &&
            !(position.occupied() & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{i, futureFile * 8 + futureRank});

            if (file == startingRank) {

                futureFile = file + 2 * direction;

                if (!(position.occupied() & SqtoBB(futureRank, futureFile))) {

                    PsuedoLegalmoves.emplace_back(Move{i, futureFile * 8 + futureRank});
                }
            }
        }


        u64 enemy = position.own(color == WHITE ? BLACK : WHITE);

        // Capture toward rank + 1
        futureRank = rank + 1;
        futureFile = file + direction;

        if (futureRank < 8 && futureFile >= 0 && futureFile < 8 &&
            (enemy & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{i, futureFile * 8 + futureRank});
        }

        // Capture toward rank - 1
        futureRank = rank - 1;

        if (futureRank >= 0 && futureFile >= 0 && futureFile < 8 &&
            (enemy & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{i, futureFile * 8 + futureRank});
        }
    }
}

void MoveGen::GenPsuedoKnight(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? BBtoSq(position.whiteKnight) : BBtoSq(position.blackKnight));

    int Knightmoves[8][2] = {{2, 1}, {2, -1}, {1, -2}, {-1, -2},
                             {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}};

    vector<Move> PsuedoLegalMoves = {};

    for (auto &&i : squares) {
        int rank = i % 8;
        int file = i / 8;

        vector<tuple<int, int>> movesAvailable = {};

        for (int i = 0; i < 8; i++) {
            int futureRank = rank - Knightmoves[i][0];
            int futureFile = file - Knightmoves[i][1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & SqtoBB(futureRank, futureFile))) {
                PsuedoLegalMoves.emplace_back(Move{i, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? BBtoSq(position.whiteKing) : BBtoSq(position.blackKing));

    int Knightmoves[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

    for (auto &&i : squares) {
        int rank = i % 8;
        int file = i / 8;

        for (int i = 0; i < 8; i++) {
            int futureRank = rank - Knightmoves[i][0];
            int futureFile = file - Knightmoves[i][1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & SqtoBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{i, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {}

void MoveGen::GenPsuedoQueen(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {}

void MoveGen::GenPsuedoRook(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {}