#pragma once

#include "move.h"
#include "position.h"
#include "utils.h"
#include <cstdint>
#include <vector>

using namespace std;
#define u64 uint64_t

class MoveGen {
  public:
    void GenPsuedoPawn(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoKnight(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoQueen(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoRook(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves);

    // ownBB/enemyBB are passed in precomputed so a piece with 4-8 rays
    // (bishop/rook/queen) only pays for own()/occupied() once per piece
    // instead of once per ray direction.
    void GenRay(Position &position, Color color, int square, int rankDirection, int fileDirection,
                u64 ownBB, u64 enemyBB, vector<Move> &moves);
    bool RayHitsPiece(Position &position, int rank, int file, int rankDirection, int fileDirection,
                      u64 targetPieces, u64 occupied);

    bool IsSquareAttacked(Position &position, int square, Color attacker);
    bool CheckCheck(Position &position, Color color);
    bool CheckPromotion(Position &position, Color color);

    // Bitboard of every square, of 'color', that is pinned to its own king
    // by an enemy slider. Used by ValidateMoves to skip the expensive
    // clone+re-check-legality path for moves that can't possibly be illegal.
    u64 ComputePinnedPieces(Position &position, Color color);

    void ValidateMoves(Position &position, vector<Move> &moves, Color color);

    vector<Move> GenerateMoves(Position &position, Color color) {
        vector<Move> result = {};
        result.reserve(48); // average branching factor ~35; avoids reallocation churn

        GenPsuedoBishop(position, color, result);
        GenPsuedoKing(position, color, result);
        GenPsuedoQueen(position, color, result);
        GenPsuedoRook(position, color, result);
        GenPsuedoPawn(position, color, result);
        GenPsuedoKnight(position, color, result);
        ValidateMoves(position, result, color);

        return result;
    }

  private:
    void GeneratePseudoMoves(Position &position, Color color, vector<Move> &moves) {
        moves.clear();
        moves.reserve(48);
        GenPsuedoBishop(position, color, moves);
        GenPsuedoKing(position, color, moves);
        GenPsuedoQueen(position, color, moves);
        GenPsuedoRook(position, color, moves);
        GenPsuedoPawn(position, color, moves);
        GenPsuedoKnight(position, color, moves);
    }
};