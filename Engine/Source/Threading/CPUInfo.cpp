#include "Engine/Threading/CPUInfo.h"

#include <memory>
#include <sstream>
#include <thread>

// ============================================================================
//  Platform includes
// ============================================================================

#if ENGINE_PLATFORM_LINUX
    #include <fstream>
    #include <numeric>
    #include <unistd.h>
#endif

#if ENGINE_PLATFORM_MACOS
    #include <sys/sysctl.h>
#endif

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
#endif

// ============================================================================
//  x86 CPUID intrinsics
// ============================================================================

#if (ENGINE_COMPILER_MSVC || ENGINE_COMPILER_GCC || ENGINE_COMPILER_CLANG) && \
    (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86))
    #define ENGINE_HAS_CPUID 1
#else
    #define ENGINE_HAS_CPUID 0
#endif

#if ENGINE_HAS_CPUID
    #if ENGINE_COMPILER_MSVC
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
#endif

namespace engine::threading
{

    using engine::core::i32;
    using engine::core::u32;
    using engine::core::u64;
    using engine::core::u8;
    using engine::core::usize;

    // ========================================================================
    //  Helpers (Linux only, file-local)
    // ========================================================================

#if ENGINE_PLATFORM_LINUX

    /// Reads the first matching value for a key in /proc/cpuinfo.
    /// Returns an empty string on failure.
    static std::string ReadCpuinfoField(const std::string& field)
    {
        std::ifstream ifs("/proc/cpuinfo");
        if (!ifs.good())
        {
            return {};
        }

        const std::string prefix = field + " : ";
        std::string line;
        while (std::getline(ifs, line))
        {
            if (line.find(prefix) == 0)
            {
                return line.substr(prefix.size());
            }
        }

        return {};
    }

    /// Reads a sysfs file that contains a single integer value.
    /// Returns @p fallback on failure.
    static u64 ReadSysfsU64(const std::string& path, u64 fallback = 0)
    {
        std::ifstream ifs(path);
        if (!ifs.good())
        {
            return fallback;
        }

        u64 value = fallback;
        ifs >> value;
        return value;
    }

    /// Formats a byte count into a human-readable string (e.g. "32 KiB").
    static std::string FormatByteSize(u64 bytes)
    {
        constexpr u64 kKiB = 1024ULL;
        constexpr u64 kMiB = 1024ULL * 1024ULL;

        if (bytes >= kMiB)
        {
            const auto mib = static_cast<double>(bytes) / static_cast<double>(kMiB);
            std::ostringstream oss;
            oss << mib << " MiB";
            return oss.str();
        }
        if (bytes >= kKiB)
        {
            const auto kib = static_cast<double>(bytes) / static_cast<double>(kKiB);
            std::ostringstream oss;
            oss << kib << " KiB";
            return oss.str();
        }

        std::ostringstream oss;
        oss << bytes << " B";
        return oss.str();
    }

#endif // ENGINE_PLATFORM_LINUX

    // ========================================================================
    //  CPUID helpers (x86)
    // ========================================================================

#if ENGINE_HAS_CPUID
    /// Executes the CPUID instruction with leaf @p eax and returns results
    /// in the output parameters.
    static void RunCpuid(u32 eax, u32 ecx, u32& outEax, u32& outEbx,
                         u32& outEcx, u32& outEdx)
    {
        i32 cpuInfo[4] = {};
#if ENGINE_COMPILER_MSVC
        __cpuidex(cpuInfo, static_cast<i32>(eax), static_cast<i32>(ecx));
#else
        __cpuid_count(static_cast<u32>(eax), static_cast<u32>(ecx),
                      cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif
        outEax = static_cast<u32>(cpuInfo[0]);
        outEbx = static_cast<u32>(cpuInfo[1]);
        outEcx = static_cast<u32>(cpuInfo[2]);
        outEdx = static_cast<u32>(cpuInfo[3]);
    }

    /// Returns the maximum supported CPUID leaf.
    static u32 MaxCpuidLeaf()
    {
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        RunCpuid(0, 0, eax, ebx, ecx, edx);
        return eax;
    }
#endif // ENGINE_HAS_CPUID

    // ========================================================================
    //  Core counts
    // ========================================================================

    u32 CPUInfo::LogicalCoreCount()
    {
        const auto cores = std::thread::hardware_concurrency();
        return cores > 0 ? static_cast<u32>(cores) : 1;
    }

    u32 CPUInfo::PhysicalCoreCount()
    {
#if ENGINE_PLATFORM_LINUX
        // On Linux, parse "cpu cores" from /proc/cpuinfo (physical cores
        // per socket).
        const std::string coresStr = ReadCpuinfoField("cpu cores");
        if (!coresStr.empty())
        {
            const u32 cores = static_cast<u32>(std::stoul(coresStr));
            if (cores > 0)
            {
                return cores;
            }
        }

        // Fallback: try sysconf.
        const long count = ::sysconf(_SC_NPROCESSORS_ONLN);
        if (count > 0)
        {
            return static_cast<u32>(count);
        }

        return LogicalCoreCount();

#elif ENGINE_PLATFORM_MACOS
        i32 physicalCores = 0;
        usize len = sizeof(physicalCores);
        if (::sysctlbyname("hw.physicalcpu", &physicalCores, &len, nullptr, 0) == 0
            && physicalCores > 0)
        {
            return static_cast<u32>(physicalCores);
        }
        return LogicalCoreCount();

#elif ENGINE_PLATFORM_WINDOWS
        // Use GetLogicalProcessorInformationEx to count physical cores.
        DWORD len = 0;
        ::GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);

        if (len == 0)
        {
            return LogicalCoreCount();
        }

        auto buffer = std::make_unique<u8[]>(len);
        if (!::GetLogicalProcessorInformationEx(
                RelationProcessorCore,
                reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.get()),
                &len))
        {
            return LogicalCoreCount();
        }

        u32 count = 0;
        usize offset = 0;
        while (offset < len)
        {
            auto* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(
                buffer.get() + offset);
            if (info->Relationship == RelationProcessorCore)
            {
                // Each ProcessorCore entry represents one physical core
                // (which may contain multiple logical processors).
                ++count;
            }
            offset += info->Size;
        }

        return count > 0 ? count : LogicalCoreCount();

#else
        return LogicalCoreCount();
#endif
    }

    // ========================================================================
    //  Cache sizes
    // ========================================================================

    std::string CPUInfo::CacheSizeL1()
    {
#if ENGINE_PLATFORM_LINUX
        // On x86 Linux, try /sys/devices/system/cpu/cpu0/cache/index0/size
        // (index0 = L1 data, index1 = L1 instruction).
        const u64 size = ReadSysfsU64(
            "/sys/devices/system/cpu/cpu0/cache/index0/size", 0);
        if (size > 0)
        {
            return FormatByteSize(size);
        }
        return "Unknown";

#elif ENGINE_PLATFORM_MACOS
        // macOS does not expose per-cache-level sizes via sysctl.
        // We can use CPUID on x86 Macs, or just return "Unknown" on Apple Silicon.
    #if ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        if (MaxCpuidLeaf() >= 4)
        {
            RunCpuid(4, 0, eax, ebx, ecx, edx);
            // Bits 31-22 of EAX = (cache line size * associativity * partitions)
            // times (EBX + 1) = cache size in bytes.
            u32 ways  = ((eax >> 22) & 0x3FF) + 1;
            u32 parts = ((eax >> 12) & 0x3FF) + 1;
            u32 lineSize = (ebx & 0xFFF) + 1;
            u32 sets  = ecx + 1;
            u64 totalSize = static_cast<u64>(ways) * parts * lineSize * sets;
            return FormatByteSize(totalSize);
        }
    #endif
        return "Unknown";

#elif ENGINE_PLATFORM_WINDOWS
    #if ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        if (MaxCpuidLeaf() >= 4)
        {
            // CPUID leaf 4, sub-leaf 0 typically returns L1 data cache info.
            RunCpuid(4, 0, eax, ebx, ecx, edx);
            u32 cacheLevel = (eax >> 5) & 0x7;
            if (cacheLevel == 1)
            {
                u32 ways     = ((eax >> 22) & 0x3FF) + 1;
                u32 parts    = ((eax >> 12) & 0x3FF) + 1;
                u32 lineSize = (ebx & 0xFFF) + 1;
                u32 sets     = ecx + 1;
                u64 totalSize = static_cast<u64>(ways) * parts * lineSize * sets;
                return FormatByteSize(totalSize);
            }
        }
    #endif
        return "Unknown";

#else
        return "Unknown";
#endif
    }

    std::string CPUInfo::CacheSizeL2()
    {
#if ENGINE_PLATFORM_LINUX
        const u64 size = ReadSysfsU64(
            "/sys/devices/system/cpu/cpu0/cache/index2/size", 0);
        if (size > 0)
        {
            return FormatByteSize(size);
        }
        return "Unknown";

#elif ENGINE_PLATFORM_MACOS || ENGINE_PLATFORM_WINDOWS
    #if ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        if (MaxCpuidLeaf() >= 4)
        {
            // Iterate sub-leaves to find L2.
            for (u32 subleaf = 0; subleaf < 16; ++subleaf)
            {
                RunCpuid(4, subleaf, eax, ebx, ecx, edx);
                u32 cacheLevel = (eax >> 5) & 0x7;
                u32 cacheType  = eax & 0x1F;

                // cacheType 2 = unified, 1 = data.
                if (cacheLevel == 2 && (cacheType == 1 || cacheType == 2))
                {
                    u32 ways     = ((eax >> 22) & 0x3FF) + 1;
                    u32 parts    = ((eax >> 12) & 0x3FF) + 1;
                    u32 lineSize = (ebx & 0xFFF) + 1;
                    u32 sets     = ecx + 1;
                    u64 totalSize = static_cast<u64>(ways) * parts * lineSize * sets;
                    return FormatByteSize(totalSize);
                }

                // If the cache type is 0, no more caches.
                if (cacheType == 0)
                {
                    break;
                }
            }
        }
    #endif
        return "Unknown";

#else
        return "Unknown";
#endif
    }

    std::string CPUInfo::CacheSizeL3()
    {
#if ENGINE_PLATFORM_LINUX
        const u64 size = ReadSysfsU64(
            "/sys/devices/system/cpu/cpu0/cache/index3/size", 0);
        if (size > 0)
        {
            return FormatByteSize(size);
        }
        return "Unknown";

#elif ENGINE_PLATFORM_MACOS || ENGINE_PLATFORM_WINDOWS
    #if ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        if (MaxCpuidLeaf() >= 4)
        {
            for (u32 subleaf = 0; subleaf < 16; ++subleaf)
            {
                RunCpuid(4, subleaf, eax, ebx, ecx, edx);
                u32 cacheLevel = (eax >> 5) & 0x7;
                u32 cacheType  = eax & 0x1F;

                if (cacheLevel == 3 && (cacheType == 1 || cacheType == 2 || cacheType == 3))
                {
                    u32 ways     = ((eax >> 22) & 0x3FF) + 1;
                    u32 parts    = ((eax >> 12) & 0x3FF) + 1;
                    u32 lineSize = (ebx & 0xFFF) + 1;
                    u32 sets     = ecx + 1;
                    u64 totalSize = static_cast<u64>(ways) * parts * lineSize * sets;
                    return FormatByteSize(totalSize);
                }

                if (cacheType == 0)
                {
                    break;
                }
            }
        }
    #endif
        return "Unknown";

#else
        return "Unknown";
#endif
    }

    // ========================================================================
    //  Processor model
    // ========================================================================

    std::string CPUInfo::ProcessorModel()
    {
#if ENGINE_PLATFORM_LINUX
        const std::string model = ReadCpuinfoField("model name");
        if (!model.empty())
        {
            return model;
        }
        return "Unknown";

#elif ENGINE_PLATFORM_MACOS
        // On macOS we can use sysctl "machdep.cpu.brand_string" on x86.
        char buffer[256] = {};
        usize bufLen = sizeof(buffer);
        if (::sysctlbyname("machdep.cpu.brand_string", buffer, &bufLen, nullptr, 0) == 0
            && bufLen > 0)
        {
            return std::string(buffer);
        }

        // On Apple Silicon, try hw.machine.
        bufLen = sizeof(buffer);
        if (::sysctlbyname("hw.machine", buffer, &bufLen, nullptr, 0) == 0
            && bufLen > 0)
        {
            return std::string(buffer);
        }

        return "Unknown";

#elif ENGINE_PLATFORM_WINDOWS
    #if ENGINE_HAS_CPUID
        // Use CPUID extended leaf 0x80000000 to check max extended leaf,
        // then leaf 0x80000004 for the processor brand string.
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        RunCpuid(0x80000000, 0, eax, ebx, ecx, edx);
        if (eax >= 0x80000004)
        {
            char brand[49] = {};
            u32* parts = reinterpret_cast<u32*>(brand);

            RunCpuid(0x80000002, 0, parts[0], parts[1], parts[2], parts[3]);
            RunCpuid(0x80000003, 0, parts[4], parts[5], parts[6], parts[7]);
            RunCpuid(0x80000004, 0, parts[8], parts[9], parts[10], parts[11]);

            std::string result(brand);
            // Trim trailing whitespace.
            while (!result.empty() && (result.back() == ' ' || result.back() == '\0'))
            {
                result.pop_back();
            }
            return result;
        }
    #endif
        return "Unknown";

#else
        return "Unknown";
#endif
    }

    // ========================================================================
    //  Instruction-set feature detection
    // ========================================================================

    bool CPUInfo::SupportsAVX()
    {
#if ENGINE_COMPILER_GCC || ENGINE_COMPILER_CLANG
        return __builtin_cpu_supports("avx") != 0;
#elif ENGINE_COMPILER_MSVC && ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        RunCpuid(1, 0, eax, ebx, ecx, edx);
        // Check both the CPUID bit and the OS-enabled bit.
        // Bit 28 of ECX = AVX; XGETBV bit 1 = OS has enabled XMM/YMM state.
        bool cpuSupportsAVX = (ecx & (1u << 28)) != 0;
        if (!cpuSupportsAVX)
        {
            return false;
        }
        // Check OS support via XGETBV.
        u64 xcr0 = 0;
#if defined(_XCR_XFEATURE_ENABLED_MASK)
        xcr0 = static_cast<u64>(::_xgetbv(_XCR_XFEATURE_ENABLED_MASK));
#endif
        return (xcr0 & 0x6ULL) == 0x6ULL;
#else
        return false;
#endif
    }

    bool CPUInfo::SupportsAVX2()
    {
#if ENGINE_COMPILER_GCC || ENGINE_COMPILER_CLANG
        return __builtin_cpu_supports("avx2") != 0;
#elif ENGINE_COMPILER_MSVC && ENGINE_HAS_CPUID
        // AVX2 requires AVX support first (OS must have enabled it).
        if (!SupportsAVX())
        {
            return false;
        }
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        RunCpuid(7, 0, eax, ebx, ecx, edx);
        return (ebx & (1u << 5)) != 0;
#else
        return false;
#endif
    }

    bool CPUInfo::SupportsSSE42()
    {
#if ENGINE_COMPILER_GCC || ENGINE_COMPILER_CLANG
        return __builtin_cpu_supports("sse4.2") != 0;
#elif ENGINE_COMPILER_MSVC && ENGINE_HAS_CPUID
        u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
        RunCpuid(1, 0, eax, ebx, ecx, edx);
        return (ecx & (1u << 20)) != 0;
#else
        return false;
#endif
    }

    // ========================================================================
    //  Formatting
    // ========================================================================

    std::string CPUInfo::GetFullInfoString()
    {
        std::ostringstream oss;
        oss << "=== CPU Information ===\n";
        oss << "  Processor        : " << ProcessorModel() << "\n";
        oss << "  Logical Cores    : " << LogicalCoreCount() << "\n";
        oss << "  Physical Cores   : " << PhysicalCoreCount() << "\n";
        oss << "  L1 Cache         : " << CacheSizeL1() << "\n";
        oss << "  L2 Cache         : " << CacheSizeL2() << "\n";
        oss << "  L3 Cache         : " << CacheSizeL3() << "\n";
        oss << "  SSE 4.2          : " << (SupportsSSE42() ? "Yes" : "No") << "\n";
        oss << "  AVX              : " << (SupportsAVX()   ? "Yes" : "No") << "\n";
        oss << "  AVX2             : " << (SupportsAVX2()  ? "Yes" : "No") << "\n";
        return oss.str();
    }

} // namespace engine::threading