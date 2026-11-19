#pragma once

#include "include/position.h"

struct evaluationWeights {
    double material;
    double pawnStructure;
    double mobility;
    double kingSafety;
    double controlledSquares;
};

double materialScore(const Position& position);
double pawnStructureScore(const Position& position);
double mobilityScore(Position& position, u64 whiteControlled, u64 blackControlled);
double kingSafetyScore(Position& position, u64 whiteControlled, u64 blackControlled);
double controlledSquaresScore(u64 whiteControlled, u64 blackControlled);
double evaluate(Position& position, const evaluationWeights& weights, Color side);