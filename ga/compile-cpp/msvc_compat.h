#pragma once

// The engine is portable C++ except for these GCC builtin spellings.  MSVC
// provides equivalent x64 intrinsics, so map them here without changing the
// engine/evaluation source owned by the evaluation component.
#ifdef _MSC_VER
#include <cstdlib>
#include <intrin.h>

// POSIX has random(); MSVC provides rand(). The engine uses random only to
// create default, non-training bot weights.
inline int random() {
    return std::rand();
}

inline int __builtin_ctzll(unsigned long long value) {
    unsigned long index = 0;
    return _BitScanForward64(&index, value) ? static_cast<int>(index) : 64;
}

inline int __builtin_popcountll(unsigned long long value) {
    return static_cast<int>(__popcnt64(value));
}
#endif
