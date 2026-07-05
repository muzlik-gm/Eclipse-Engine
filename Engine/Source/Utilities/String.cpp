/**
 * @file String.cpp
 * @brief Implementations of the string manipulation utilities declared in
 *        Engine/Utilities/String.h.
 */

#include "Engine/Utilities/String.h"
#include "Engine/Utilities/Hash.h"
#include "Engine/Utilities/UTF.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iterator>

namespace engine::util
{

using engine::core::i32;

    // ========================================================================
    // Query helpers
    // ========================================================================

    bool StartsWith(std::string_view str, std::string_view prefix)
    {
        if (prefix.size() > str.size())
        {
            return false;
        }
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    bool EndsWith(std::string_view str, std::string_view suffix)
    {
        if (suffix.size() > str.size())
        {
            return false;
        }
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool Contains(std::string_view str, std::string_view substr)
    {
        return str.find(substr) != std::string_view::npos;
    }

    bool EqualsIgnoreCase(std::string_view a, std::string_view b)
    {
        if (a.size() != b.size())
        {
            return false;
        }
        return std::equal(a.begin(), a.end(), b.begin(),
            [](unsigned char lhs, unsigned char rhs)
            {
                return std::tolower(lhs) == std::tolower(rhs);
            });
    }

    // ========================================================================
    // Case conversion
    // ========================================================================

    std::string ToLower(std::string_view str)
    {
        std::string result;
        result.reserve(str.size());
        std::transform(str.begin(), str.end(), std::back_inserter(result),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    std::string ToUpper(std::string_view str)
    {
        std::string result;
        result.reserve(str.size());
        std::transform(str.begin(), str.end(), std::back_inserter(result),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return result;
    }

    // ========================================================================
    // Whitespace trimming
    // ========================================================================

    namespace
    {
        /// Returns true for ASCII whitespace characters.
        constexpr bool IsWhitespace(char c) noexcept
        {
            return c == ' '  || c == '\t' || c == '\n' ||
                   c == '\r' || c == '\f' || c == '\v';
        }
    } // anonymous namespace

    std::string Trim(std::string_view str)
    {
        const usize start = static_cast<usize>(
            std::distance(str.begin(),
                std::find_if_not(str.begin(), str.end(), IsWhitespace)));
        const usize end = static_cast<usize>(
            std::distance(str.begin(),
                std::find_if_not(str.rbegin(), str.rend(), IsWhitespace).base()));
        if (start >= end)
        {
            return "";
        }
        return std::string(str.substr(start, end - start));
    }

    std::string TrimLeft(std::string_view str)
    {
        auto it = std::find_if_not(str.begin(), str.end(), IsWhitespace);
        return std::string(it, str.end());
    }

    std::string TrimRight(std::string_view str)
    {
        auto rit = std::find_if_not(str.rbegin(), str.rend(), IsWhitespace).base();
        return std::string(str.begin(), rit);
    }

    // ========================================================================
    // Split / Join / Replace
    // ========================================================================

    std::vector<std::string> Split(std::string_view str, char delimiter)
    {
        std::vector<std::string> result;
        usize start = 0;

        while (start < str.size())
        {
            usize pos = str.find(delimiter, start);
            if (pos == std::string_view::npos)
            {
                break;
            }
            if (pos > start) // skip empty segments
            {
                result.emplace_back(str.substr(start, pos - start));
            }
            start = pos + 1;
        }

        // Append the trailing segment if non-empty.
        if (start < str.size())
        {
            result.emplace_back(str.substr(start));
        }

        return result;
    }

    std::vector<std::string> SplitKeepEmpty(std::string_view str, char delimiter)
    {
        std::vector<std::string> result;
        usize start = 0;

        while (start <= str.size())
        {
            usize pos = str.find(delimiter, start);
            if (pos == std::string_view::npos)
            {
                pos = str.size();
            }
            result.emplace_back(str.substr(start, pos - start));
            start = pos + 1;
        }

        return result;
    }

    std::string Join(const std::vector<std::string>& parts, std::string_view delimiter)
    {
        if (parts.empty())
        {
            return "";
        }

        // Pre-compute the total size to avoid repeated reallocations.
        usize totalSize = 0;
        for (usize i = 0; i < parts.size(); ++i)
        {
            totalSize += parts[i].size();
            if (i > 0)
            {
                totalSize += delimiter.size();
            }
        }

        std::string result;
        result.reserve(totalSize);

        for (usize i = 0; i < parts.size(); ++i)
        {
            if (i > 0)
            {
                result.append(delimiter);
            }
            result.append(parts[i]);
        }

        return result;
    }

    std::string Replace(std::string_view str, std::string_view from, std::string_view to)
    {
        if (from.empty())
        {
            return std::string(str);
        }

        std::string result;
        result.reserve(str.size());

        usize start = 0;
        while (start < str.size())
        {
            usize pos = str.find(from, start);
            if (pos == std::string_view::npos)
            {
                result.append(str.substr(start));
                break;
            }
            result.append(str.substr(start, pos - start));
            result.append(to);
            start = pos + from.size();
        }

        return result;
    }

    // ========================================================================
    // Formatting
    // ========================================================================

    std::string Format(std::string_view format, fmt::format_args args)
    {
        return fmt::vformat(format, args);
    }

    // ========================================================================
    // Hashing
    // ========================================================================

    usize Hash(std::string_view str)
    {
        return static_cast<usize>(FNV1a64(str.data(), str.size()));
    }

    // ========================================================================
    // UTF-8 splitting
    // ========================================================================

    std::vector<std::u8string> SplitUTF8(std::u8string_view str)
    {
        std::vector<std::u8string> result;

        const char8_t* data = str.data();
        const usize len = str.size();
        usize pos = 0;

        while (pos < len)
        {
            char8_t firstByte = data[pos];
            i32 seqLen = UTF8CodePointLength(static_cast<std::uint8_t>(firstByte));

            if (seqLen <= 0)
            {
                // Invalid leading byte — push it as a single byte and advance.
                result.emplace_back(&firstByte, 1);
                ++pos;
                continue;
            }

            // Clamp to available data.
            usize actualLen = static_cast<usize>(seqLen);
            if (pos + actualLen > len)
            {
                // Truncated sequence — push remaining bytes as-is.
                result.emplace_back(&data[pos], len - pos);
                break;
            }

            // Validate continuation bytes.
            bool valid = true;
            for (usize i = 1; i < actualLen; ++i)
            {
                if (!IsUTF8ContinuationByte(static_cast<std::uint8_t>(data[pos + i])))
                {
                    valid = false;
                    break;
                }
            }

            if (!valid)
            {
                // Malformed sequence — emit the leading byte alone.
                result.emplace_back(&firstByte, 1);
                ++pos;
                continue;
            }

            result.emplace_back(&data[pos], actualLen);
            pos += actualLen;
        }

        return result;
    }

} // namespace engine::util