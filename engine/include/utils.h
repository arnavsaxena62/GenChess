#pragma once
#include <cstdint>
#include <vector>
#include "move.h"

using namespace std;

#define u64 uint64_t

enum Color : int { WHITE, BLACK };

/**
 * ts returns array of occupied bits as int
 */
vector<int> occupancyBBtoInts(u64 bb);

/**
 * ts turns Int to rank and file
 */
vector<int> InttoRankFile(int sq);

// The functions below are `inline` and defined here in the header (not in
// utils.cpp) on purpose: they're one-line bit-shifts called on the hottest
// path in move generation (hundreds of millions of calls in a real search).
// A function defined in a different .cpp file can't be inlined across that
// translation-unit boundary without LTO, so calling these from movegen.cpp/
// position.h as out-of-line functions was paying real call/return overhead
// for what should just be a single shift instruction. Defining them here
// lets every caller inline them directly.

/**
 * ts turns Rank File to a BB with a single 1 bit
 */
inline u64 RankFiletoBB(int rank, int file) { return 1ULL << (rank * 8 + file); }

/**
 * ts turns Int to a single bit BB
 */
inline u64 InttoBB(int sq) { return 1ULL << sq; }

inline Color opposite(Color color) { return color == WHITE ? BLACK : WHITE; }
