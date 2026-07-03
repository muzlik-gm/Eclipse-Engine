#pragma once

/**
 * @file CPUInfo.h
 * @brief Static utility class for querying CPU topology and instruction-set
 *        support at runtime.
 *
 * All methods are stateless and read from OS-provided interfaces
 * (/proc/cpuinfo, /sys/... on Linux; CPUID intrinsics on x86; system info
 * APIs on macOS/Windows).
 */

#include "Engine/Core/Types.h"

#include <string>

namespace engine::threading
{

    using engine::core::u32;
    using engine::core::u64;

    /// Stateless utility for querying CPU information at runtime.
    class CPUInfo
    {
    public:
        CPUInfo() = delete;

        // ----------------------------------------------------------------
        //  Core counts
        // ----------------------------------------------------------------

        /// Returns the number of logical cores reported by the OS / hardware.
        /// Falls back to 1 if the value cannot be determined.
        [[nodiscard]] static u32 LogicalCoreCount();

        /// Returns the number of physical cores.
        /// Falls back to LogicalCoreCount() on platforms where the
        /// distinction cannot be determined.
        [[nodiscard]] static u32 PhysicalCoreCount();

        // ----------------------------------------------------------------
        //  Cache sizes
        // ----------------------------------------------------------------

        /// Returns the L1 data cache size as a human-readable string
        /// (e.g. "32 KiB").  Returns "Unknown" if the value cannot be read.
        [[nodiscard]] static std::string CacheSizeL1();

        /// Returns the L2 cache size per core as a human-readable string.
        /// Returns "Unknown" if the value cannot be read.
        [[nodiscard]] static std::string CacheSizeL2();

        /// Returns the total L3 cache size as a human-readable string.
        /// Returns "Unknown" if the value cannot be read.
        [[nodiscard]] static std::string CacheSizeL3();

        // ----------------------------------------------------------------
        //  Processor model
        // ----------------------------------------------------------------

        /// Returns the processor model name string (e.g. the "model name"
        /// field in /proc/cpuinfo on Linux).  Returns "Unknown" if
        /// unavailable.
        [[nodiscard]] static std::string ProcessorModel();

        // ----------------------------------------------------------------
        //  Instruction-set feature detection
        // ----------------------------------------------------------------

        /// Returns true if the CPU supports AVX.
        [[nodiscard]] static bool SupportsAVX();

        /// Returns true if the CPU supports AVX2.
        [[nodiscard]] static bool SupportsAVX2();

        /// Returns true if the CPU supports SSE4.2.
        [[nodiscard]] static bool SupportsSSE42();

        // ----------------------------------------------------------------
        //  Formatting
        // ----------------------------------------------------------------

        /// Returns a multi-line formatted string summarising all CPU info.
        [[nodiscard]] static std::string GetFullInfoString();
    };

} // namespace engine::threading