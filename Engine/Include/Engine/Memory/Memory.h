// ============================================================================
// File: Engine/Include/Engine/Memory/Memory.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <cstring>

namespace engine::memory {

    using engine::core::u8;
    using engine::core::usize;

    // ========================================================================
    // Alignment helpers
    // ========================================================================

    /// Round `value` up to the nearest multiple of `alignment`.
    /// Precondition: alignment must be a power of two.
    [[nodiscard]] constexpr usize AlignUp(usize value, usize alignment) noexcept
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    /// Round `value` down to the nearest multiple of `alignment`.
    /// Precondition: alignment must be a power of two.
    [[nodiscard]] constexpr usize AlignDown(usize value, usize alignment) noexcept
    {
        return value & ~(alignment - 1);
    }

    // ========================================================================
    // Low-level memory operations
    // ========================================================================

    /// Copy `size` bytes from `src` to `dst`.
    inline void CopyBytes(void* dst, const void* src, usize size) noexcept
    {
        std::memcpy(dst, src, size);
    }

    /// Fill `size` bytes starting at `dst` with `value` (reinterpreted as u8).
    inline void SetBytes(void* dst, u8 value, usize size) noexcept
    {
        std::memset(dst, static_cast<int>(value), size);
    }

    /// Zero out `size` bytes starting at `dst`.
    inline void ZeroMemory(void* dst, usize size) noexcept
    {
        std::memset(dst, 0, size);
    }

} // namespace engine::memory