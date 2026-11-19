#include "bot.h"
#include "move.h"
#include "movegen.h"
#include "position.h"
#include "evaluation.h"
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

int main() {
    Bot b1;
    Bot b2;

    auto start = high_resolution_clock::now();
    for (int j = 0; j < 1; j++) {
        Move bestmove;
        pair<Move, int> result;
        Position starting(true);
        for (int i = 0; i < 100; i++) {
            result = b1.findBestMove(starting, 14, WHITE);
            bestmove = result.first;
            starting.makeMove(bestmove);

            result = b2.findBestMove(starting, 14, BLACK);
            starting.makeMove(result.first);
        }
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds\n";
}