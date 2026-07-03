#pragma once

/**
 * @file BuildInfo.h
 * @brief Compile-time and minimal-runtime build metadata.
 *
 * Provides a centralised API for querying the engine name, version, compiler,
 * platform, and build date/time.  Purely compile-time queries use consteval
 * so they are evaluated at compile time and incur no runtime cost.
 */

#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Types.h"

#include <string>
#include <string_view>

namespace engine::diagnostics
{

/// @brief Static access to build-time metadata.
///
/// All methods are static; the class cannot be instantiated.
/// Consteval methods are resolved entirely at compile time.
class BuildInfo
{
public:
    BuildInfo() = delete;

    /// @brief Returns the engine name.
    [[nodiscard]] static consteval std::string_view EngineName() { return "Engine"; }

    /// @brief Returns the engine version string (semver).
    [[nodiscard]] static consteval std::string_view VersionString() { return "0.1.0"; }

    /// @brief Returns the date the translation unit was compiled (__DATE__).
    [[nodiscard]] static consteval std::string_view BuildDate() { return __DATE__; }

    /// @brief Returns the time the translation unit was compiled (__TIME__).
    [[nodiscard]] static consteval std::string_view BuildTime() { return __TIME__; }

    /// @brief Returns the compiler name (e.g. "Clang", "GCC", "MSVC").
    [[nodiscard]] static std::string_view CompilerName();

    /// @brief Returns the target platform name (e.g. "Linux", "Windows", "macOS").
    [[nodiscard]] static std::string_view PlatformName();

    /// @brief Returns true when the engine is compiled in a debug build.
    [[nodiscard]] static bool IsDebugBuild();

    /// @brief Returns a formatted multi-line string containing all build info.
    [[nodiscard]] static std::string GetFullBuildInfoString();
};

} // namespace engine::diagnostics