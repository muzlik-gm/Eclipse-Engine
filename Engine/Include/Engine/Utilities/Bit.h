#pragma once

/**
 * @file Bit.h
 * @brief Compile-time bit-manipulation utilities.
 *
 * All functions are constexpr and templated on an unsigned integral type @p T.
 * They delegate to <bit> intrinsics (std::popcount, std::countl_zero, etc.)
 * where appropriate.
 */

#include "Engine/Core/Types.h"

#include <bit>
#include <type_traits>

namespace engine::util
{

    /// Sets bit @p bit (0-based from LSB) in @p value.
    template <typename T>
    [[nodiscard]] constexpr T SetBit(T value, i32 bit) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value | (T(1) << bit);
    }

    /// Clears bit @p bit (0-based from LSB) in @p value.
    template <typename T>
    [[nodiscard]] constexpr T ClearBit(T value, i32 bit) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value & ~(T(1) << bit);
    }

    /// Toggles (flips) bit @p bit (0-based from LSB) in @p value.
    template <typename T>
    [[nodiscard]] constexpr T ToggleBit(T value, i32 bit) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value ^ (T(1) << bit);
    }

    /// Returns true when bit @p bit (0-based from LSB) is set in @p value.
    template <typename T>
    [[nodiscard]] constexpr bool IsBitSet(T value, i32 bit) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return (value & (T(1) << bit)) != T(0);
    }

    /// Sets every bit that is set in @p mask within @p value.
    template <typename T>
    [[nodiscard]] constexpr T SetBits(T value, T mask) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value | mask;
    }

    /// Clears every bit that is set in @p mask within @p value.
    template <typename T>
    [[nodiscard]] constexpr T ClearBits(T value, T mask) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value & ~mask;
    }

    /// Toggles (flips) every bit that is set in @p mask within @p value.
    template <typename T>
    [[nodiscard]] constexpr T ToggleBits(T value, T mask) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return value ^ mask;
    }

    /// Returns true when at least one bit in @p mask is set in @p value.
    template <typename T>
    [[nodiscard]] constexpr bool AnyBitSet(T value, T mask) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return (value & mask) != T(0);
    }

    /// Returns true when every bit in @p mask is set in @p value.
    template <typename T>
    [[nodiscard]] constexpr bool AllBitsSet(T value, T mask) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return (value & mask) == mask;
    }

    /// Counts the number of set bits (population count) in @p value.
    template <typename T>
    [[nodiscard]] constexpr i32 CountSetBits(T value) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return static_cast<i32>(std::popcount(value));
    }

    /// Returns the number of leading zero bits in @p value.
    /// Result is undefined when @p value is zero.
    template <typename T>
    [[nodiscard]] constexpr i32 CountLeadingZeros(T value) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return static_cast<i32>(std::countl_zero(value));
    }

    /// Returns the number of trailing zero bits in @p value.
    /// Result is undefined when @p value is zero.
    template <typename T>
    [[nodiscard]] constexpr i32 CountTrailingZeros(T value) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return static_cast<i32>(std::countr_zero(value));
    }

    /// Returns the 0-based index of the highest set bit in @p value.
    /// Returns -1 when @p value is zero.
    template <typename T>
    [[nodiscard]] constexpr i32 FindHighestSetBit(T value) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        if (value == T(0))
        {
            return -1;
        }
        return static_cast<i32>(sizeof(T) * 8 - 1 - std::countl_zero(value));
    }

    /// Rotates the bits of @p value to the left by @p count positions.
    template <typename T>
    [[nodiscard]] constexpr T RotateLeft(T value, i32 count) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        constexpr i32 kBits = static_cast<i32>(sizeof(T) * 8);
        count &= (kBits - 1); // modulo bit-width
        if (count == 0)
        {
            return value;
        }
        return (value << count) | (value >> (kBits - count));
    }

    /// Rotates the bits of @p value to the right by @p count positions.
    template <typename T>
    [[nodiscard]] constexpr T RotateRight(T value, i32 count) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        constexpr i32 kBits = static_cast<i32>(sizeof(T) * 8);
        count &= (kBits - 1); // modulo bit-width
        if (count == 0)
        {
            return value;
        }
        return (value >> count) | (value << (kBits - count));
    }

    /// Reverses the byte order of @p value (big-endian ↔ little-endian).
    template <typename T>
    [[nodiscard]] constexpr T ByteSwap(T value) noexcept
        requires std::is_unsigned_v<T> && std::is_integral_v<T>
    {
        return std::byteswap(value);
    }

} // namespace engine::util