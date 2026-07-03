#pragma once

/**
 * @file Types.h
 * @brief Fundamental type aliases, sentinel constants, and small utility
 *        templates used across the entire engine.
 *
 * Every translation unit in the engine ultimately depends on this header.
 * It intentionally avoids pulling in heavyweight standard headers — only
 * <cstdint>, <cstddef>, <limits>, <type_traits>, and <bit> are included.
 */

#include <bit>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace engine::core
{
    // ========================================================================
    // Integer type aliases — fixed-width everywhere, unambiguous intent.
    // ========================================================================

    /// Signed 8-bit integer.
    using i8  = std::int8_t;

    /// Signed 16-bit integer.
    using i16 = std::int16_t;

    /// Signed 32-bit integer.
    using i32 = std::int32_t;

    /// Signed 64-bit integer.
    using i64 = std::int64_t;

    /// Unsigned 8-bit integer.
    using u8  = std::uint8_t;

    /// Unsigned 16-bit integer.
    using u16 = std::uint16_t;

    /// Unsigned 32-bit integer.
    using u32 = std::uint32_t;

    /// Unsigned 64-bit integer.
    using u64 = std::uint64_t;

    /// IEEE-754 single-precision floating point.
    using f32 = float;

    /// IEEE-754 double-precision floating point.
    using f64 = double;

    /// Unsigned size type — matches std::size_t semantics but is named
    /// consistently with the other fixed-width aliases.
    using usize = std::size_t;

    /// Signed counterpart to usize.
    using isize = std::ptrdiff_t;

    /// Individual byte with no arithmetic semantics.
    using byte = std::byte;

    // ========================================================================
    // Sentinel / invalid-value constants
    // ========================================================================

    /// Index value that signals "no valid position" in containers and
    /// slot maps that use usize as their index type.
    inline constexpr usize kInvalidIndex = std::numeric_limits<usize>::max();

    /// ID value that signals "unassigned" or "null" for 32-bit identifiers.
    inline constexpr u32 kInvalidId = std::numeric_limits<u32>::max();

    /// Smallest representable difference such that 1.0f + kFloatEpsilon != 1.0f.
    inline constexpr f32 kFloatEpsilon = std::numeric_limits<f32>::epsilon();

    /// Ratio of a circle's circumference to its diameter.
    inline constexpr f64 kPi = 3.14159265358979323846;

    // ========================================================================
    // Non-copyable / Non-movable base classes
    // ========================================================================

    /// Inherit privately from this to delete copy construction and copy
    /// assignment for the derived class.  Provides a clear, self-documenting
    /// alternative to manually deleting the two special members.
    class NonCopyable
    {
    protected:
        NonCopyable()  = default;
        ~NonCopyable() = default;

    public:
        NonCopyable(const NonCopyable&)            = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

    /// Inherit privately from this to delete move construction and move
    /// assignment.  Useful for types whose address stability is a
    /// requirement (e.g. objects stored in intrusive containers).
    class NonMovable
    {
    protected:
        NonMovable()  = default;
        ~NonMovable() = default;

    public:
        NonMovable(NonMovable&&)            = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };

    // ========================================================================
    // General-purpose constexpr math helpers
    // ========================================================================

    /// Returns the smaller of two comparable values.
    template <typename T>
    [[nodiscard]] constexpr auto Min(const T& a, const T& b) noexcept
        -> const T&
    {
        return (b < a) ? b : a;
    }

    /// Returns the larger of two comparable values.
    template <typename T>
    [[nodiscard]] constexpr auto Max(const T& a, const T& b) noexcept
        -> const T&
    {
        return (a < b) ? b : a;
    }

    /// Clamps a value into the inclusive range [lo, hi].
    template <typename T>
    [[nodiscard]] constexpr const T& Clamp(
        const T& value, const T& lo, const T& hi) noexcept
    {
        return Max(lo, Min(value, hi));
    }

    /// Linearly interpolates between a and b by factor t (unclamped).
    /// When t = 0 the result is a; when t = 1 the result is b.
    template <typename T>
    [[nodiscard]] constexpr T Lerp(const T& a, const T& b, const T& t) noexcept
        requires std::is_arithmetic_v<T>
    {
        return a + t * (b - a);
    }

    /// Converts an angle in degrees to radians.
    template <typename T>
    [[nodiscard]] constexpr T DegreesToRadians(T degrees) noexcept
        requires std::is_floating_point_v<T>
    {
        return static_cast<T>(degrees * kPi / static_cast<T>(180.0));
    }

    /// Converts an angle in radians to degrees.
    template <typename T>
    [[nodiscard]] constexpr T RadiansToDegrees(T radians) noexcept
        requires std::is_floating_point_v<T>
    {
        return static_cast<T>(radians * static_cast<T>(180.0) / kPi);
    }

    // ========================================================================
    // Integer bit-manipulation helpers
    // ========================================================================

    /// True when n is a positive power of two (1, 2, 4, 8, …).
    template <typename T>
    [[nodiscard]] constexpr bool IsPowerOfTwo(T n) noexcept
        requires std::is_integral_v<T>
    {
        return n > 0 && (n & (n - 1)) == 0;
    }

    /// Rounds value up to the next multiple of alignment.
    /// Alignment must be a power of two.
    template <typename T>
    [[nodiscard]] constexpr T AlignUp(T value, T alignment) noexcept
        requires std::is_integral_v<T>
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    /// Returns a mask with only bit n set (0-based from LSB).
    template <typename T>
    [[nodiscard]] constexpr T Bit(unsigned int n) noexcept
        requires std::is_integral_v<T>
    {
        return static_cast<T>(T(1) << n);
    }

    /// True when the n-th bit (0-based from LSB) is set in value.
    template <typename T>
    [[nodiscard]] constexpr bool IsBitSet(T value, unsigned int bit) noexcept
        requires std::is_integral_v<T>
    {
        return (value & Bit<T>(bit)) != T(0);
    }

    /// Counts the number of set bits (population count).
    template <typename T>
    [[nodiscard]] constexpr int PopCount(T value) noexcept
        requires std::is_integral_v<T>
    {
        return static_cast<int>(std::popcount(static_cast<std::make_unsigned_t<T>>(value)));
    }

    /// Returns the number of trailing zero bits.  Undefined for value == 0.
    template <typename T>
    [[nodiscard]] constexpr int CountTrailingZeros(T value) noexcept
        requires std::is_integral_v<T>
    {
        return static_cast<int>(std::countr_zero(static_cast<std::make_unsigned_t<T>>(value)));
    }

    /// Returns the number of leading zero bits.  Undefined for value == 0.
    template <typename T>
    [[nodiscard]] constexpr int CountLeadingZeros(T value) noexcept
        requires std::is_integral_v<T>
    {
        return static_cast<int>(std::countl_zero(static_cast<std::make_unsigned_t<T>>(value)));
    }

    /// Returns the bit-width of an integer type (e.g. 32 for u32).
    template <typename T>
    inline constexpr usize BitsOf = sizeof(T) * 8;

} // namespace engine::core