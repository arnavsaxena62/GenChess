#include "movegen.h"
#include "move.h"
#include "utils.h"
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

        u64 target = RankFiletoBB(futureRank, futureFile);

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
        squares = occupancyBBtoInts(position.whitePawn);
        direction = 1;
        startingRank = 1;
    } else {
        squares = occupancyBBtoInts(position.blackPawn);
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
            !(position.occupied() & RankFiletoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});

            if (rank == startingRank) {

                int futureRank2 = rank + 2 * direction;

                if (!(position.occupied() & RankFiletoBB(futureRank2, futureFile))) {

                    PsuedoLegalmoves.emplace_back(Move{square, futureRank2 * 8 + futureFile});
                }
            }
        }

        // Capture toward file + 1
        futureRank = rank + direction;
        futureFile = file + 1;

        if (futureRank >= 0 && futureRank < 8 && futureFile < 8 &&
            (enemy & RankFiletoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});
        }

        // Capture toward file - 1
        futureFile = file - 1;

        if (futureRank >= 0 && futureRank < 8 && futureFile >= 0 &&
            (enemy & RankFiletoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});
        }
    }
}

void MoveGen::GenPsuedoKnight(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? occupancyBBtoInts(position.whiteKnight) : occupancyBBtoInts(position.blackKnight));

    static const int KnightOffsets[8][2] = {{2, 1}, {2, -1}, {1, -2}, {-1, -2},
                                            {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}};

    for (auto square : squares) {
        int rank = square / 8;
        int file = square % 8;

        for (auto &offset : KnightOffsets) {
            int futureRank = rank + offset[0];
            int futureFile = file + offset[1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & RankFiletoBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{square, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares =
        (color == WHITE ? occupancyBBtoInts(position.whiteKing) : occupancyBBtoInts(position.blackKing));

    static const int KingOffsets[8][2] = {{1, 1},  {1, 0},  {1, -1}, {0, 1},
                                          {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

    for (auto square : squares) {
        int rank = square / 8;
        int file = square % 8;

        for (auto &offset : KingOffsets) {
            int futureRank = rank + offset[0];
            int futureFile = file + offset[1];
            if (futureFile >= 0 && futureFile < 8 && futureRank >= 0 && futureRank < 8 &&
                !(position.own(color) & RankFiletoBB(futureRank, futureFile))) {
                PsuedoLegalmoves.emplace_back(Move{square, 8 * futureRank + futureFile});
            }
        }
    }
}

void MoveGen::GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    vector<int> squares;

    if (color == WHITE) {
        squares = occupancyBBtoInts(position.whiteBishop);
    } else {
        squares = occupancyBBtoInts(position.blackBishop);
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
        squares = occupancyBBtoInts(position.whiteQueen);
    } else {
        squares = occupancyBBtoInts(position.blackQueen);
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
        squares = occupancyBBtoInts(position.whiteRook);
    } else {
        squares = occupancyBBtoInts(position.blackRook);
    }

    for (auto &&square : squares) {
        GenRay(position, color, square, 1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 0, PsuedoLegalmoves);
        GenRay(position, color, square, 0, 1, PsuedoLegalmoves);
        GenRay(position, color, square, 0, -1, PsuedoLegalmoves);
    }
}

bool MoveGen::CheckPromotion(Position &position, Color color) {
    constexpr u64 RANK_8 = 0xFF00000000000000ULL;
    constexpr u64 RANK_1 = 0x00000000000000FFULL;

    if (color == WHITE) {
        return (position.whitePawn & RANK_8) != 0;
    } else {
        return (position.blackPawn & RANK_1) != 0;
    }
}
bool MoveGen::CheckCheck(Position &position, Color color) {
    Color attacker = (color == WHITE) ? BLACK : WHITE;
    vector<Move> pseudoAttacks;
    GeneratePseudoMoves(position, attacker, pseudoAttacks);

    u64 kingBB = (color == WHITE) ? position.whiteKing : position.blackKing;
    vector<int> kingSquares = occupancyBBtoInts(kingBB);
    if (kingSquares.empty())
        return true;

    int kingSq = kingSquares[0];
    for (const auto &m : pseudoAttacks) {
        if (m.to == kingSq)
            return true;
    }
    return false;
}

void MoveGen::ValidateMoves(Position &position, vector<Move> &moves, Color color) {
    vector<Move> validMoves;

    for (const auto &move : moves) {
        Position cloned(position);

        cloned.makeMove(move);

        if (!CheckCheck(cloned, color)) {
            validMoves.push_back(move);
        }
    }

    moves = validMoves;
}