#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>

#include "evaluation.h"
#include "move.h"
#include "movegen.h"
#include "position.h"

using namespace std;
const int INF = 1000000000;

constexpr int NUM_EVAL_TERMS = 5;

class Bot {
  public:
    array<double, NUM_EVAL_TERMS> weights{};
    MoveGen handler; // stateless, but only built once now instead of once per node

    Bot() {
        for (auto &w : weights) {
            w = random() % 5;
        }
    }

    Bot(vector<double> inpweights) {
        for (int i = 0; i < NUM_EVAL_TERMS; i++) {
            weights[i] = inpweights[i];
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

int won(const vector<double> &vb1, const vector<double> &vb2) {
    Bot b1(vb1);
    Bot b2(vb2);

    Move bestmove;
    pair<Move, int> result;
    Position starting(true);

    for (int i = 0; i < 30; i++) {
        try {
            result = b1.findBestMove(starting, 3, WHITE);
            starting.makeMove(result.first);
        } catch (const std::exception &) {
            return 0;
        }

        try {
            result = b2.findBestMove(starting, 3, BLACK);
            starting.makeMove(result.first);
        } catch (const std::exception &) {
            return 1;
        }
    }

    return 2;
}

vector<tuple<int, int, int>> batchMatches(const vector<vector<double>> &bots) {
    vector<tuple<int, int, int>> results;

    const size_t botCount = bots.size();
    const size_t totalGames = botCount * (botCount - 1) / 2;
    size_t completedGames = 0;

    results.reserve(totalGames);

    for (size_t i = 0; i < botCount; ++i) {
        for (size_t j = i + 1; j < botCount; ++j) {
            int result = won(bots[i], bots[j]);

            results.emplace_back(static_cast<int>(i), static_cast<int>(j), result);

            ++completedGames;

            cout << "\rGames completed: " << completedGames << " / " << totalGames
                      << std::flush;
        }
    }

    cout << '\n';

    return results;
}

vector<tuple<int, int, int>> batchMatchesDefined(const vector<vector<double>> &bots,
                                                 const vector<tuple<int, int>> matches) {

    vector<tuple<int, int, int>> result;
    int completedGames = 0;
    int totalGames = matches.size();
    int gameResult;
    for (auto &&i : matches) {
        gameResult = won(bots[get<0>(i)], bots[get<1>(i)]);
        result.emplace_back(get<0>(i), get<1>(i), gameResult);
        completedGames++;
        cout << "\rGames completed: " << completedGames << " / " << totalGames << flush;
    }
    cout << endl;
    return result;
}