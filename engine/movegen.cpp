#include "movegen.h"
#include <vector>
using namespace std;
#define u64 uint64_t

void MoveGen::GenPsuedoPawn(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {
    if (color == WHITE) {
        for (int i = A2; i <= 64; i << 2) {
            u64 freeSquares = ~position.occupied();
        }
    } else if (color == BLACK) {
    }
}

void MoveGen::GenPsuedoKnight(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {
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
                !(position.occupied() & squareBB(futureRank, futureFile))) {
                PsuedoLegalMoves.emplace_back(Move{i, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoKing(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {
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
                !(position.occupied() & squareBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{i, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoBishop(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {}

void MoveGen::GenPsuedoQueen(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {}

void MoveGen::GenPsuedoRook(Position &position, Color color, std::vector<Move> &PsuedoLegalmoves) {}