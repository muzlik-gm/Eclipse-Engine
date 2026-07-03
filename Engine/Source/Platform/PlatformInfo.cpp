// ============================================================================
// File: Engine/Source/Platform/PlatformInfo.cpp
// Implementation of PlatformInfo static queries.
// ============================================================================

#include "Engine/Platform/PlatformInfo.h"
#include "Engine/Diagnostics/SystemInfo.h"

#include <cstdio>
#include <string>

namespace engine::platform
{

    std::string PlatformInfo::GetOSName()
    {
        return diagnostics::SystemInfo::OSName();
    }

    std::string PlatformInfo::GetOSVersion()
    {
#if defined(ENGINE_PLATFORM_LINUX)
        FILE* f = std::fopen("/proc/version", "r");
        if (f)
        {
            char buf[256] = {};
            if (std::fgets(buf, sizeof(buf), f))
            {
                std::fclose(f);
                std::string str(buf);
                auto pos = str.find("Linux version ");
                if (pos != std::string::npos)
                {
                    auto start = pos + 15;
                    auto end = str.find(' ', start);
                    if (end != std::string::npos)
                    {
                        return str.substr(start, end - start);
                    }
                }
                return str;
            }
            std::fclose(f);
        }
        return "Unknown";
#elif defined(ENGINE_PLATFORM_MACOS)
        return diagnostics::SystemInfo::OSName();
#elif defined(ENGINE_PLATFORM_WINDOWS)
        return diagnostics::SystemInfo::OSName();
#else
        return "Unknown";
#endif
    }

    std::string PlatformInfo::GetArchitecture()
    {
        return diagnostics::SystemInfo::Architecture();
    }

    std::string PlatformInfo::GetPlatformName()
    {
#if defined(ENGINE_PLATFORM_LINUX)
        return "Linux";
#elif defined(ENGINE_PLATFORM_MACOS)
        return "macOS";
#elif defined(ENGINE_PLATFORM_WINDOWS)
        return "Windows";
#else
        return "Unknown";
#endif
    }

    std::string PlatformInfo::GetEnginePlatformString()
    {
        return GetPlatformName() + " " + GetArchitecture() + " | " + GetOSName();
    }

    bool PlatformInfo::Is64Bit()
    {
        return sizeof(void*) == 8;
    }

    PlatformInfo PlatformInfo::Gather()
    {
        PlatformInfo info;
        info.OSName         = GetOSName();
        info.OSVersion      = GetOSVersion();
        info.Architecture    = GetArchitecture();
        info.PlatformName    = GetPlatformName();
        info.Is64BitPlatform = Is64Bit();
        return info;
    }

} // namespace engine::platform
