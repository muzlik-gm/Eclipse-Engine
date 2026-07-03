/// @file UUID.cpp
/// @brief Implementation of the UUID v4 class.

#include "Engine/Core/UUID.h"

#include <array>
#include <charconv>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>

namespace engine::core {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

UUID::UUID()
{
    std::random_device rd;
    std::mt19937_64    gen(rd());
    std::uniform_int_distribution<u64> dist;

    const u64 part0 = dist(gen);
    const u64 part1 = dist(gen);

    // Scatter random bits into the 16-byte buffer (native byte order is fine
    // because the values are purely random — only the version/variant masks
    // need to land on the correct byte indices).
    for (std::size_t i = 0; i < 8; ++i)
    {
        m_bytes[i]     = static_cast<u8>(part0 >> (i * 8));
        m_bytes[8 + i] = static_cast<u8>(part1 >> (i * 8));
    }

    // RFC 4122 §4.4 — set version 4 bits (byte 6 high nibble = 0100).
    m_bytes[6] = static_cast<u8>((m_bytes[6] & 0x0F) | 0x40);

    // RFC 4122 §4.4 — set variant 1 bits (byte 8 high 2 bits = 10).
    m_bytes[8] = static_cast<u8>((m_bytes[8] & 0x3F) | 0x80);
}

UUID::UUID(std::array<u8, 16> bytes)
    : m_bytes(std::move(bytes))
{
}

// ---------------------------------------------------------------------------
// Factory methods
// ---------------------------------------------------------------------------

UUID UUID::FromString(std::string_view str)
{
    // Expected format: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"  (36 chars)
    if (str.size() != 36)
    {
        return Invalid();
    }

    // Hyphens must appear at positions 8, 13, 18, 23.
    constexpr std::array<std::size_t, 4> kHyphenPositions = {8, 13, 18, 23};
    for (const auto pos : kHyphenPositions)
    {
        if (str[pos] != '-')
        {
            return Invalid();
        }
    }

    // Version nibble (char at index 14) must be '4'.
    if (str[14] != '4')
    {
        return Invalid();
    }

    std::array<u8, 16> bytes{};

    // Helper: parse a single hex character into its numeric value.
    const auto hexValue = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    // Walk through the 32 hex digit positions (skipping hyphens).
    std::size_t byteIdx = 0;
    for (std::size_t i = 0; i < 36; ++i)
    {
        if (str[i] == '-')
        {
            continue;
        }

        const int hi = hexValue(str[i]);
        const int lo = (i + 1 < 36 && str[i + 1] != '-') ? hexValue(str[i + 1]) : -1;

        if (hi < 0 || lo < 0)
        {
            return Invalid();
        }

        bytes[byteIdx++] = static_cast<u8>((hi << 4) | lo);
        ++i; // skip the second nibble
    }

    if (byteIdx != 16)
    {
        return Invalid();
    }

    return UUID{bytes};
}

UUID UUID::Parse(std::string_view str)
{
    return FromString(str);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

std::string UUID::ToString() const
{
    // Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < 16; ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            oss << '-';
        }
        oss << std::setw(2) << static_cast<unsigned>(m_bytes[i]);
    }

    return oss.str();
}

bool UUID::IsValid() const
{
    return m_bytes != std::array<u8, 16>{};
}

std::size_t UUID::Hash() const
{
    // FNV-1a 64-bit hash over all 16 bytes.
    constexpr u64 kOffsetBasis = 14695981039346656037ULL;
    constexpr u64 kPrime       = 1099511628211ULL;

    u64 hash = kOffsetBasis;
    for (const u8 byte : m_bytes)
    {
        hash ^= static_cast<u64>(byte);
        hash *= kPrime;
    }

    return static_cast<std::size_t>(hash);
}

UUID UUID::Invalid()
{
    return UUID{std::array<u8, 16>{}};
}

} // namespace engine::core