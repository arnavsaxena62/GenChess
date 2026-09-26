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

u64 RankFiletoBB(int rank, int file) { return 1ULL << (rank * 8 + file); }

u64 InttoBB(int sq) { return 1ULL << sq; }

Color opposite(Color color) {
    if (color == WHITE) {
        return BLACK;
    }
    if (color == BLACK) {
        return WHITE;
    }
}

vector<int> InttoRankFile(int sq) {
    int rank = sq / 8;
    int file = sq % 8;
    return vector<int>{rank, file};
}