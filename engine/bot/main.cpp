#include "bot.h"
#include "move.h"
#include "movegen.h"
#include "position.h"
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

int main() {
    Bot b1;
    Bot b2;

    Position starting = Position(true);
    cout << "---------------" << endl;
    starting.display();
    auto start = high_resolution_clock::now();
    int i = 0;
    while (i < 50) {
        starting.makeMove(b1.findBestMove(starting, 3, WHITE));
        i++;
        // cout << "---------------" << endl;
        // starting.display();
        starting.makeMove(b2.findBestMove(starting, 3, BLACK));
        // cout << "---------------" << endl;
        i++;
        // starting.display();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken: " << duration.count() << " microseconds";
    starting.display();
}