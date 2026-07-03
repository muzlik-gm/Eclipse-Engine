#pragma once

/**
 * @file CommandLine.h
 * @brief Command-line argument parsing.
 *
 * Parses argc/argv into a structured representation that supports querying
 * for flags and key-value pairs.  The expected syntax is:
 *
 *   program-name --key value --flag --key2 value2 positionalA positionalB
 *
 * Anything preceded by "--" is treated as a key.  The next argument, if it
 * does not start with "--", is taken as the key's value.  Arguments that
 * are not part of any key-value pair are retained as positional arguments.
 */

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine::util
{

using engine::core::i32;
using engine::core::f64;

    /// Parses and stores command-line arguments for easy querying.
    class CommandLine
    {
    public:
        /// Constructs a CommandLine by parsing @p argc / @p argv.
        /// @p argv[0] is captured as the executable name.
        CommandLine(int argc, const char* const argv[]);

        // ====================================================================
        // Queries
        // ====================================================================

        /// Returns true when a key named @p name was provided (even if it
        /// has no associated value).
        [[nodiscard]] bool Has(std::string_view name) const;

        /// Returns the string value associated with @p name, or
        /// @p defaultValue when the key is absent.
        [[nodiscard]] std::string Get(std::string_view name, std::string_view defaultValue = "") const;

        /// Returns the integer value associated with @p name, or
        /// @p defaultValue when the key is absent or not a valid integer.
        [[nodiscard]] i32 GetInt(std::string_view name, i32 defaultValue = 0) const;

        /// Returns the floating-point value associated with @p name, or
        /// @p defaultValue when the key is absent or not a valid number.
        [[nodiscard]] f64 GetFloat(std::string_view name, f64 defaultValue = 0.0) const;

        /// Returns true when @p name is present (value is ignored; common
        /// for boolean flags like "--verbose").
        [[nodiscard]] bool GetBool(std::string_view name, bool defaultValue = false) const;

        /// Returns all arguments (including the executable name at index 0).
        [[nodiscard]] const std::vector<std::string>& Args() const
        {
            return m_args;
        }

        /// Returns the executable name (argv[0]).
        [[nodiscard]] std::string_view ExecutableName() const
        {
            return m_executableName;
        }

    private:
        /// All raw arguments as strings (including argv[0]).
        std::vector<std::string> m_args;

        /// The program name (argv[0]).
        std::string m_executableName;

        /// Parsed key → value map.  Keys without an explicit value store an
        /// empty string.
        std::unordered_map<std::string, std::string> m_values;
    };

} // namespace engine::util