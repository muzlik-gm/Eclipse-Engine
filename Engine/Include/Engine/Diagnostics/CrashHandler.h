#pragma once

// ============================================================================
// File: Engine/Include/Engine/Diagnostics/CrashHandler.h
// Crash diagnostics interface. Installs platform-specific signal / exception
// handlers so that fatal crashes (SIGSEGV, SIGABRT, etc.) are logged with a
// full stack trace before the process terminates.
// ============================================================================

#include "Engine/Core/Types.h"

#include <functional>
#include <string>

namespace engine::diagnostics
{

/// @brief Installs fatal-crash signal handlers with stack-trace capture.
///
/// All methods are static; the class cannot be instantiated.
/// On POSIX systems the following signals are intercepted:
///   `SIGSEGV`, `SIGABRT`, `SIGFPE`, `SIGILL`.
/// On Windows `SetUnhandledExceptionFilter()` is used instead.
///
/// Typical usage:
/// @code
///   engine::diagnostics::CrashHandler::Initialize();
///   // ... engine lifetime ...
///   engine::diagnostics::CrashHandler::Shutdown();
/// @endcode
class CrashHandler
{
public:
    CrashHandler() = delete;

    /// @brief Callback signature invoked when a fatal signal is received.
    /// @param signal      Signal number (POSIX) or exception code (Windows).
    /// @param description Human-readable description of the fault.
    using CrashCallback = std::function<void(int signal, const std::string& description)>;

    /// @brief Installs signal/exception handlers.
    ///
    /// Safe to call multiple times — subsequent calls are no-ops.
    static void Initialize();

    /// @brief Restores the default signal/exception handlers.
    static void Shutdown();

    /// @brief Sets an optional user callback invoked after the engine logs
    ///        the crash information but before the process exits.
    /// @param cb The callback to invoke on crash.
    static void SetCallback(CrashCallback cb);
};

} // namespace engine::diagnostics