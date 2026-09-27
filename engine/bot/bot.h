#include <iostream>
#include <vector>

#include "move.h"
#include "movegen.h"
#include "position.h"
#include <random>
using namespace std;
const int INF = 1000000000;

vector<int> EvalFunc(Position &pos, Color color) {
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
    // return vector<int> {color == WHITE ? score : -score, moves};

    return vector<int>{random()%5,random()%5,random()%5,random()%5};
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

    pair<Move, int> findBestMove(Position pos, int depth, Color side) {
        int alpha = -INF;
        int beta = INF;

        Move bestMove{};
        int nodes;

        MoveGen handler;
        for (Move move : handler.GenerateMoves(pos, side)) {
            Position next = pos;
            next.makeMove(move);

            int score = -search(next, depth - 1, opposite(side), -beta, -alpha, nodes);

            if (score > alpha) {
                alpha = score;
                bestMove = move;
            }
        }

        return pair<Move, int>{bestMove, nodes};
    }

    int search(Position pos, int depth, Color side, int alpha, int beta, int nodes) {
        if (depth == 0)
            return evaluationWithWeights(pos, side);

        int best = -INF;

        nodes++;

        MoveGen handler;
        vector<Move> moves = handler.GenerateMoves(pos, side);

        for (Move move : moves) {
            Position next = pos;
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
