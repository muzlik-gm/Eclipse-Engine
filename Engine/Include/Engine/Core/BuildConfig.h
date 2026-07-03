#pragma once

/**
 * @file BuildConfig.h
 * @brief Compile-time build configuration flags and the FORCE_INLINE macro.
 *
 * Centralises all build-variant toggles (debug/release, assertions, profiling,
 * logging) so that the rest of the engine never has to probe preprocessor
 * state directly.
 */

#include "Engine/Core/Platform.h"
#include "Engine/Core/Compiler.h"

// ============================================================================
// Build variant — debug vs. release
// ============================================================================

#ifndef ENGINE_DEBUG
#    if defined(NDEBUG) || defined(ENGINE_RELEASE)
#        define ENGINE_DEBUG 0
#    else
#        define ENGINE_DEBUG 1
#    endif
#endif

#ifndef ENGINE_RELEASE
#    define ENGINE_RELEASE (!ENGINE_DEBUG)
#endif

// ============================================================================
// Feature toggles — default to enabled unless explicitly disabled.
// CMake passes these via add_compile_definitions when the corresponding
// option() is ON.  When compiling without CMake, the defaults below apply.
// ============================================================================

#ifndef ENGINE_ENABLE_ASSERTIONS
#    define ENGINE_ENABLE_ASSERTIONS 1
#endif

#ifndef ENGINE_ENABLE_PROFILING
#    define ENGINE_ENABLE_PROFILING 1
#endif

#ifndef ENGINE_ENABLE_LOGGING
#    define ENGINE_ENABLE_LOGGING 1
#endif

// ============================================================================
// FORCE_INLINE — aggressively inline a function for maximum performance.
// On MSVC the compiler-specific __forceinline is used; elsewhere the
// standard [[nodiscard]] inline is applied.
// ============================================================================

#if ENGINE_COMPILER_MSVC
#    define FORCE_INLINE [[nodiscard]] __forceinline
#else
#    define FORCE_INLINE [[nodiscard]] inline
#endif

// ============================================================================
// ALWAYS_INLINE — same as FORCE_INLINE but without [[nodiscard]].
// Use this for void-returning performance helpers.
// ============================================================================

#if ENGINE_COMPILER_MSVC
#    define ALWAYS_INLINE __forceinline
#else
#    define ALWAYS_INLINE inline
#endif

// ============================================================================
// NO_INLINE — prevent inlining (e.g. to keep a function symbol observable
// for profiling, or to avoid code bloat in hot loops).
// ============================================================================

#if ENGINE_COMPILER_MSVC
#    define NO_INLINE __declspec(noinline)
#else
#    define NO_INLINE __attribute__((noinline))
#endif

// ============================================================================
// Compile-assist macros
// ============================================================================

/// Marks an enum class as using its underlying type as a bitmask.
#define ENGINE_ENUM_FLAGS(T)                                                   \
    static_assert(std::is_enum_v<T>, "ENGINE_ENUM_FLAGS requires an enum");    \
    constexpr inline T operator|(T a, T b) noexcept                           \
    {                                                                          \
        using U = std::underlying_type_t<T>;                                   \
        return static_cast<T>(static_cast<U>(a) | static_cast<U>(b));          \
    }                                                                          \
    constexpr inline T operator&(T a, T b) noexcept                           \
    {                                                                          \
        using U = std::underlying_type_t<T>;                                   \
        return static_cast<T>(static_cast<U>(a) & static_cast<U>(b));          \
    }                                                                          \
    constexpr inline T operator^(T a, T b) noexcept                           \
    {                                                                          \
        using U = std::underlying_type_t<T>;                                   \
        return static_cast<T>(static_cast<U>(a) ^ static_cast<U>(b));          \
    }                                                                          \
    constexpr inline T operator~(T a) noexcept                                \
    {                                                                          \
        using U = std::underlying_type_t<T>;                                   \
        return static_cast<T>(~static_cast<U>(a));                             \
    }                                                                          \
    constexpr inline T& operator|=(T& a, T b) noexcept                        \
    {                                                                          \
        return a = a | b;                                                     \
    }                                                                          \
    constexpr inline T& operator&=(T& a, T b) noexcept                        \
    {                                                                          \
        return a = a & b;                                                     \
    }                                                                          \
    constexpr inline T& operator^=(T& a, T b) noexcept                        \
    {                                                                          \
        return a = a ^ b;                                                     \
    }

// ============================================================================
// Runtime build-configuration queries via constexpr
// ============================================================================

namespace engine::buildconfig
{
    /// True when the engine is compiled in a debug (non-optimised) build.
    constexpr bool IsDebug = (ENGINE_DEBUG == 1);

    /// True when the engine is compiled in a release (optimised) build.
    constexpr bool IsRelease = (ENGINE_RELEASE == 1);

    /// True when assertion macros expand to real runtime checks.
    constexpr bool EnableAssertions = (ENGINE_ENABLE_ASSERTIONS == 1);

    /// True when profiling instrumentation is compiled in.
    constexpr bool EnableProfiling = (ENGINE_ENABLE_PROFILING == 1);

    /// True when the logging subsystem is available.
    constexpr bool EnableLogging = (ENGINE_ENABLE_LOGGING == 1);
} // namespace engine::buildconfig

// ENGINE_ENUM_FLAGS needs <type_traits> — include lazily so the header
// stays lightweight when the macro is not used.
#include <type_traits>