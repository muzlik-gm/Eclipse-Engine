/**
 * @file Hash.cpp
 * @brief Implementations of runtime hashing utilities declared in
 *        Engine/Utilities/Hash.h.
 */

#include "Engine/Utilities/Hash.h"

namespace engine::util
{

    // ========================================================================
    // Runtime wrappers
    // ========================================================================

    u64 HashBytes(const void* data, usize size)
    {
        return FNV1a64(data, size);
    }

    // ========================================================================
    // Hash combination
    // ========================================================================

    u64 HashCombine(u64 seed, u64 hash)
    {
        // Boost-style hash combine.
        // The golden-ratio derived constant improves avalanche behavior.
        constexpr u64 kGoldenRatio = 0x9e3779b97f4a7c15ULL;
        seed ^= hash + kGoldenRatio + (seed << 6) + (seed >> 2);
        return seed;
    }

    u64 HashCombine(u64 seed, u64 h1, u64 h2)
    {
        seed = HashCombine(seed, h1);
        seed = HashCombine(seed, h2);
        return seed;
    }

} // namespace engine::util