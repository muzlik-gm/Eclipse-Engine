#pragma once

/**
 * @file SystemInfo.h
 * @brief Runtime system information queries (host, CPU, memory, OS).
 *
 * Provides a static API for gathering information about the machine the
 * engine is running on.  Platform-specific implementations use #ifdef
 * guards internally so that call sites remain portable.
 */

#include "Engine/Core/Types.h"

#include <string>

namespace engine::diagnostics
{

using engine::core::u32;
using engine::core::u64;

/// @brief Static access to runtime system information.
///
/// All methods are static; the class cannot be instantiated.
/// Platform-specific logic is encapsulated in the source file.
class SystemInfo
{
public:
    SystemInfo() = delete;

    /// @brief Returns the network hostname of the current machine.
    [[nodiscard]] static std::string HostName();

    /// @brief Returns the login name of the current user.
    [[nodiscard]] static std::string UserName();

    /// @brief Returns the number of logical CPU cores reported by the OS.
    [[nodiscard]] static u32 LogicalCoreCount();

    /// @brief Returns the total physical RAM in bytes.
    [[nodiscard]] static u64 TotalPhysicalMemoryBytes();

    /// @brief Returns the currently available physical RAM in bytes.
    [[nodiscard]] static u64 AvailablePhysicalMemoryBytes();

    /// @brief Returns a human-readable OS name and version string.
    [[nodiscard]] static std::string OSName();

    /// @brief Returns the target CPU architecture string
    ///        (e.g. "x86_64", "aarch64", "x86", "arm").
    [[nodiscard]] static std::string Architecture();

    /// @brief Returns a formatted multi-line string with all system info.
    [[nodiscard]] static std::string GetFullSystemInfoString();
};

} // namespace engine::diagnostics