#include <iostream>
#include <vector>

#include "move.h"
#include "movegen.h"
#include "position.h"
using namespace std;

vector<int> EvalFunc(Position* pos, Color color){
}

class Bot {
  public:
    vector<int> weights;

    Move makeMove(Position *pos, Color color) {
        int depth = 1;

        Position starting = Position(true);
        MoveGen handler;

        auto moves = handler.GenerateMoves(starting, WHITE);
        vector<pair<Move, int>> depth1;
        for (auto &&i : moves) {
            Position clone = starting;
            clone.makeMove(i);
            int eval= 0;
            auto evalIndices = EvalFunc(pos, color);
            for (int i = 0; i< evalIndices.size(); i++)
            {
                eval+=weights[i] * evalIndices[i];
            }
            depth1.emplace_back(pair(i, eval));
        }
    }
};

int main() {}