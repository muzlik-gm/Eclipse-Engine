#pragma once

/**
 * @file String.h
 * @brief General-purpose string manipulation utilities.
 *
 * Provides a collection of non-allocating string-view-based helpers (prefix /
 * suffix checks, case-insensitive comparison, trimming) as well as functions
 * that produce new std::string objects (split, join, replace, format).  An
 * FNV-1a convenience hash and a basic UTF-8 code-point splitter are also
 * included.
 */

#include "Engine/Core/Types.h"

#include <spdlog/fmt/fmt.h>
#include <string>
#include <string_view>
#include <vector>

namespace engine::util
{

using engine::core::usize;

    // ========================================================================
    // Query helpers — all operate on string_view, zero allocations.
    // ========================================================================

    /// Returns true if @p str begins with @p prefix.
    [[nodiscard]] bool StartsWith(std::string_view str, std::string_view prefix);

    /// Returns true if @p str ends with @p suffix.
    [[nodiscard]] bool EndsWith(std::string_view str, std::string_view suffix);

    /// Returns true if @p substr occurs anywhere inside @p str.
    [[nodiscard]] bool Contains(std::string_view str, std::string_view substr);

    /// Case-insensitive equality comparison using ASCII rules.
    [[nodiscard]] bool EqualsIgnoreCase(std::string_view a, std::string_view b);

    // ========================================================================
    // Case conversion — return newly allocated strings.
    // ========================================================================

    /// Returns a lower-cased copy of @p str (ASCII only).
    [[nodiscard]] std::string ToLower(std::string_view str);

    /// Returns an upper-cased copy of @p str (ASCII only).
    [[nodiscard]] std::string ToUpper(std::string_view str);

    // ========================================================================
    // Whitespace trimming — return newly allocated strings.
    // ========================================================================

    /// Removes leading and trailing whitespace (ASCII space, tab, newline,
    /// carriage return, form-feed, vertical-tab).
    [[nodiscard]] std::string Trim(std::string_view str);

    /// Removes leading whitespace only.
    [[nodiscard]] std::string TrimLeft(std::string_view str);

    /// Removes trailing whitespace only.
    [[nodiscard]] std::string TrimRight(std::string_view str);

    // ========================================================================
    // Split / Join / Replace
    // ========================================================================

    /// Splits @p str on every occurrence of @p delimiter.  Consecutive
    /// delimiters produce no empty entries in the result.
    [[nodiscard]] std::vector<std::string> Split(std::string_view str, char delimiter);

    /// Splits @p str on every occurrence of @p delimiter, keeping empty
    /// entries that arise from consecutive delimiters.
    [[nodiscard]] std::vector<std::string> SplitKeepEmpty(std::string_view str, char delimiter);

    /// Concatenates @p parts with @p delimiter inserted between consecutive
    /// elements.
    [[nodiscard]] std::string Join(const std::vector<std::string>& parts, std::string_view delimiter);

    /// Returns a copy of @p str with every non-overlapping occurrence of
    /// @p from replaced by @p to.
    [[nodiscard]] std::string Replace(std::string_view str, std::string_view from, std::string_view to);

    // ========================================================================
    // Formatting
    // ========================================================================

    /// Formats arguments according to @p format using {fmt} and returns the
    /// result as a std::string.
    [[nodiscard]] std::string Format(std::string_view format, fmt::format_args args);

    // ========================================================================
    // Hashing
    // ========================================================================

    /// Computes the 64-bit FNV-1a hash of @p str.
    [[nodiscard]] usize Hash(std::string_view str);

    // ========================================================================
    // UTF-8
    // ========================================================================

    /// Splits a UTF-8 encoded string into individual code points, each
    /// represented as a std::u8string.  Handles ASCII and valid 2-4 byte
    /// sequences.  Invalid byte sequences are passed through as individual
    /// bytes.
    [[nodiscard]] std::vector<std::u8string> SplitUTF8(std::u8string_view str);

} // namespace engine::util