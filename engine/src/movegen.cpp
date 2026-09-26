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
    vector<int> squares = (color == WHITE ? occupancyBBtoInts(position.whiteKnight)
                                          : occupancyBBtoInts(position.blackKnight));

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
    vector<int> squares = (color == WHITE ? occupancyBBtoInts(position.whiteKing)
                                          : occupancyBBtoInts(position.blackKing));

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
// bool MoveGen::CheckCheck(Position &position, Color color) {
//     Color attacker = (color == WHITE) ? BLACK : WHITE;
//     vector<Move> pseudoAttacks;
//     GeneratePseudoMoves(position, attacker, pseudoAttacks);

//     u64 kingBB = (color == WHITE) ? position.whiteKing : position.blackKing;
//     vector<int> kingSquares = occupancyBBtoInts(kingBB);
//     if (kingSquares.empty())
//         return true;

//     int kingSq = kingSquares[0];
//     for (const auto &m : pseudoAttacks) {
//         if (m.to == kingSq)
//             return true;
//     }
//     return false;
// }

bool MoveGen::RayHitsPiece(Position &position, int rank, int file, int rankDirection,
                           int fileDirection, u64 targetPieces, u64 occupied) {
    int futureRank = rank + rankDirection;
    int futureFile = file + fileDirection;

    while (futureRank >= 0 && futureRank < 8 && futureFile >= 0 && futureFile < 8) {
        u64 target = RankFiletoBB(futureRank, futureFile);

        if (occupied & target) {
            return (target & targetPieces) != 0;
        }

        futureRank += rankDirection;
        futureFile += fileDirection;
    }
    return false;
}

bool MoveGen::IsSquareAttacked(Position &position, int square, Color attacker) {
    int rank = square / 8;
    int file = square % 8;

    // Knights
    static const int KnightOffsets[8][2] = {{2, 1}, {2, -1}, {1, -2}, {-1, -2},
                                            {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}};
    u64 enemyKnight = (attacker == WHITE) ? position.whiteKnight : position.blackKnight;
    for (auto &offset : KnightOffsets) {
        int r = rank + offset[0];
        int f = file + offset[1];
        if (r >= 0 && r < 8 && f >= 0 && f < 8 && (enemyKnight & RankFiletoBB(r, f)))
            return true;
    }

    // King (needed so kings can't move next to each other, also harmless here)
    static const int KingOffsets[8][2] = {{1, 1},  {1, 0},  {1, -1}, {0, 1},
                                          {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
    u64 enemyKing = (attacker == WHITE) ? position.whiteKing : position.blackKing;
    for (auto &offset : KingOffsets) {
        int r = rank + offset[0];
        int f = file + offset[1];
        if (r >= 0 && r < 8 && f >= 0 && f < 8 && (enemyKing & RankFiletoBB(r, f)))
            return true;
    }

    // Pawns: look at the diagonal squares behind us, from attacker's perspective
    u64 enemyPawn = (attacker == WHITE) ? position.whitePawn : position.blackPawn;
    int pawnRank = (attacker == WHITE) ? rank - 1 : rank + 1;
    if (pawnRank >= 0 && pawnRank < 8) {
        if (file - 1 >= 0 && (enemyPawn & RankFiletoBB(pawnRank, file - 1)))
            return true;
        if (file + 1 < 8 && (enemyPawn & RankFiletoBB(pawnRank, file + 1)))
            return true;
    }

    // Sliders
    u64 enemyBishopQueen = (attacker == WHITE) ? (position.whiteBishop | position.whiteQueen)
                                               : (position.blackBishop | position.blackQueen);
    u64 enemyRookQueen = (attacker == WHITE) ? (position.whiteRook | position.whiteQueen)
                                             : (position.blackRook | position.blackQueen);

    static const int DiagDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    static const int StraightDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    auto occupied = position.occupied();

    for (auto &dir : DiagDirs)
        if (RayHitsPiece(position, rank, file, dir[0], dir[1], enemyBishopQueen, occupied))
            return true;

    for (auto &dir : StraightDirs)
        if (RayHitsPiece(position, rank, file, dir[0], dir[1], enemyRookQueen, occupied))
            return true;

    return false;
}

bool MoveGen::CheckCheck(Position &position, Color color) {
    Color attacker = (color == WHITE) ? BLACK : WHITE;
    u64 kingBB = (color == WHITE) ? position.whiteKing : position.blackKing;
    vector<int> kingSquares = occupancyBBtoInts(kingBB);

    if (kingSquares.empty())
        return true;

    return IsSquareAttacked(position, kingSquares[0], attacker);
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