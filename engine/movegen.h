#pragma once

#include "move.h"
#include "position.h"
#include "utils.cpp"
#include <cstdint>
#include <vector>

#define u64 uint64_t

class MoveGen {
  public:
    void GenPsuedoPawn(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoKnight(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoBishop(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoQueen(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoRook(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenPsuedoKing(Position &position, Color color, vector<Move> &PsuedoLegalmoves);
    void GenRay(Position &position, Color color, int square, int rankDirection, int fileDirection, vector<Move> &moves);

    void CheckCheck(Position &position, Color color);
    void CheckPromotion(Position &position, Color color);
    void ValidateMoves(vector<Move> &moves, Color color);
    
    vector<Move> GenerateMoves(Position &position, Color color){
      vector<Move> result = {};

      GenPsuedoBishop(position, color, result);
      GenPsuedoKing(position, color, result);
      GenPsuedoQueen(position, color, result);
      GenPsuedoRook(position, color, result);
      GenPsuedoPawn(position, color, result);
      GenPsuedoKnight(position, color, result);
      ValidateMoves(result, color);
    }
};