#pragma once

#include <cstdint>
#include "position.h"
#include "move.h"
#include <vector>

#define u64 uint64_t

class MoveGen {
public:
    void Pawn(const Position& position, Color color, std::vector<Move>& moves);
    void Knight(const Position& position, Color color, std::vector<Move>& moves);
    void Bishop(const Position& position, Color color, std::vector<Move>& moves);
    void Queen(const Position& position, Color color, std::vector<Move>& moves);
    void Rook(const Position& position, Color color, std::vector<Move>& moves);
    void King(const Position& position, Color color, std::vector<Move>& moves);
};