// ============================================================================
// File: Engine/Source/Diagnostics/MemoryStats.cpp
// Full implementation of MemoryStats — queries OS-level physical memory
// information and process-level memory usage.
//
// Platform back-ends:
//   Linux  : /proc/meminfo, /proc/self/status  (VmRSS)
//   macOS  : sysctl (HW_MEMSIZE), getrusage()
//   Windows: GlobalMemoryStatusEx(), GetProcessMemoryInfo()
// ============================================================================

#include "Engine/Diagnostics/MemoryStats.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Platform.h"

#include <sstream>
#include <iomanip>
#include <cmath>

// ============================================================================
// Platform-specific includes
// ============================================================================

#if ENGINE_PLATFORM_LINUX
    #include <cstdio>
#elif ENGINE_PLATFORM_MACOS
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <sys/resource.h>
#elif ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <psapi.h>
    #pragma comment(lib, "psapi.lib")
#endif

namespace engine::diagnostics
{

using engine::core::u64;
using engine::core::f64;
using engine::core::usize;

namespace
{

// ===========================================================================
// Helpers
// ===========================================================================

/// @brief Formats a byte count into a human-readable string with the best
///        fitting unit (B, KB, MB, GB, TB).
std::string FormatBytes(u64 bytes)
{
    static constexpr const char* kUnits[] = { "B", "KB", "MB", "GB", "TB" };
    static constexpr usize kUnitCount = sizeof(kUnits) / sizeof(kUnits[0]);

    if (bytes == 0)
    {
        return "0 B";
    }

    usize unitIndex = 0;
    f64 value       = static_cast<f64>(bytes);

    while (value >= 1024.0 && unitIndex + 1 < kUnitCount)
    {
        value /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << " " << kUnits[unitIndex];
    return oss.str();
}

} // anonymous namespace

// ===========================================================================
// Linux implementation
// ===========================================================================

#if ENGINE_PLATFORM_LINUX

namespace
{

/// Reads a single numeric value (in kB) from /proc/meminfo for the given key.
/// Returns 0 on failure.
u64 ReadProcMeminfoValue(const char* key)
{
    FILE* fp = std::fopen("/proc/meminfo", "r");
    if (fp == nullptr)
    {
        ENGINE_LOG_ERROR("MemoryStats: failed to open /proc/meminfo");
        return 0;
    }

    u64 valueKB = 0;
    char line[256] = {};

    while (std::fgets(line, static_cast<int>(sizeof(line)), fp) != nullptr)
    {
        if (std::strncmp(line, key, std::strlen(key)) == 0)
        {
            // Format: "MemTotal:       16384000 kB"
            // Scan past the label and colon to the number.
            char* ptr = line;
            while (*ptr != ':' && *ptr != '\0')
            {
                ++ptr;
            }
            if (*ptr == ':')
            {
                ++ptr;
                if (std::sscanf(ptr, " %llu", reinterpret_cast<unsigned long long*>(&valueKB)) == 1)
                {
                    break;
                }
            }
        }
    }

    std::fclose(fp);
    // Convert kB to bytes.
    return valueKB * 1024ULL;
}

/// Reads VmRSS from /proc/self/status and returns it in bytes.
u64 ReadProcessRssBytes()
{
    FILE* fp = std::fopen("/proc/self/status", "r");
    if (fp == nullptr)
    {
        ENGINE_LOG_ERROR("MemoryStats: failed to open /proc/self/status");
        return 0;
    }

    u64 valueKB = 0;
    char line[256] = {};

    while (std::fgets(line, static_cast<int>(sizeof(line)), fp) != nullptr)
    {
        if (std::strncmp(line, "VmRSS:", 6) == 0)
        {
            char* ptr = line + 6;
            if (std::sscanf(ptr, " %llu", reinterpret_cast<unsigned long long*>(&valueKB)) == 1)
            {
                break;
            }
        }
    }

    std::fclose(fp);
    return valueKB * 1024ULL;
}

} // anonymous namespace (Linux)

u64 MemoryStats::TotalPhysicalMemory()
{
    return ReadProcMeminfoValue("MemTotal:");
}

u64 MemoryStats::AvailablePhysicalMemory()
{
    // MemAvailable was added in Linux 3.14. Fall back to MemFree if absent.
    u64 available = ReadProcMeminfoValue("MemAvailable:");
    if (available == 0)
    {
        // Approximate: MemFree + Buffers + Cached
        available = ReadProcMeminfoValue("MemFree:")
                  + ReadProcMeminfoValue("Buffers:")
                  + ReadProcMeminfoValue("Cached:");
    }
    return available;
}

u64 MemoryStats::UsedPhysicalMemory()
{
    u64 total     = TotalPhysicalMemory();
    u64 available = AvailablePhysicalMemory();
    return (total > available) ? (total - available) : 0;
}

u64 MemoryStats::ProcessMemoryUsage()
{
    return ReadProcessRssBytes();
}

f64 MemoryStats::MemoryUsagePercentage()
{
    u64 total = TotalPhysicalMemory();
    if (total == 0)
    {
        return 0.0;
    }
    u64 used = UsedPhysicalMemory();
    return (static_cast<f64>(used) / static_cast<f64>(total)) * 100.0;
}

#endif // ENGINE_PLATFORM_LINUX

// ===========================================================================
// macOS implementation
// ===========================================================================

#if ENGINE_PLATFORM_MACOS

u64 MemoryStats::TotalPhysicalMemory()
{
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    u64 memory = 0;
    usize len  = sizeof(memory);

    if (sysctl(mib, 2, &memory, &len, nullptr, 0) != 0)
    {
        ENGINE_LOG_ERROR("MemoryStats: sysctl(HW_MEMSIZE) failed");
        return 0;
    }

    return memory;
}

u64 MemoryStats::AvailablePhysicalMemory()
{
    // macOS does not provide a straightforward sysctl for available memory.
    // Use host_statistics64 from mach API for accurate results.
    // Fallback: approximate via vm_page size and free count from sysctl.

    u64 pageSize = 0;
    usize len    = sizeof(pageSize);

    if (sysctlbyname("vm.pagesize", &pageSize, &len, nullptr, 0) != 0)
    {
        ENGINE_LOG_ERROR("MemoryStats: sysctlbyname(vm.pagesize) failed");
        return 0;
    }

    // vm.page_free_count
    u64 freePages = 0;
    len = sizeof(freePages);
    if (sysctlbyname("vm.page_free_count", &freePages, &len, nullptr, 0) != 0)
    {
        ENGINE_LOG_ERROR("MemoryStats: sysctlbyname(vm.page_free_count) failed");
        return 0;
    }

    return freePages * pageSize;
}

u64 MemoryStats::UsedPhysicalMemory()
{
    u64 total     = TotalPhysicalMemory();
    u64 available = AvailablePhysicalMemory();
    return (total > available) ? (total - available) : 0;
}

u64 MemoryStats::ProcessMemoryUsage()
{
    // getrusage() returns max RSS, not current. For current RSS on macOS
    // the Mach task_info API is preferred, but getrusage is a reasonable
    // portable approximation.
    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) != 0)
    {
        ENGINE_LOG_ERROR("MemoryStats: getrusage() failed");
        return 0;
    }

    // ru_maxrss is in bytes on macOS (in KB on Linux).
    return static_cast<u64>(usage.ru_maxrss);
}

f64 MemoryStats::MemoryUsagePercentage()
{
    u64 total = TotalPhysicalMemory();
    if (total == 0)
    {
        return 0.0;
    }
    u64 used = UsedPhysicalMemory();
    return (static_cast<f64>(used) / static_cast<f64>(total)) * 100.0;
}

#endif // ENGINE_PLATFORM_MACOS

// ===========================================================================
// Windows implementation
// ===========================================================================

#if ENGINE_PLATFORM_WINDOWS

u64 MemoryStats::TotalPhysicalMemory()
{
    MEMORYSTATUSEX status {};
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
    {
        ENGINE_LOG_ERROR("MemoryStats: GlobalMemoryStatusEx() failed");
        return 0;
    }

    return static_cast<u64>(status.ullTotalPhys);
}

u64 MemoryStats::AvailablePhysicalMemory()
{
    MEMORYSTATUSEX status {};
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
    {
        ENGINE_LOG_ERROR("MemoryStats: GlobalMemoryStatusEx() failed");
        return 0;
    }

    return static_cast<u64>(status.ullAvailPhys);
}

u64 MemoryStats::UsedPhysicalMemory()
{
    MEMORYSTATUSEX status {};
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
    {
        return 0;
    }

    return static_cast<u64>(status.ullTotalPhys - status.ullAvailPhys);
}

u64 MemoryStats::ProcessMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX pmc {};
    pmc.cb = sizeof(pmc);

    if (!GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc)))
    {
        ENGINE_LOG_ERROR("MemoryStats: GetProcessMemoryInfo() failed");
        return 0;
    }

    return static_cast<u64>(pmc.WorkingSetSize);
}

f64 MemoryStats::MemoryUsagePercentage()
{
    MEMORYSTATUSEX status {};
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
    {
        return 0.0;
    }

    return static_cast<f64>(status.dwMemoryLoad);
}

#endif // ENGINE_PLATFORM_WINDOWS

// ===========================================================================
// Fallback for unknown platforms
// ===========================================================================

#if !ENGINE_PLATFORM_LINUX && !ENGINE_PLATFORM_MACOS && !ENGINE_PLATFORM_WINDOWS

u64 MemoryStats::TotalPhysicalMemory()     { return 0; }
u64 MemoryStats::AvailablePhysicalMemory() { return 0; }
u64 MemoryStats::UsedPhysicalMemory()      { return 0; }
u64 MemoryStats::ProcessMemoryUsage()      { return 0; }
f64 MemoryStats::MemoryUsagePercentage()   { return 0.0; }

#endif

// ===========================================================================
// Report — platform-independent formatting
// ===========================================================================

std::string MemoryStats::Report()
{
    u64 total     = TotalPhysicalMemory();
    u64 available = AvailablePhysicalMemory();
    u64 used      = UsedPhysicalMemory();
    u64 process   = ProcessMemoryUsage();
    f64 percent   = MemoryUsagePercentage();

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    oss << "===== Memory Statistics =====\n";
    oss << "  Total Physical:    " << FormatBytes(total)     << "\n";
    oss << "  Available Physical:" << FormatBytes(available) << "\n";
    oss << "  Used Physical:     " << FormatBytes(used)      << "\n";
    oss << "  Memory Usage:      " << percent << "%\n";
    oss << "  Process RSS:       " << FormatBytes(process)   << "\n";
    oss << "============================";

    return oss.str();
}

} // namespace engine::diagnostics