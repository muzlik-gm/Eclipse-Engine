#pragma once

/**
 * @file Instrumentation.h
 * @brief Profiling instrumentation with Chrome Trace Event Format output.
 *
 * Provides RAII scope-based profiling via ProfilerTimer and session
 * management via Instrumentation.  When ENGINE_ENABLE_PROFILING is 0 all
 * profiling code compiles to no-ops with zero runtime overhead.
 *
 * Output files use the Chrome Trace Event Format (JSON) and can be loaded
 * at chrome://tracing or in the Perfetto UI (https://ui.perfetto.dev).
 */

#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Types.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <source_location>

namespace engine::diagnostics
{

using engine::core::i64;
using engine::core::u32;

// ===========================================================================
// ProfileResult
// ===========================================================================

/// @brief Stores the result of a single profiled scope.
struct ProfileResult
{
    std::string          name;
    std::string          category;
    i64                  start;     ///< Start timestamp in microseconds (relative to session).
    i64                  end;       ///< End timestamp in microseconds (relative to session).
    u32                  threadId;
    std::source_location location;
};

// ===========================================================================
// ProfilerTimer
// ===========================================================================

/// @brief RAII profiler timer that records a scope's duration on destruction.
///
/// Place an instance at the top of any function or scope you wish to profile.
/// The timer captures the start time in its constructor and writes a
/// ProfileResult to the active Instrumentation session on destruction.
///
/// When ENGINE_ENABLE_PROFILING is 0 the constructor and destructor bodies
/// are empty and no member variables exist, so the compiler eliminates all
/// overhead entirely.
class ProfilerTimer
{
public:
    /// @brief Begins a profiled scope.
    /// @param name      Human-readable label for the profiled region.
    /// @param category  Trace event category (defaults to "default").
    /// @param loc       Source location captured at the call site.
    ProfilerTimer(
        std::string_view name,
        std::string_view category = "default",
        std::source_location loc  = std::source_location::current());

    /// @brief Ends the profiled scope and writes the result.
    ~ProfilerTimer();

    ProfilerTimer(const ProfilerTimer&)            = delete;
    ProfilerTimer& operator=(const ProfilerTimer&) = delete;
    ProfilerTimer(ProfilerTimer&&)                 = delete;
    ProfilerTimer& operator=(ProfilerTimer&&)      = delete;

private:
#if ENGINE_ENABLE_PROFILING
    std::string          m_name;
    std::string          m_category;
    std::source_location m_location;
    i64                  m_startTime = 0;
#endif
};

// ===========================================================================
// Instrumentation
// ===========================================================================

/// @brief Manages instrumentation sessions and writes Chrome Trace Event Format JSON.
///
/// All methods are static; the class cannot be instantiated.  When
/// ENGINE_ENABLE_PROFILING is 0 every method is a no-op.
///
/// Typical usage:
/// @code
///   engine::diagnostics::Instrumentation::Initialize();
///   engine::diagnostics::Instrumentation::BeginSession("GameLoop");
///   // ... profiled code runs ...
///   engine::diagnostics::Instrumentation::EndSession();
///   engine::diagnostics::Instrumentation::Shutdown();
/// @endcode
class Instrumentation
{
public:
    Instrumentation() = delete;

    /// @brief Initialises the instrumentation subsystem.
    ///
    /// Called once at engine startup.  Does not open a file — use
    /// BeginSession() to start an active profiling session.
    static void Initialize();

    /// @brief Shuts down the instrumentation subsystem.
    ///
    /// Ends any active session, flushes, and releases resources.
    static void Shutdown();

    /// @brief Begins a named profiling session that writes to @p filepath.
    ///
    /// If a session is already active it is ended first.
    /// @param name     Session name included as a trace metadata event.
    /// @param filepath Output file path (defaults to "profiling.json").
    static void BeginSession(
        std::string_view name,
        std::string_view filepath = "profiling.json");

    /// @brief Ends the current profiling session and closes the output file.
    static void EndSession();
};

} // namespace engine::diagnostics

// ============================================================================
// Convenience macros
// ============================================================================

#if ENGINE_ENABLE_PROFILING

    /// @brief Creates a profiling scope with an explicit name.
    #define ENGINE_PROFILE_SCOPE(name)                                              \
        ::engine::diagnostics::ProfilerTimer ENGINE_ANONYMOUS_VAR(engineProfiler_)( \
            name, "default")

    /// @brief Creates a profiling scope that automatically uses the enclosing
    ///        function name from std::source_location.
    #define ENGINE_PROFILE_FUNCTION()                                               \
        ::engine::diagnostics::ProfilerTimer ENGINE_ANONYMOUS_VAR(engineProfiler_)( \
            std::source_location::current().function_name())

#else // !ENGINE_ENABLE_PROFILING

    /// @brief No-op when profiling is disabled.
    #define ENGINE_PROFILE_SCOPE(name)  ((void)0)

    /// @brief No-op when profiling is disabled.
    #define ENGINE_PROFILE_FUNCTION()   ((void)0)

#endif // ENGINE_ENABLE_PROFILING

// Helper macro for generating unique variable names (compiler-specific).
#ifndef ENGINE_ANONYMOUS_VAR
    #if defined(__COUNTER__)
        #define ENGINE_ANONYMOUS_VAR(prefix) prefix##__COUNTER__
    #else
        #define ENGINE_ANONYMOUS_VAR(prefix) prefix##__LINE__
    #endif
#endif