#pragma once

/**
 * @file Compiler.h
 * @brief Compile-time compiler detection and feature-capability queries.
 *
 * Identifies the active compiler and its version, and exposes preprocessor
 * flags for optional C++20/23 features so the rest of the codebase can
 * conditionally use them without repeating version checks.
 */

// ============================================================================
// Compiler identification — each is 0 or 1.
// Order matters: Clang also defines __GNUC__, so check it first.
// ============================================================================

#ifndef ENGINE_COMPILER_CLANG
#    if defined(__clang__)
#        define ENGINE_COMPILER_CLANG 1
#    else
#        define ENGINE_COMPILER_CLANG 0
#    endif
#endif

#ifndef ENGINE_COMPILER_GCC
#    if defined(__GNUC__) && !ENGINE_COMPILER_CLANG
#        define ENGINE_COMPILER_GCC 1
#    else
#        define ENGINE_COMPILER_GCC 0
#    endif
#endif

#ifndef ENGINE_COMPILER_MSVC
#    if defined(_MSC_VER)
#        define ENGINE_COMPILER_MSVC 1
#    else
#        define ENGINE_COMPILER_MSVC 0
#    endif
#endif

// ============================================================================
// Compiler version decomposition
// ============================================================================

#if ENGINE_COMPILER_MSVC
#    ifndef ENGINE_COMPILER_VERSION_MAJOR
#        define ENGINE_COMPILER_VERSION_MAJOR (_MSC_VER / 100)
#    endif
#    ifndef ENGINE_COMPILER_VERSION_MINOR
#        define ENGINE_COMPILER_VERSION_MINOR ((_MSC_VER % 100) / 10)
#    endif
#    ifndef ENGINE_COMPILER_VERSION_PATCH
#        define ENGINE_COMPILER_VERSION_PATCH (_MSC_VER % 10)
#    endif
#elif ENGINE_COMPILER_CLANG
#    ifndef ENGINE_COMPILER_VERSION_MAJOR
#        define ENGINE_COMPILER_VERSION_MAJOR __clang_major__
#    endif
#    ifndef ENGINE_COMPILER_VERSION_MINOR
#        define ENGINE_COMPILER_VERSION_MINOR __clang_minor__
#    endif
#    ifndef ENGINE_COMPILER_VERSION_PATCH
#        define ENGINE_COMPILER_VERSION_PATCH __clang_patchlevel__
#    endif
#elif ENGINE_COMPILER_GCC
#    ifndef ENGINE_COMPILER_VERSION_MAJOR
#        define ENGINE_COMPILER_VERSION_MAJOR __GNUC__
#    endif
#    ifndef ENGINE_COMPILER_VERSION_MINOR
#        define ENGINE_COMPILER_VERSION_MINOR __GNUC_MINOR__
#    endif
#    ifndef ENGINE_COMPILER_VERSION_PATCH
#        define ENGINE_COMPILER_VERSION_PATCH __GNUC_PATCHLEVEL__
#    endif
#else
#    define ENGINE_COMPILER_VERSION_MAJOR 0
#    define ENGINE_COMPILER_VERSION_MINOR 0
#    define ENGINE_COMPILER_VERSION_PATCH 0
#endif

// Human-readable compiler name as a string literal.
#if ENGINE_COMPILER_MSVC
#    define ENGINE_COMPILER_NAME "MSVC"
#elif ENGINE_COMPILER_CLANG
#    define ENGINE_COMPILER_NAME "Clang"
#elif ENGINE_COMPILER_GCC
#    define ENGINE_COMPILER_NAME "GCC"
#else
#    define ENGINE_COMPILER_NAME "Unknown"
#endif

// ============================================================================
// Feature-capability flags — each is 0 or 1.
// ============================================================================

// consteval requires C++20 and a compiler version that actually implements it.
#ifndef ENGINE_SUPPORTS_CONSTEVAL
#    if ENGINE_COMPILER_MSVC && (ENGINE_COMPILER_VERSION_MAJOR < 19 || \
        (ENGINE_COMPILER_VERSION_MAJOR == 19 && ENGINE_COMPILER_VERSION_MINOR < 29))
#        define ENGINE_SUPPORTS_CONSTEVAL 0
#    elif ENGINE_COMPILER_CLANG && ENGINE_COMPILER_VERSION_MAJOR < 12
#        define ENGINE_SUPPORTS_CONSTEVAL 0
#    elif ENGINE_COMPILER_GCC && ENGINE_COMPILER_VERSION_MAJOR < 12
#        define ENGINE_SUPPORTS_CONSTEVAL 0
#    else
#        define ENGINE_SUPPORTS_CONSTEVAL 1
#    endif
#endif

// C++20 modules — flag when the compiler is known to handle the
// export/module syntax used by this project.
#ifndef ENGINE_SUPPORTS_MODULES
#    if ENGINE_COMPILER_MSVC && ENGINE_COMPILER_VERSION_MAJOR >= 19
#        define ENGINE_SUPPORTS_MODULES 1
#    elif ENGINE_COMPILER_CLANG && ENGINE_COMPILER_VERSION_MAJOR >= 16
#        define ENGINE_SUPPORTS_MODULES 1
#    elif ENGINE_COMPILER_GCC && ENGINE_COMPILER_VERSION_MAJOR >= 14
#        define ENGINE_SUPPORTS_MODULES 0
#    else
#        define ENGINE_SUPPORTS_MODULES 0
#    endif
#endif

// C++20 concepts — the engine requires them, but the flag allows
// concept-free fallback paths for extremely old toolchains.
#ifndef ENGINE_SUPPORTS_CONCEPTS
#    if ENGINE_COMPILER_MSVC && ENGINE_COMPILER_VERSION_MAJOR >= 19
#        define ENGINE_SUPPORTS_CONCEPTS 1
#    elif ENGINE_COMPILER_CLANG && ENGINE_COMPILER_VERSION_MAJOR >= 12
#        define ENGINE_SUPPORTS_CONCEPTS 1
#    elif ENGINE_COMPILER_GCC && ENGINE_COMPILER_VERSION_MAJOR >= 10
#        define ENGINE_SUPPORTS_CONCEPTS 1
#    else
#        define ENGINE_SUPPORTS_CONCEPTS 0
#    endif
#endif

// ============================================================================
// Compiler-warning push / pop helpers
// ============================================================================

#if ENGINE_COMPILER_MSVC
#    define ENGINE_PRAGMA_PUSH __pragma(warning(push))
#    define ENGINE_PRAGMA_POP  __pragma(warning(pop))
#else
#    define ENGINE_PRAGMA_PUSH _Pragma("GCC diagnostic push")
#    define ENGINE_PRAGMA_POP  _Pragma("GCC diagnostic pop")
#endif

// Disable a specific warning by name (e.g. ENGINE_PRAGMA_DISABLE(shadow)).
// On MSVC the name is the warning number; on GCC/Clang it is the flag suffix.
#if ENGINE_COMPILER_MSVC
#    define ENGINE_PRAGMA_DISABLE(w) __pragma(warning(disable : w))
#else
#    define ENGINE_STRINGIFY_(x) #x
#    define ENGINE_STRINGIFY(x) ENGINE_STRINGIFY_(x)
#    define ENGINE_PRAGMA_DISABLE(w) _Pragma(ENGINE_STRINGIFY(GCC diagnostic ignored "-W" w))
#endif

// ============================================================================
// Runtime compiler queries via constexpr
// ============================================================================

namespace engine::compiler
{
    /// True when compiling with Microsoft Visual C++.
    constexpr bool IsMSVC = (ENGINE_COMPILER_MSVC == 1);

    /// True when compiling with GNU Compiler Collection (not Clang).
    constexpr bool IsGCC = (ENGINE_COMPILER_GCC == 1);

    /// True when compiling with Clang (including Apple Clang).
    constexpr bool IsClang = (ENGINE_COMPILER_CLANG == 1);

    /// Major component of the detected compiler version.
    constexpr int VersionMajor = ENGINE_COMPILER_VERSION_MAJOR;

    /// Minor component of the detected compiler version.
    constexpr int VersionMinor = ENGINE_COMPILER_VERSION_MINOR;

    /// Returns a compile-time string literal identifying the compiler.
    consteval const char* Name()
    {
        return ENGINE_COMPILER_NAME;
    }
} // namespace engine::compiler