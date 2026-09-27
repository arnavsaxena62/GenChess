#include "utils.h"

vector<int> occupancyBBtoInts(u64 bb) {
    vector<int> squares;

    while (bb) {
        int sq = __builtin_ctzll(bb);
        squares.emplace_back(sq);
        bb &= bb - 1;
    }

    return squares;
}

vector<int> InttoRankFile(int sq) {
    int rank = sq / 8;
    int file = sq % 8;
    return vector<int>{rank, file};
}
