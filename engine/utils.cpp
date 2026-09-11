#pragma once

#include "position.h"
#include <vector>
using namespace std;

vector<int> BBtoSq(u64 bb) {
    vector<int> squares;
    for (u64 i = 1ULL; i != 0; i <<= 1) {
        if (bb & i) {
            squares.emplace_back(__builtin_ctzll(i));
        }
    }
}

u64 SqtoBB(int rank, int file) { return 1ULL << (rank * 8 + file); }