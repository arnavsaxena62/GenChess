#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>

#include "move.h"
#include "movegen.h"
#include "position.h"
#include "evaluation.h"

using namespace std;
const int INF = 1000000000;

constexpr int NUM_EVAL_TERMS = 5;

class Bot {
  public:
    array<int, NUM_EVAL_TERMS> weights{};
    MoveGen handler; // stateless, but only built once now instead of once per node

    Bot() {
        for (auto &w : weights) {
            w = random() % 5;
        }
    }

    int evaluationWithWeights(Position &pos, Color color) {
        evaluationWeights evalWeights{
            static_cast<double>(weights[0]), // material
            static_cast<double>(weights[1]), // pawn structure
            static_cast<double>(weights[2]), // mobility
            static_cast<double>(weights[3]), // king safety
            static_cast<double>(weights[4])  // controlled squares
        };

        return static_cast<int>(evaluate(pos, evalWeights, color));
    }

    // pos taken by reference and mutated in place via makeMoveWithUndo/
    // unmakeMove — no Position clone at all now, at any depth.
    pair<Move, int> findBestMove(Position &pos, int depth, Color side) {
        int alpha = -INF;
        int beta = INF;

        Move bestMove{};
        long long nodes = 0; // was uninitialized before — real bug, not just unused

        for (Move move : handler.GenerateMoves(pos, side)) {
            Position next(pos);
            next.makeMove(move);

            int score = -search(next, depth - 1, opposite(side), -beta, -alpha, nodes);

            if (score > alpha) {
                alpha = score;
                bestMove = move;
            }
        }

        return pair<Move, int>{bestMove, (int)nodes};
    }

    // nodes taken by reference: previously passed by value, so nodes++ only
    // ever incremented a copy that was discarded when the call returned —
    // the count you got back from findBestMove was meaningless.
    int search(Position &pos, int depth, Color side, int alpha, int beta, long long &nodes) {
        if (depth == 0)
            return evaluationWithWeights(pos, side);

        int best = -INF;

        nodes++;

        vector<Move> moves = handler.GenerateMoves(pos, side);

        for (Move move : moves) {
            Position next(pos);
            next.makeMove(move);

            int score = -search(next, depth - 1, opposite(side), -beta, -alpha, nodes);

            best = max(best, score);
            alpha = max(alpha, score);

            // Beta cutoff
            if (alpha >= beta)
                break;
        }

        return best;
    }
};
