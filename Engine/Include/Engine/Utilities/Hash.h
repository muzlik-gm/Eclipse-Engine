#pragma once

/**
 * @file Hash.h
 * @brief Hashing primitives and combiners.
 *
 * Provides constexpr FNV-1a implementations (32/64-bit), a runtime
 * convenience wrapper, and boost-style hash-combine helpers for building
 * composite hashes from multiple fields.
 */

#include "Engine/Core/Types.h"

#include <cstddef>
#include <cstdint>

namespace engine::util
{

using engine::core::u8;
using engine::core::u32;
using engine::core::u64;
using engine::core::usize;

    // ========================================================================
    // FNV-1a constexpr implementations
    // ========================================================================

    /// Standard FNV-1a 64-bit hash.  Fully constexpr so it can be used in
    /// constant expressions and template parameters.
    ///
    /// @param data  Pointer to the byte buffer to hash.
    /// @param size  Number of bytes in the buffer.
    /// @return      The 64-bit FNV-1a hash value.
    constexpr u64 FNV1a64(const void* data, usize size)
    {
        const u8* bytes = static_cast<const u8*>(data);

        u64 hash = 0xCBF29CE484222325ULL; // FNV offset basis
        constexpr u64 kPrime = 0x100000001B3ULL; // FNV prime

        for (usize i = 0; i < size; ++i)
        {
            hash ^= static_cast<u64>(bytes[i]);
            hash *= kPrime;
        }

        return hash;
    }

    /// Standard FNV-1a 32-bit hash.  Fully constexpr.
    ///
    /// @param data  Pointer to the byte buffer to hash.
    /// @param size  Number of bytes in the buffer.
    /// @return      The 32-bit FNV-1a hash value.
    constexpr u32 FNV1a32(const void* data, usize size)
    {
        const u8* bytes = static_cast<const u8*>(data);

        u32 hash = 0x811C9DC5U; // FNV offset basis
        constexpr u32 kPrime = 0x01000193U; // FNV prime

        for (usize i = 0; i < size; ++i)
        {
            hash ^= static_cast<u32>(bytes[i]);
            hash *= kPrime;
        }

        return hash;
    }

    // ========================================================================
    // Runtime convenience wrappers
    // ========================================================================

    /// Computes a 64-bit FNV-1a hash of an arbitrary byte buffer.
    /// Delegates to FNV1a64 but is provided as a non-constexpr overload for
    /// use in non-constexpr contexts where the function-pointer address
    /// matters (e.g. passing to a hashing interface).
    [[nodiscard]] u64 HashBytes(const void* data, usize size);

    // ========================================================================
    // Hash combination (boost-style)
    // ========================================================================

    /// Combines @p seed with @p hash using the boost hash-combine algorithm:
    ///     seed ^ (hash + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2))
    [[nodiscard]] u64 HashCombine(u64 seed, u64 hash);

    /// Combines @p seed with two additional hash values @p h1 and @p h2 in
    /// sequence, returning the final combined hash.
    [[nodiscard]] u64 HashCombine(u64 seed, u64 h1, u64 h2);

} // namespace engine::util