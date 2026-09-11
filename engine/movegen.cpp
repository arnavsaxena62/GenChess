#include "movegen.h"
#include <vector>
using namespace std;
#define u64 uint64_t


void MoveGen::GenRay(Position &position, Color color, int square, int rankDirection,
                     int fileDirection, vector<Move> &moves) {

    int rank = square / 8;
    int file = square % 8;

    int futureRank = rank + rankDirection;
    int futureFile = file + fileDirection;

    while (futureRank >= 0 && futureRank < 8 && futureFile >= 0 && futureFile < 8) {

        u64 target = SqtoBB(futureRank, futureFile);

        if (target & position.own(color))
            break;

        moves.emplace_back(Move{square, futureRank * 8 + futureFile});

        if (target & position.own(color == WHITE ? BLACK : WHITE))
            break;

        futureRank += rankDirection;
        futureFile += fileDirection;
    }
}

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

    u64 enemy = position.own(color == WHITE ? BLACK : WHITE);

    for (auto square : squares) {
        int rank = square / 8;
        int file = square % 8;

        int futureRank = rank + direction;
        int futureFile = file;

        if (futureRank >= 0 && futureRank < 8 &&
            !(position.occupied() & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});

            if (rank == startingRank) {

                int futureRank2 = rank + 2 * direction;

                if (!(position.occupied() & SqtoBB(futureRank2, futureFile))) {

                    PsuedoLegalmoves.emplace_back(Move{square, futureRank2 * 8 + futureFile});
                }
            }
        }

        // Capture toward file + 1
        futureRank = rank + direction;
        futureFile = file + 1;

        if (futureRank >= 0 && futureRank < 8 && futureFile < 8 &&
            (enemy & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});
        }

        // Capture toward file - 1
        futureFile = file - 1;

        if (futureRank >= 0 && futureRank < 8 && futureFile >= 0 &&
            (enemy & SqtoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});
        }
    }
}

void MoveGen::GenPsuedoKnight(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? BBtoSq(position.whiteKnight) : BBtoSq(position.blackKnight));

    static const int KnightOffsets[8][2] = {{2, 1}, {2, -1}, {1, -2}, {-1, -2},
                                            {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}};

    for (auto square : squares) {
        int rank = square / 8;
        int file = square % 8;

        for (auto &offset : KnightOffsets) {
            int futureRank = rank + offset[0];
            int futureFile = file + offset[1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & SqtoBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{square, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? BBtoSq(position.whiteKing) : BBtoSq(position.blackKing));

    static const int KingOffsets[8][2] = {{1, 1},  {1, 0},  {1, -1}, {0, 1},
                                          {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

    for (auto square : squares) {
        int rank = square / 8;
        int file = square % 8;

        for (auto &offset : KingOffsets) {
            int futureRank = rank + offset[0];
            int futureFile = file + offset[1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & SqtoBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{square, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares;

    if (color == WHITE) {
        squares = BBtoSq(position.whiteBishop);
    } else {
        squares = BBtoSq(position.blackBishop);
    }

    for (auto &&square : squares) {
        GenRay(position, color, square, 1, 1, PsuedoLegalmoves);
        GenRay(position, color, square, 1, -1, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 1, PsuedoLegalmoves);
        GenRay(position, color, square, -1, -1, PsuedoLegalmoves);
    }
}

void MoveGen::GenPsuedoQueen(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares;

    if (color == WHITE) {
        squares = BBtoSq(position.whiteQueen);
    } else {
        squares = BBtoSq(position.blackQueen);
    }

    for (auto &&square : squares) {
        GenRay(position, color, square, 1, 1, PsuedoLegalmoves);
        GenRay(position, color, square, 1, -1, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 1, PsuedoLegalmoves);
        GenRay(position, color, square, -1, -1, PsuedoLegalmoves);
        GenRay(position, color, square, 1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, 0, 1, PsuedoLegalmoves);
        GenRay(position, color, square, 0, -1, PsuedoLegalmoves);
    }
}

void MoveGen::GenPsuedoRook(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares;

    if (color == WHITE) {
        squares = BBtoSq(position.whiteRook);
    } else {
        squares = BBtoSq(position.blackRook);
    }

    for (auto &&square : squares) {
        GenRay(position, color, square, 1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, 0, 1, PsuedoLegalmoves);
        GenRay(position, color, square, 0, -1, PsuedoLegalmoves);
    }
}