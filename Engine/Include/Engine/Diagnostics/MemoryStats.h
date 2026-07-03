#pragma once

// ============================================================================
// File: Engine/Include/Engine/Diagnostics/MemoryStats.h
// Memory statistics interface. Queries the operating system for physical
// memory information (total, available, used) and current process memory
// usage (RSS), with human-readable formatting helpers.
// ============================================================================

#include "Engine/Core/Types.h"

#include <string>

namespace engine::diagnostics
{

using engine::core::u64;
using engine::core::f64;

/// @brief Queries OS-level and process-level memory statistics.
///
/// All methods are static; the class cannot be instantiated.
/// Platform back-ends:
///   - Linux  : `/proc/meminfo`, `/proc/self/status`
///   - macOS  : `sysctl` (`HW_MEMSIZE`), `getrusage()`
///   - Windows: `GlobalMemoryStatusEx()`, `GetProcessMemoryInfo()`
class MemoryStats
{
public:
    MemoryStats() = delete;

    /// @brief Returns the total physical RAM installed on the system (bytes).
    [[nodiscard]] static u64 TotalPhysicalMemory();

    /// @brief Returns the currently available physical RAM (bytes).
    [[nodiscard]] static u64 AvailablePhysicalMemory();

    /// @brief Returns the physical RAM in use, computed as
    ///        `TotalPhysicalMemory() - AvailablePhysicalMemory()` (bytes).
    [[nodiscard]] static u64 UsedPhysicalMemory();

    /// @brief Returns the current process's resident set size (RSS) in bytes.
    [[nodiscard]] static u64 ProcessMemoryUsage();

    /// @brief Returns the physical memory usage as a percentage (0.0 – 100.0).
    [[nodiscard]] static f64 MemoryUsagePercentage();

    /// @brief Returns a human-readable multi-line report of all memory stats.
    ///
    /// Values are automatically formatted to the most appropriate unit
    /// (KB, MB, or GB) for readability.
    [[nodiscard]] static std::string Report();
};

} // namespace engine::diagnostics