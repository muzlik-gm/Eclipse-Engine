/**
 * @file SystemInfo.cpp
 * @brief Platform-specific system information implementation.
 */

#include "Engine/Diagnostics/SystemInfo.h"

#include "Engine/Core/Platform.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <thread>

// ===========================================================================
// Platform includes
// ===========================================================================

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <Lmcons.h>
#elif ENGINE_PLATFORM_LINUX
    #include <cstring>
    #include <unistd.h>
    #include <sys/utsname.h>
#elif ENGINE_PLATFORM_MACOS
    #include <cstring>
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

namespace engine::diagnostics
{

using engine::core::u32;
using engine::core::u64;
using engine::core::f64;
using engine::core::usize;

// ===========================================================================
// Helpers (file-scope)
// ===========================================================================

namespace
{

using engine::core::u64;

/// @brief Formats a byte count as a human-readable string with binary units.
std::string FormatBytes(u64 bytes)
{
    constexpr u64 kKiB = 1024ULL;
    constexpr u64 kMiB = 1024ULL * kKiB;
    constexpr u64 kGiB = 1024ULL * kMiB;
    constexpr u64 kTiB = 1024ULL * kGiB;

    std::ostringstream oss;
    oss << bytes << " bytes";

    if (bytes >= kTiB)
    {
        oss << " (" << std::fixed << std::setprecision(2)
            << static_cast<f64>(bytes) / static_cast<f64>(kTiB) << " TiB)";
    }
    else if (bytes >= kGiB)
    {
        oss << " (" << std::fixed << std::setprecision(2)
            << static_cast<f64>(bytes) / static_cast<f64>(kGiB) << " GiB)";
    }
    else if (bytes >= kMiB)
    {
        oss << " (" << std::fixed << std::setprecision(2)
            << static_cast<f64>(bytes) / static_cast<f64>(kMiB) << " MiB)";
    }
    else if (bytes >= kKiB)
    {
        oss << " (" << std::fixed << std::setprecision(2)
            << static_cast<f64>(bytes) / static_cast<f64>(kKiB) << " KiB)";
    }

    return oss.str();
}

#if ENGINE_PLATFORM_LINUX
/// @brief Parses a named field from /proc/meminfo (e.g. "MemTotal", "MemAvailable").
///        Returns 0 on failure.
u64 ParseMeminfoField(const char* fieldName)
{
    FILE* file = std::fopen("/proc/meminfo", "r");
    if (!file)
    {
        return 0;
    }

    u64 result       = 0;
    bool found       = false;
    char line[256]   = {};

    const usize nameLen = std::strlen(fieldName);

    while (std::fgets(line, sizeof(line), file) != nullptr)
    {
        if (std::strncmp(line, fieldName, nameLen) == 0)
        {
            // Find the first digit in the line (value follows the colon).
            char* ptr = line + nameLen;
            while (*ptr != '\0' && !std::isdigit(static_cast<unsigned char>(*ptr)))
            {
                ++ptr;
            }

            if (*ptr != '\0')
            {
                result = static_cast<u64>(std::strtoull(ptr, nullptr, 10));
                // /proc/meminfo reports in KiB — convert to bytes.
                result *= 1024ULL;
                found = true;
            }
            break;
        }
    }

    std::fclose(file);
    return found ? result : 0;
}
#endif // ENGINE_PLATFORM_LINUX

} // anonymous namespace

// ===========================================================================
// SystemInfo
// ===========================================================================

std::string SystemInfo::HostName()
{
    std::string result;

#if ENGINE_PLATFORM_WINDOWS
    char buffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD size = static_cast<DWORD>(sizeof(buffer));
    if (::GetComputerNameA(buffer, &size) != 0)
    {
        result = buffer;
    }
    else
    {
        result = "unknown";
    }
#elif ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    char buffer[256] = {};
    if (::gethostname(buffer, sizeof(buffer)) == 0)
    {
        buffer[sizeof(buffer) - 1] = '\0';
        result = buffer;
    }
    else
    {
        result = "unknown";
    }
#else
    result = "unknown";
#endif

    return result;
}

std::string SystemInfo::UserName()
{
    std::string result;

#if ENGINE_PLATFORM_WINDOWS
    char buffer[UNLEN + 1] = {};
    DWORD size = static_cast<DWORD>(sizeof(buffer));
    if (::GetUserNameA(buffer, &size) != 0)
    {
        result = buffer;
    }
    else
    {
        result = "unknown";
    }
#elif ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    // getlogin() may fail in non-interactive contexts (e.g. services).
    const char* login = ::getlogin();
    if (login != nullptr)
    {
        result = login;
    }
    else
    {
        const char* envUser = std::getenv("USER");
        if (envUser != nullptr)
        {
            result = envUser;
        }
        else
        {
            result = "unknown";
        }
    }
#else
    result = "unknown";
#endif

    return result;
}

u32 SystemInfo::LogicalCoreCount()
{
    // std::thread::hardware_concurrency returns 0 when the value cannot be
    // determined, in which case we fall back to 1.
    u32 cores = std::thread::hardware_concurrency();
    return cores > 0 ? cores : 1;
}

u64 SystemInfo::TotalPhysicalMemoryBytes()
{
#if ENGINE_PLATFORM_WINDOWS
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    if (::GlobalMemoryStatusEx(&status) != 0)
    {
        return static_cast<u64>(status.ullTotalPhys);
    }
    return 0;

#elif ENGINE_PLATFORM_MACOS
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    u64 memBytes = 0;
    usize len = sizeof(memBytes);
    if (::sysctl(mib, 2, &memBytes, &len, nullptr, 0) == 0)
    {
        return memBytes;
    }
    return 0;

#elif ENGINE_PLATFORM_LINUX
    return ParseMeminfoField("MemTotal:");

#else
    return 0;
#endif
}

u64 SystemInfo::AvailablePhysicalMemoryBytes()
{
#if ENGINE_PLATFORM_WINDOWS
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    if (::GlobalMemoryStatusEx(&status) != 0)
    {
        return static_cast<u64>(status.ullAvailPhys);
    }
    return 0;

#elif ENGINE_PLATFORM_MACOS
    // hw.usermem gives usable memory (total minus kernel overhead).
    int mib[2] = {CTL_HW, HW_USERMEM};
    u64 memBytes = 0;
    usize len = sizeof(memBytes);
    if (::sysctl(mib, 2, &memBytes, &len, nullptr, 0) == 0)
    {
        return memBytes;
    }
    return 0;

#elif ENGINE_PLATFORM_LINUX
    // MemAvailable is the preferred field (kernel ≥ 3.14).  Fall back to
    // MemFree + Buffers + Cached for older kernels.
    u64 available = ParseMeminfoField("MemAvailable:");
    if (available == 0)
    {
        const u64 memFree  = ParseMeminfoField("MemFree:");
        const u64 buffers  = ParseMeminfoField("Buffers:");
        const u64 cached   = ParseMeminfoField("Cached:");
        available = memFree + buffers + cached;
    }
    return available;

#else
    return 0;
#endif
}

std::string SystemInfo::OSName()
{
    std::string result;

#if ENGINE_PLATFORM_WINDOWS
    // Use RtlGetVersion for accurate version info even on Windows 10+.
    // We avoid GetVersionEx because it may lie due to the compatibility manifest.
    typedef LONG (NTAPI* RtlGetVersionFn)(OSVERSIONINFOEXW*);

    HMODULE ntdll = ::GetModuleHandleA("ntdll.dll");
    if (ntdll != nullptr)
    {
        auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
            ::GetProcAddress(ntdll, "RtlGetVersion"));

        if (rtlGetVersion != nullptr)
        {
            OSVERSIONINFOEXW vi = {};
            vi.dwOSVersionInfoSize = sizeof(vi);
            if (rtlGetVersion(&vi) == 0)
            {
                std::ostringstream oss;
                oss << "Windows " << vi.dwMajorVersion << "." << vi.dwMinorVersion
                    << " (Build " << vi.dwBuildNumber << ")";
                result = oss.str();
            }
        }
    }

    if (result.empty())
    {
        result = "Windows (unknown version)";
    }

#elif ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    struct utsname info = {};
    if (::uname(&info) == 0)
    {
        std::ostringstream oss;
        oss << info.sysname << " " << info.release;
        result = oss.str();
    }
    else
    {
        result = "Unix (unknown version)";
    }
#else
    result = "Unknown OS";
#endif

    return result;
}

std::string SystemInfo::Architecture()
{
    // These preprocessor defines are set by the compiler for the target arch.
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64__)
    return "aarch64";
#elif defined(__i386__) || defined(_M_IX86) || defined(_X86_)
    return "x86";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#elif defined(__riscv)
    #if __riscv_xlen == 64
        return "riscv64";
    #elif __riscv_xlen == 32
        return "riscv32";
    #else
        return "riscv";
    #endif
#elif defined(__powerpc64__)
    return "ppc64";
#elif defined(__powerpc__)
    return "ppc";
#elif defined(__s390x__)
    return "s390x";
#elif defined(__s390__)
    return "s390";
#elif defined(__mips64)
    return "mips64";
#elif defined(__mips__)
    return "mips";
#elif defined(__sparc__) && defined(__arch64__)
    return "sparc64";
#elif defined(__sparc__)
    return "sparc";
#elif defined(__e2k__)
    return "e2k";
#else
    return "unknown";
#endif
}

std::string SystemInfo::GetFullSystemInfoString()
{
    std::ostringstream oss;
    oss << "Host:     " << HostName() << "\n";
    oss << "User:     " << UserName() << "\n";
    oss << "OS:       " << OSName() << "\n";
    oss << "Arch:     " << Architecture() << "\n";
    oss << "CPU Cores:" << LogicalCoreCount() << "\n";
    oss << "Total Mem:" << FormatBytes(TotalPhysicalMemoryBytes()) << "\n";
    oss << "Avail Mem:" << FormatBytes(AvailablePhysicalMemoryBytes());
    return oss.str();
}

} // namespace engine::diagnostics