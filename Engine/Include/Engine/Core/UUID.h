#pragma once

/// @file UUID.h
/// @brief RFC 4122 UUID version-4 (random) implementation.
///
/// Provides generation, parsing, string conversion, and hashing for
/// 128-bit universally unique identifiers.  The generated UUIDs conform
/// to the variant-1 / version-4 layout required by the specification.
///
/// Usage:
///   auto id = engine::core::UUID{};              // random v4
///   auto id2 = engine::core::UUID::FromString("550e8400-e29b-41d4-a716-446655440000");
///   std::cout << id << "\n";

#include "Engine/Core/Types.h"

#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

namespace engine::core {

/// @brief A 128-bit UUID (version 4, variant 1).
///
/// Stores the UUID as 16 raw bytes in big-endian / standard layout order.
class UUID
{
public:
    /// @brief Generates a random UUID v4 on construction.
    UUID();

    /// @brief Constructs a UUID from an explicit 16-byte array.
    /// @param bytes Raw bytes in standard UUID layout order.
    explicit UUID(std::array<u8, 16> bytes);

    // -- Factory methods -----------------------------------------------------

    /// @brief Parses a UUID string in the form
    ///        "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
    ///
    /// The version nibble (character at index 14) must be '4'.
    /// On failure, returns UUID::Invalid().
    [[nodiscard]] static UUID FromString(std::string_view str);

    /// @brief Alias for FromString with identical semantics.
    [[nodiscard]] static UUID Parse(std::string_view str);

    // -- Observers ------------------------------------------------------------

    /// @brief Returns the standard 8-4-4-4-12 hyphenated string representation.
    [[nodiscard]] std::string ToString() const;

    /// @brief Returns true if the UUID is not the all-zeros sentinel.
    [[nodiscard]] bool IsValid() const;

    /// @brief Returns a hash value suitable for use with std::unordered_map/set.
    [[nodiscard]] std::size_t Hash() const;

    /// @brief Read-only access to the underlying 16 bytes.
    [[nodiscard]] const std::array<u8, 16>& Bytes() const { return m_bytes; }

    /// @brief Returns the all-zeros sentinel UUID.
    [[nodiscard]] static UUID Invalid();

    // -- Comparison operators -------------------------------------------------

    [[nodiscard]] bool operator==(const UUID& other) const { return m_bytes == other.m_bytes; }
    [[nodiscard]] bool operator!=(const UUID& other) const { return m_bytes != other.m_bytes; }

    /// @brief Lexicographic comparison (enables use as key in ordered containers).
    [[nodiscard]] bool operator<(const UUID& other) const { return m_bytes < other.m_bytes; }

    // -- Stream output --------------------------------------------------------

    friend std::ostream& operator<<(std::ostream& os, const UUID& uuid)
    {
        return os << uuid.ToString();
    }

private:
    std::array<u8, 16> m_bytes{};
};

} // namespace engine::core

// ---------------------------------------------------------------------------
// std::hash specialization – enables UUID as key in std::unordered_map/set.
// ---------------------------------------------------------------------------
template <>
struct std::hash<engine::core::UUID>
{
    [[nodiscard]] std::size_t operator()(const engine::core::UUID& uuid) const noexcept
    {
        return uuid.Hash();
    }
};