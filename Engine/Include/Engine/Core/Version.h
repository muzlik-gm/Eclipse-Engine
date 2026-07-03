#pragma once

/**
 * @file Version.h
 * @brief Engine version information exposed as a compile-time struct.
 *
 * The version numbers here are the single source of truth.  CMake's
 * project(VERSION …) should be kept in sync manually — the preprocessor
 * cannot read CMake variables, so we duplicate them here for runtime use.
 */

namespace engine
{
    /// Compile-time engine version descriptor.
    ///
    /// All members are static constexpr so they can be consumed both at
    /// compile time (e.g. in static_asserts or as template arguments) and
    /// at runtime (e.g. logging the engine version on startup).
    struct EngineVersion
    {
        static constexpr int Major = 0;
        static constexpr int Minor = 1;
        static constexpr int Patch = 0;

        /// Returns the version as a dotted string, e.g. "0.1.0".
        static consteval const char* String()
        {
            return "0.1.0";
        }

        /// Returns a human-friendly version string including the engine name,
        /// e.g. "Engine v0.1.0".
        static consteval const char* ToString()
        {
            return "Engine v0.1.0";
        }
    };
} // namespace engine