#pragma once

/**
 * @file UTF.h
 * @brief Low-level UTF-8 encoding / decoding utilities.
 *
 * Provides compile-time and runtime helpers for inspecting individual bytes,
 * decoding and encoding Unicode code points, counting logical characters in
 * a UTF-8 string, and validating entire strings.
 */

#include "Engine/Core/Types.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace engine::util
{

using engine::core::i32;
using engine::core::u8;

    // ========================================================================
    // Byte-level inspection (all constexpr)
    // ========================================================================

    /// Returns true when @p c is a pure 7-bit ASCII character (0x00–0x7F).
    constexpr bool IsASCII(char c) noexcept
    {
        return (static_cast<unsigned char>(c) & 0x80) == 0x00;
    }

    /// Returns true when @p b is a UTF-8 continuation byte (10xxxxxx).
    constexpr bool IsUTF8ContinuationByte(std::uint8_t b) noexcept
    {
        return (b & 0xC0) == 0x80;
    }

    /// Determines the expected byte length of a UTF-8 code point from its
    /// leading byte.
    ///
    /// @param firstByte  The first (leading) byte of a code point.
    /// @return           1–4 for valid leading bytes, or -1 if the byte
    ///                   does not start a valid UTF-8 sequence.
    constexpr i32 UTF8CodePointLength(std::uint8_t firstByte) noexcept
    {
        if (firstByte <= 0x7F)       return 1; // 0xxxxxxx
        if (firstByte <= 0xBF)       return -1; // bare continuation byte
        if (firstByte <= 0xDF)       return 2; // 110xxxxx
        if (firstByte <= 0xEF)       return 3; // 1110xxxx
        if (firstByte <= 0xF7)       return 4; // 11110xxx
        return -1;                     // 11111xxx — invalid per UTF-8
    }

    // ========================================================================
    // Decode / Encode
    // ========================================================================

    /// Decodes one Unicode code point from the beginning of @p str.
    ///
    /// @param str          The string view.  On success the view is advanced
    ///                     past the decoded bytes.  On failure the view is
    ///                     advanced by one byte (the invalid one).
    /// @param outCodePoint Receives the decoded code point (0x00–0x10FFFF).
    ///                     Set to 0xFFFD (replacement character) on error.
    /// @return             The number of bytes consumed (1–4).
    i32 UTF8Decode(std::string_view& str, i32& outCodePoint);

    /// Encodes a single Unicode code point into a UTF-8 std::string.
    ///
    /// @param codePoint  A valid Unicode code point (0x00–0x10FFFF).
    ///                   Values outside this range or surrogates (0xD800–0xDFFF)
    ///                   are replaced with U+FFFD.
    /// @return           A string containing 1–4 UTF-8 bytes.
    std::string UTF8Encode(i32 codePoint);

    // ========================================================================
    // String-level utilities
    // ========================================================================

    /// Counts the number of Unicode code points (logical characters) in a
    /// UTF-8 encoded string.  Invalid byte sequences are counted as one
    /// character each.
    [[nodiscard]] i32 UTF8StringLength(std::string_view str);

    /// Validates that every byte in @p str forms correct UTF-8 sequences
    /// and that no overlong encodings or surrogate code points are present.
    [[nodiscard]] bool IsValidUTF8(std::string_view str);

} // namespace engine::util