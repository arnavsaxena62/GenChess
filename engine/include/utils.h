#pragma once
#include <cstdint>
#include <vector>
#include "move.h"

using namespace std;

#define u64 uint64_t

enum Color : int { WHITE, BLACK };

Color opposite(Color color);

/**
 * ts returns array of occupied bits as int
 */
vector<int> occupancyBBtoInts(u64 bb);

/**
 * ts turns Rank File to a BB with a single 1 bit
 */
u64 RankFiletoBB(int rank, int file);


/**
 * ts turns Int to a single bit BB
 */
u64 InttoBB(int sq);

/**
 * ts turns Int to rank and file
 */
vector<int> InttoRankFile(int sq);
