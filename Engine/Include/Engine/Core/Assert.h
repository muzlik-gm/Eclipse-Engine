#pragma once

/// @file Assert.h
/// @brief Assertion macros for the Engine.
///
/// Provides ENGINE_ASSERT / ENGINE_CORE_ASSERT and their explicit-message
/// variants.  When ENGINE_ENABLE_ASSERTIONS is 0 the condition is still
/// evaluated (to suppress unused-variable warnings) but no runtime check
/// is performed on failure.
///
/// Usage:
///   ENGINE_ASSERT(ptr != nullptr);
///   ENGINE_ASSERT(count > 0, "Expected at least one item, got {}", count);
///   ENGINE_ASSERT_MSG(isValid, "Entity must be valid before update");

#include "Engine/Core/BuildConfig.h"

#include <spdlog/fmt/fmt.h>

namespace engine::core::detail {

/// @brief Core assertion-failure handler (implemented in Assert.cpp).
///
/// Logs a critical error with full context and terminates the process
/// with a platform-specific debug trap.
///
/// @param condition The source text of the assertion expression.
/// @param message   User-supplied message (may be "").
/// @param file      Source file (__FILE__).
/// @param line      Source line number (__LINE__).
/// @param function  Enclosing function (__func__).
[[noreturn]]
void AssertFail(const char* condition,
                const char* message,
                const char* file,
                int         line,
                const char* function);

/// @brief No-message convenience overload that forwards an empty message.
[[noreturn]] inline void
AssertFail(const char* condition,
           const char* file,
           int         line,
           const char* function)
{
    AssertFail(condition, "", file, line, function);
}

/// @brief Variadic-format overload.
///
/// Formats the message via fmt::format and forwards to the core
/// AssertFail.  This overload is only resolved when a format string
/// (and optional arguments) are provided by the caller.
template <typename... Args>
[[noreturn]] inline void
AssertFail(const char*                    condition,
           const char*                    file,
           int                            line,
           const char*                    function,
           fmt::format_string<Args...>    fmt,
           Args&&...                      args)
{
    const std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    AssertFail(condition, message.c_str(), file, line, function);
}

} // namespace engine::core::detail

// ---------------------------------------------------------------------------
// ENGINE_ASSERT_MSG / ENGINE_ASSERT
// ---------------------------------------------------------------------------
#if ENGINE_ENABLE_ASSERTIONS

    /// @def ENGINE_ASSERT_MSG(condition, message)
    /// @brief Asserts that @p condition is true.
    ///        On failure, calls AssertFail with the given @p message.
    #define ENGINE_ASSERT_MSG(condition, message)                            \
        do {                                                                 \
            if (!(condition)) {                                              \
                ::engine::core::detail::AssertFail(                          \
                    #condition, message,                                     \
                    __FILE__, static_cast<int>(__LINE__),                    \
                    static_cast<const char*>(__func__));                     \
            }                                                                \
        } while (0)

    /// @def ENGINE_ASSERT(condition, ...)
    /// @brief Asserts that @p condition is true.
    ///        When extra arguments are supplied they are formatted via
    ///        fmt::format to produce the failure message.  With no extra
    ///        arguments an empty message is used.
    #define ENGINE_ASSERT(condition, ...)                                    \
        do {                                                                 \
            if (!(condition)) {                                              \
                ::engine::core::detail::AssertFail(                          \
                    #condition,                                              \
                    __FILE__, static_cast<int>(__LINE__),                    \
                    static_cast<const char*>(__func__)                       \
                    __VA_OPT__(,) __VA_ARGS__);                              \
            }                                                                \
        } while (0)

#else // !ENGINE_ENABLE_ASSERTIONS

    /// @brief No-op (still evaluates @p condition) when assertions are disabled.
    #define ENGINE_ASSERT_MSG(condition, message)                            \
        do { static_cast<void>(condition); } while (0)

    /// @brief No-op (still evaluates @p condition) when assertions are disabled.
    #define ENGINE_ASSERT(condition, ...)                                    \
        do { static_cast<void>(condition); } while (0)

#endif // ENGINE_ENABLE_ASSERTIONS

// ---------------------------------------------------------------------------
// Aliases – ENGINE_CORE_ASSERT / ENGINE_CORE_ASSERT_MSG
// ---------------------------------------------------------------------------
#define ENGINE_CORE_ASSERT(condition, ...)      ENGINE_ASSERT(condition, __VA_ARGS__)
#define ENGINE_CORE_ASSERT_MSG(condition, msg)  ENGINE_ASSERT_MSG(condition, msg)