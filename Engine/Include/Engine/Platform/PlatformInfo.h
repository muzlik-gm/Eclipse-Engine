// ============================================================================
// File: Engine/Include/Engine/Platform/PlatformInfo.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <string>

namespace engine::platform {

    // ========================================================================
    // PlatformInfo – snapshot of host platform data + static query helpers
    // ========================================================================
    struct PlatformInfo
    {
        std::string OSName;
        std::string OSVersion;
        std::string Architecture;
        std::string PlatformName;
        bool        Is64BitPlatform = false;

        /// Returns the name of the operating system (e.g. "Ubuntu 24.04", "Windows 10", "macOS").
        [[nodiscard]] static std::string GetOSName();

        /// Returns the OS version string (e.g. "6.5.0-44-generic", "10.0.22631", "14.5").
        [[nodiscard]] static std::string GetOSVersion();

        /// Returns the CPU architecture string (e.g. "x86_64", "aarch64").
        [[nodiscard]] static std::string GetArchitecture();

        /// Returns the canonical platform name: "Linux", "Windows", or "macOS".
        [[nodiscard]] static std::string GetPlatformName();

        /// Returns a formatted string combining platform, OS, and architecture
        /// (e.g. "Linux x86_64 | Ubuntu 24.04").
        [[nodiscard]] static std::string GetEnginePlatformString();

        /// Returns true if the process is running on a 64-bit architecture.
        [[nodiscard]] static bool Is64Bit();

        /// Populates and returns a fully resolved PlatformInfo snapshot.
        [[nodiscard]] static PlatformInfo Gather();
    };

} // namespace engine::platform