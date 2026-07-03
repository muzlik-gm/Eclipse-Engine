#pragma once

/**
 * @file Platform.h
 * @brief Compile-time platform detection and identification.
 *
 * Provides preprocessor macros and constexpr namespace queries for determining
 * the target platform at compile time. Used throughout the engine to conditionally
 * compile platform-specific code paths.
 */

// ============================================================================
// Platform detection — mutually exclusive, each is 0 or 1.
// CMake may pre-define these via add_compile_definitions; the guards allow
// standalone compilation without the build system.
// ============================================================================

#ifndef ENGINE_PLATFORM_WINDOWS
#    if defined(_WIN32) || defined(_WIN64)
#        define ENGINE_PLATFORM_WINDOWS 1
#    else
#        define ENGINE_PLATFORM_WINDOWS 0
#    endif
#endif

#ifndef ENGINE_PLATFORM_LINUX
#    if defined(__linux__) && !defined(__APPLE__)
#        define ENGINE_PLATFORM_LINUX 1
#    else
#        define ENGINE_PLATFORM_LINUX 0
#    endif
#endif

#ifndef ENGINE_PLATFORM_MACOS
#    if defined(__APPLE__) && defined(__MACH__)
#        define ENGINE_PLATFORM_MACOS 1
#    else
#        define ENGINE_PLATFORM_MACOS 0
#    endif
#endif

// Human-readable platform name as a string literal.
#if ENGINE_PLATFORM_WINDOWS
#    define ENGINE_PLATFORM_NAME "Windows"
#elif ENGINE_PLATFORM_MACOS
#    define ENGINE_PLATFORM_NAME "macOS"
#elif ENGINE_PLATFORM_LINUX
#    define ENGINE_PLATFORM_NAME "Linux"
#else
#    define ENGINE_PLATFORM_NAME "Unknown"
#endif

// ============================================================================
// Architecture detection
// ============================================================================

#ifndef ENGINE_PLATFORM_64BIT
#    if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || \
        defined(_M_ARM64) || defined(__arm64__) || defined(_WIN64)
#        define ENGINE_PLATFORM_64BIT 1
#    else
#        define ENGINE_PLATFORM_64BIT 0
#    endif
#endif

// ============================================================================
// Runtime platform queries via constexpr
// ============================================================================

namespace engine::platform
{
    /// True when targeting the Windows operating system.
    constexpr bool IsWindows = (ENGINE_PLATFORM_WINDOWS == 1);

    /// True when targeting Linux (excludes Android and other Unix-like OSes).
    constexpr bool IsLinux = (ENGINE_PLATFORM_LINUX == 1);

    /// True when targeting macOS / Darwin.
    constexpr bool IsMacOS = (ENGINE_PLATFORM_MACOS == 1);

    /// True when compiling for a 64-bit instruction set.
    constexpr bool Is64Bit = (ENGINE_PLATFORM_64BIT == 1);

    /// Returns a compile-time string literal identifying the current platform.
    consteval const char* Name()
    {
        return ENGINE_PLATFORM_NAME;
    }
} // namespace engine::platform