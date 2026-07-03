/**
 * @file CommandLine.cpp
 * @brief Implementation of the CommandLine argument parser declared in
 *        Engine/Utilities/CommandLine.h.
 */

#include "Engine/Utilities/CommandLine.h"

#include <cstdlib>
#include <charconv>
#include <system_error>

namespace engine::util
{

using engine::core::usize;
using engine::core::i32;
using engine::core::f64;

    CommandLine::CommandLine(int argc, const char* const argv[])
    {
        // Store all raw arguments.
        m_args.reserve(static_cast<usize>(argc));
        for (int i = 0; i < argc; ++i)
        {
            m_args.emplace_back(argv[i]);
        }

        if (argc > 0)
        {
            m_executableName = m_args[0];
        }

        // Parse key-value pairs.  Arguments starting with "--" are keys.
        // The immediately following argument, if it does NOT start with "--",
        // is treated as the value for that key.
        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = m_args[static_cast<usize>(i)];
            if (arg.starts_with("--"))
            {
                std::string key(arg.substr(2)); // strip the leading "--"

                // Check if the next argument exists and is not another flag.
                if (i + 1 < argc)
                {
                    std::string_view nextArg = m_args[static_cast<usize>(i + 1)];
                    if (!nextArg.starts_with("--"))
                    {
                        m_values[key] = std::string(nextArg);
                        ++i; // consume the value argument
                        continue;
                    }
                }

                // Flag with no value.
                m_values[key] = "";
            }
        }
    }

    bool CommandLine::Has(std::string_view name) const
    {
        return m_values.find(std::string(name)) != m_values.end();
    }

    std::string CommandLine::Get(std::string_view name, std::string_view defaultValue) const
    {
        auto it = m_values.find(std::string(name));
        if (it != m_values.end())
        {
            return it->second;
        }
        return std::string(defaultValue);
    }

    i32 CommandLine::GetInt(std::string_view name, i32 defaultValue) const
    {
        std::string value = Get(name);
        if (value.empty())
        {
            return defaultValue;
        }

        // Use std::from_chars for robust parsing.
        i32 result = defaultValue;
        const char* begin = value.data();
        const char* end = begin + value.size();

        auto [ptr, ec] = std::from_chars(begin, end, result);
        if (ec != std::errc{} || ptr != end)
        {
            return defaultValue;
        }

        return result;
    }

    f64 CommandLine::GetFloat(std::string_view name, f64 defaultValue) const
    {
        std::string value = Get(name);
        if (value.empty())
        {
            return defaultValue;
        }

        // std::from_chars for double is available in C++17 but may not be
        // implemented on all standard libraries (notably older MSVC).
        // Fall back to strtod for maximum portability.
        char* end = nullptr;
        f64 result = std::strtod(value.c_str(), &end);
        if (end == value.c_str())
        {
            return defaultValue;
        }
        return result;
    }

    bool CommandLine::GetBool(std::string_view name, bool defaultValue) const
    {
        return Has(name) ? true : defaultValue;
    }

} // namespace engine::util