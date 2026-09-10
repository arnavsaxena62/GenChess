#pragma once

#include <cstdint>
#include "position.h"
#include "move.h"
#include <vector>

#define u64 uint64_t

class MoveGen {
public:
    void Pawn(Position& position, Color color, std::vector<Move>& moves);
    void Knight(Position& position, Color color, std::vector<Move>& moves);
    void Bishop(Position& position, Color color, std::vector<Move>& moves);
    void Queen(Position& position, Color color, std::vector<Move>& moves);
    void Rook(Position& position, Color color, std::vector<Move>& moves);
    void King(Position& position, Color color, std::vector<Move>& moves);
};