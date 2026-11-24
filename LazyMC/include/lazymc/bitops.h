#pragma once
// Portable popcount / count-trailing-zeros for 64-bit words (GCC/Clang/MSVC).
#include <cstdint>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace lazymc {

inline int popcount64(std::uint64_t x) {
#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_M_X64)
    return static_cast<int>(__popcnt64(x));
#else
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    return static_cast<int>((((x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL) * 0x0101010101010101ULL) >> 56);
#endif
#else
    return __builtin_popcountll(x);
#endif
}

// x must be non-zero.
inline int ctz64(std::uint64_t x) {
#if defined(_MSC_VER) && !defined(__clang__)
    unsigned long idx;
    _BitScanForward64(&idx, x);
    return static_cast<int>(idx);
#else
    return __builtin_ctzll(x);
#endif
}

} // namespace lazymc
