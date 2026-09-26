#include <iostream>
#include <vector>

#include "move.h"
#include "movegen.h"
#include "position.h"
#include <random>
using namespace std;
const int INF = 1000000000;

vector<int> EvalFunc(Position &pos, Color color) {
    return vector<int>{random() % 5, random() % 5, random() % 5, random() % 5};
}

class Bot {
  public:
    vector<int> weights = {};

    Bot() {
        for (int i = 0; i < 4; i++) {
            weights.emplace_back(random() % 5);
        }
    }

    int evaluationWithWeights(Position &pos, Color color) {
        auto EvalIndices = EvalFunc(pos, color);
        int FinalEval = 0;
        for (int i = 0; i < EvalIndices.size(); i++) {
            FinalEval = FinalEval + EvalIndices[i] * weights[i];
        }

        return FinalEval;
    }

    Move findBestMove(Position pos, int depth, Color side) {
        int alpha = -INF;
        int beta = INF;

        Move bestMove{};

        MoveGen handler;
        for (Move move : handler.GenerateMoves(pos, side)) {
            Position next = pos;
            next.makeMove(move);

            int score = -search(next, depth - 1, opposite(side), -beta, -alpha);

            if (score > alpha) {
                alpha = score;
                bestMove = move;
            }
        }

        return bestMove;
    }

    int search(Position pos, int depth, Color side, int alpha, int beta) {
        if (depth == 0)
            return evaluationWithWeights(pos, side);

        int best = -INF;

        MoveGen handler;
        vector<Move> moves = handler.GenerateMoves(pos, side);

        for (Move move : moves) {
            Position next = pos;
            next.makeMove(move);

            int score = -search(next, depth - 1, opposite(side), -beta, -alpha);

            best = max(best, score);
            alpha = max(alpha, score);

            // Beta cutoff
            if (alpha >= beta)
                break;
        }

        return best;
    }
};
