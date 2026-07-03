#pragma once

/**
 * @file Environment.h
 * @brief Platform-aware environment variable and process utilities.
 *
 * Wraps POSIX / Win32 environment APIs behind a portable interface and
 * provides helpers for querying the current working directory, executable
 * path, and raw command-line arguments.
 */

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <vector>
#include <fstream>

namespace engine::util
{

    // ========================================================================
    // Environment variables
    // ========================================================================

    /// Returns the value of environment variable @p name, or @p defaultValue
    /// when the variable is not set.
    [[nodiscard]] std::string GetEnvironmentVariable(
        std::string_view name,
        std::string_view defaultValue = "");

    /// Sets the environment variable @p name to @p value.
    /// Returns true on success.
    bool SetEnvironmentVariable(std::string_view name, std::string_view value);

    /// Returns true when an environment variable named @p name exists.
    [[nodiscard]] bool HasEnvironmentVariable(std::string_view name);

    /// Removes the environment variable @p name.
    /// Returns true on success.
    bool RemoveEnvironmentVariable(std::string_view name);

    // ========================================================================
    // Process / filesystem queries
    // ========================================================================

    /// Returns the current working directory as an absolute path string.
    [[nodiscard]] std::string GetCurrentWorkingDirectory();

    /// Returns the absolute path of the running executable.
    ///   - Linux:   reads /proc/self/exe
    ///   - macOS:   uses _NSGetExecutablePath
    ///   - Windows: calls GetModuleFileNameA
    [[nodiscard]] std::string GetExecutablePath();

    /// Returns the directory containing the executable (parent of
    /// GetExecutablePath()).
    [[nodiscard]] std::string GetExecutableDirectory();

    /// Returns the command-line arguments of the current process as a
    /// vector of strings (excluding argv[0]).
    ///   - Linux / macOS: reads /proc/self/cmdline
    ///   - Windows: uses GetCommandLineW + CommandLineToArgvW
    [[nodiscard]] std::vector<std::string> GetCommandLineArguments();

} // namespace engine::util

// ============================================================================
// Inline implementations — header-only because they are thin wrappers around
// platform-specific APIs declared above or in system headers.
// ============================================================================

#include "Engine/Core/Platform.h"

#if ENGINE_PLATFORM_WINDOWS
#    include <cstdlib>
#    include <windows.h>
#elif ENGINE_PLATFORM_MACOS
#    include <crt_externs.h>
#    include <mach-o/dyld.h>
#    include <sys/syslimits.h>
#    include <unistd.h>
#else // Linux / generic POSIX
#    include <climits>
#    include <unistd.h>
#endif

#include <filesystem>

namespace engine::util
{

    // ========================================================================
    // Environment variables — inline implementation
    // ========================================================================

    inline std::string GetEnvironmentVariable(std::string_view name, std::string_view defaultValue)
    {
        // std::getenv requires a null-terminated string.
        std::string nullTerminatedName(name);
        const char* value = std::getenv(nullTerminatedName.c_str());
        if (value != nullptr)
        {
            return std::string(value);
        }
        return std::string(defaultValue);
    }

    inline bool SetEnvironmentVariable(std::string_view name, std::string_view value)
    {
        std::string nullTerminatedName(name);
        std::string nullTerminatedValue(value);

#if ENGINE_PLATFORM_WINDOWS
        return _putenv_s(nullTerminatedName.c_str(), nullTerminatedValue.c_str()) == 0;
#else
        // POSIX setenv: the value is copied, so temporary strings are fine.
        return setenv(nullTerminatedName.c_str(), nullTerminatedValue.c_str(), /* overwrite */ 1) == 0;
#endif
    }

    inline bool HasEnvironmentVariable(std::string_view name)
    {
        std::string nullTerminatedName(name);
        return std::getenv(nullTerminatedName.c_str()) != nullptr;
    }

    inline bool RemoveEnvironmentVariable(std::string_view name)
    {
        std::string nullTerminatedName(name);

#if ENGINE_PLATFORM_WINDOWS
        return _putenv_s(nullTerminatedName.c_str(), "") == 0;
#else
        return unsetenv(nullTerminatedName.c_str()) == 0;
#endif
    }

    // ========================================================================
    // Process / filesystem queries — inline implementation
    // ========================================================================

    inline std::string GetCurrentWorkingDirectory()
    {
        return std::filesystem::current_path().string();
    }

    inline std::string GetExecutablePath()
    {
#if ENGINE_PLATFORM_WINDOWS
        char buffer[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
        if (len == 0 || len >= sizeof(buffer))
        {
            return "";
        }
        return std::string(buffer, len);
#elif ENGINE_PLATFORM_MACOS
        char buffer[PATH_MAX];
        u32 bufferSize = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &bufferSize) != 0)
        {
            // Buffer too small — allocate and retry.
            std::vector<char> dynamicBuffer(bufferSize);
            if (_NSGetExecutablePath(dynamicBuffer.data(), &bufferSize) != 0)
            {
                return "";
            }
            return std::string(dynamicBuffer.data());
        }
        return std::string(buffer);
#else // Linux / generic POSIX
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len == -1)
        {
            return "";
        }
        buffer[len] = '\0';
        return std::string(buffer, static_cast<usize>(len));
#endif
    }

    inline std::string GetExecutableDirectory()
    {
        std::string exePath = GetExecutablePath();
        if (exePath.empty())
        {
            return "";
        }
        std::filesystem::path p(exePath);
        return p.parent_path().string();
    }

    inline std::vector<std::string> GetCommandLineArguments()
    {
        std::vector<std::string> result;

#if ENGINE_PLATFORM_WINDOWS
        int argc = 0;
        LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argvW != nullptr)
        {
            // Skip argv[0] (the executable name).
            for (int i = 1; i < argc; ++i)
            {
                int wideLen = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1,
                    nullptr, 0, nullptr, nullptr);
                if (wideLen > 0)
                {
                    std::string arg;
                    arg.resize(static_cast<usize>(wideLen - 1)); // -1 to drop null terminator
                    WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1,
                        arg.data(), wideLen, nullptr, nullptr);
                    result.push_back(std::move(arg));
                }
            }
            LocalFree(argvW);
        }
#else // Linux / macOS — read /proc/self/cmdline
        std::ifstream cmdline("/proc/self/cmdline", std::ios::binary);
        if (!cmdline.is_open())
        {
            return result;
        }
        std::string token;
        bool isFirst = true;
        while (std::getline(cmdline, token, '\0'))
        {
            if (isFirst)
            {
                isFirst = false; // skip argv[0]
                continue;
            }
            result.push_back(std::move(token));
        }
#endif
        return result;
    }

} // namespace engine::util