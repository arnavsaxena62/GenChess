#include "movegen.h"
#include "move.h"
#include "utils.h"
#include <array>
#include <vector>
using namespace std;
#define u64 uint64_t

namespace {

// Precomputed once at static-init time. Turns every knight/king move-gen
// call and every knight/king attack check into a single table lookup
// instead of an 8-iteration offset loop with bounds checks.
const array<u64, 64> knightAttacks = [] {
    array<u64, 64> table{};
    static const int KnightOffsets[8][2] = {{2, 1}, {2, -1}, {1, -2}, {-1, -2},
                                            {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}};
    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8, file = sq % 8;
        u64 bb = 0;
        for (auto &o : KnightOffsets) {
            int r = rank + o[0], f = file + o[1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8)
                bb |= RankFiletoBB(r, f);
        }
        table[sq] = bb;
    }
    return table;
}();

const array<u64, 64> kingAttacks = [] {
    array<u64, 64> table{};
    static const int KingOffsets[8][2] = {{1, 1},  {1, 0},  {1, -1}, {0, 1},
                                          {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8, file = sq % 8;
        u64 bb = 0;
        for (auto &o : KingOffsets) {
            int r = rank + o[0], f = file + o[1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8)
                bb |= RankFiletoBB(r, f);
        }
        table[sq] = bb;
    }
    return table;
}();

} // namespace

void MoveGen::GenRay(Position &position, Color color, int square, int rankDirection,
                     int fileDirection, u64 ownBB, u64 enemyBB, vector<Move> &moves) {

    int rank = square / 8;
    int file = square % 8;

    int futureRank = rank + rankDirection;
    int futureFile = file + fileDirection;

    while (futureRank >= 0 && futureRank < 8 && futureFile >= 0 && futureFile < 8) {

        u64 target = RankFiletoBB(futureRank, futureFile);

        if (target & ownBB)
            break;

        moves.emplace_back(Move{square, futureRank * 8 + futureFile});

        if (target & enemyBB)
            break;

        futureRank += rankDirection;
        futureFile += fileDirection;
    }
}

void MoveGen::GenPsuedoPawn(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    u64 pawns;
    int direction;
    int startingRank;

    if (color == WHITE) {
        pawns = position.whitePawn;
        direction = 1;
        startingRank = 1;
    } else {
        pawns = position.blackPawn;
        direction = -1;
        startingRank = 6;
    }

    u64 enemy = position.own(color == WHITE ? BLACK : WHITE);
    u64 occ = position.occupied(); // position is untouched during this loop; safe to cache once

    while (pawns) {
        int square = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        int rank = square / 8;
        int file = square % 8;

        int futureRank = rank + direction;
        int futureFile = file;

        if (futureRank >= 0 && futureRank < 8 &&
            !(occ & RankFiletoBB(futureRank, futureFile))) {

            PsuedoLegalmoves.emplace_back(Move{square, futureRank * 8 + futureFile});

            if (rank == startingRank) {

                int futureRank2 = rank + 2 * direction;

                if (!(occ & RankFiletoBB(futureRank2, futureFile))) {

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
    u64 knights = (color == WHITE) ? position.whiteKnight : position.blackKnight;
    u64 ownBB = position.own(color);

    while (knights) {
        int square = __builtin_ctzll(knights);
        knights &= knights - 1;

        u64 attacks = knightAttacks[square] & ~ownBB;
        while (attacks) {
            int to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PsuedoLegalmoves.emplace_back(Move{square, to});
        }
    }
}

void MoveGen::GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    u64 king = (color == WHITE) ? position.whiteKing : position.blackKing;
    u64 ownBB = position.own(color);

    while (king) {
        int square = __builtin_ctzll(king);
        king &= king - 1;

        u64 attacks = kingAttacks[square] & ~ownBB;
        while (attacks) {
            int to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PsuedoLegalmoves.emplace_back(Move{square, to});
        }
    }
}

void MoveGen::GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    u64 bishops = (color == WHITE) ? position.whiteBishop : position.blackBishop;
    u64 ownBB = position.own(color);
    u64 enemyBB = position.own(color == WHITE ? BLACK : WHITE);

    while (bishops) {
        int square = __builtin_ctzll(bishops);
        bishops &= bishops - 1;

        GenRay(position, color, square, 1, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 1, -1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, -1, ownBB, enemyBB, PsuedoLegalmoves);
    }
}

void MoveGen::GenPsuedoQueen(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    u64 queens = (color == WHITE) ? position.whiteQueen : position.blackQueen;
    u64 ownBB = position.own(color);
    u64 enemyBB = position.own(color == WHITE ? BLACK : WHITE);

    while (queens) {
        int square = __builtin_ctzll(queens);
        queens &= queens - 1;

        GenRay(position, color, square, 1, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 1, -1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, -1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 1, 0, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 0, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 0, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 0, -1, ownBB, enemyBB, PsuedoLegalmoves);
    }
}

void MoveGen::GenPsuedoRook(Position &position, Color color, vector<Move> &PsuedoLegalmoves) {
    u64 rooks = (color == WHITE) ? position.whiteRook : position.blackRook;
    u64 ownBB = position.own(color);
    u64 enemyBB = position.own(color == WHITE ? BLACK : WHITE);

    while (rooks) {
        int square = __builtin_ctzll(rooks);
        rooks &= rooks - 1;

        GenRay(position, color, square, 1, 0, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, -1, 0, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 0, 1, ownBB, enemyBB, PsuedoLegalmoves);
        GenRay(position, color, square, 0, -1, ownBB, enemyBB, PsuedoLegalmoves);
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

    // Knights / king: O(1) table lookup instead of an 8-offset loop.
    u64 enemyKnight = (attacker == WHITE) ? position.whiteKnight : position.blackKnight;
    if (knightAttacks[square] & enemyKnight)
        return true;

    u64 enemyKing = (attacker == WHITE) ? position.whiteKing : position.blackKing;
    if (kingAttacks[square] & enemyKing)
        return true;

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
    u64 occupied = position.occupied();

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

    if (!kingBB)
        return true;

    int kingSquare = __builtin_ctzll(kingBB);
    return IsSquareAttacked(position, kingSquare, attacker);
}

u64 MoveGen::ComputePinnedPieces(Position &position, Color color) {
    u64 kingBB = (color == WHITE) ? position.whiteKing : position.blackKing;
    if (!kingBB)
        return 0;

    int kingSquare = __builtin_ctzll(kingBB);
    int kingRank = kingSquare / 8;
    int kingFile = kingSquare % 8;

    u64 ownBB = position.own(color);
    u64 occ = position.occupied();

    u64 enemyBishopQueen = (color == WHITE) ? (position.blackBishop | position.blackQueen)
                                             : (position.whiteBishop | position.whiteQueen);
    u64 enemyRookQueen = (color == WHITE) ? (position.blackRook | position.blackQueen)
                                           : (position.whiteRook | position.whiteQueen);

    u64 pinned = 0;

    static const int DiagDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    static const int StraightDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    auto scan = [&](int dr, int df, u64 relevantSliders) {
        int r = kingRank + dr;
        int f = kingFile + df;
        int firstOwnSquare = -1;

        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            u64 sq = RankFiletoBB(r, f);

            if (sq & ownBB) {
                if (firstOwnSquare == -1) {
                    firstOwnSquare = r * 8 + f;
                } else {
                    return; // a second own piece blocks any pin along this ray
                }
            } else if (sq & occ) {
                // first non-own piece hit is an enemy piece
                if (firstOwnSquare != -1 && (sq & relevantSliders)) {
                    pinned |= InttoBB(firstOwnSquare);
                }
                return;
            }

            r += dr;
            f += df;
        }
    };

    for (auto &d : DiagDirs)
        scan(d[0], d[1], enemyBishopQueen);
    for (auto &d : StraightDirs)
        scan(d[0], d[1], enemyRookQueen);

    return pinned;
}

void MoveGen::ValidateMoves(Position &position, vector<Move> &moves, Color color) {
    vector<Move> validMoves;
    validMoves.reserve(moves.size());

    u64 kingBB = (color == WHITE) ? position.whiteKing : position.blackKing;
    int kingSquare = kingBB ? __builtin_ctzll(kingBB) : -1;

    bool inCheck = CheckCheck(position, color);
    u64 pinned = ComputePinnedPieces(position, color);

    // Without pins and without being in check, a non-king move can never
    // expose its own king to check (this codebase has no en passant, which
    // is the other classic way a "safe-looking" move can expose check).
    // So we only pay for the expensive clone+re-check path when the move
    // could plausibly be illegal: king moves, pinned-piece moves, or any
    // move at all while already in check.
    for (const auto &move : moves) {
        bool mustFullyValidate =
            inCheck || move.from == kingSquare || (pinned & InttoBB(move.from));

        if (!mustFullyValidate) {
            validMoves.push_back(move);
            continue;
        }

        Position cloned(position);
        cloned.makeMove(move);

        if (!CheckCheck(cloned, color)) {
            validMoves.push_back(move);
        }
    }

    moves = std::move(validMoves);
}