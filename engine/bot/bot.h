#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>

#include "move.h"
#include "movegen.h"
#include "position.h"

using namespace std;
const int INF = 1000000000;

constexpr int NUM_EVAL_TERMS = 4;

array<int, NUM_EVAL_TERMS> EvalFunc(Position &pos, Color color) {
    // int whiteScore =
    //     1 * __builtin_popcountll(pos.whitePawn) + 3 * __builtin_popcountll(pos.whiteKnight) +
    //     3 * __builtin_popcountll(pos.whiteBishop) + 5 * __builtin_popcountll(pos.whiteRook) +
    //     9 * __builtin_popcountll(pos.whiteQueen);

    // int blackScore =
    //     1 * __builtin_popcountll(pos.blackPawn) + 3 * __builtin_popcountll(pos.blackKnight) +
    //     3 * __builtin_popcountll(pos.blackBishop) + 5 * __builtin_popcountll(pos.blackRook) +
    //     9 * __builtin_popcountll(pos.blackQueen);

    // int score = whiteScore - blackScore;

    // MoveGen mg;
    // int moves = mg.GenerateMoves(pos, color).size();
    // return array<int, NUM_EVAL_TERMS> {color == WHITE ? score : -score, moves};

    return array<int, NUM_EVAL_TERMS>{int(random() % 5), int(random() % 5), int(random() % 5), int(random() % 5)};
}

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
        auto EvalIndices = EvalFunc(pos, color);
        int FinalEval = 0;
        for (int i = 0; i < NUM_EVAL_TERMS; i++) {
            FinalEval += EvalIndices[i] * weights[i];
        }

        return FinalEval;
    }

    // pos taken by reference: the caller already made a Position copy (`next`)
    // to hand off the moved-into position, so copying it *again* on the way
    // into search()/findBestMove() was a second, unnecessary full-position
    // copy at every single node.
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