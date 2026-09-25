#include "utils.h"


vector<int> occupancyBBtoInts(u64 bb) {
    vector<int> squares;
    for (u64 i = 1ULL; i != 0; i <<= 1) {
        if (bb & i) {
            squares.emplace_back(__builtin_ctzll(i));
        }
    }
    return squares;
}

u64 RankFiletoBB(int rank, int file) { return 1ULL << (rank * 8 + file); }

u64 InttoBB(int sq) {
    return 1ULL << sq;
}

vector<int> InttoRankFile(int sq){
    int rank = sq / 8;
    int file = sq % 8;
    return vector<int>{rank, file};
}