#pragma once

// ============================================================================
// File: Engine/Include/Engine/Diagnostics/StackTrace.h
// Stack trace capture interface. Captures the current call stack, optionally
// demangles C++ symbols, and prints the trace through the engine log system.
// ============================================================================

#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine::diagnostics
{

using engine::core::usize;

/// @brief Captures and formats the current call stack.
///
/// All methods are static; the class cannot be instantiated.
/// On POSIX platforms the implementation uses `backtrace()` / `backtrace_symbols()`.
/// On Windows it uses `CaptureStackBackTrace()` + DbgHelp's `SymFromAddr()`.
///
/// @note On Linux, the binary must be linked with `-rdynamic` (or equivalent)
///       for function names to appear in the trace.
class StackTrace
{
public:
    StackTrace() = delete;

    /// @brief Captures the current call stack as a formatted string.
    /// @param maxFrames Maximum number of frames to capture (default 32).
    /// @return Multi-line string with one frame per line, or a placeholder on
    ///         unsupported platforms.
    static std::string Capture(usize maxFrames = 32);

    /// @brief Captures and logs the current call stack via ENGINE_LOG_ERROR.
    static void Print();

    /// @brief Demangles a C++ mangled symbol name.
    /// @param mangled The mangled symbol string (e.g. `_ZN3foo4barEi`).
    /// @return The human-readable demangled name, or the original string if
    ///         demangling fails or is unavailable.
    static std::string Demangle(const std::string& mangled);
};

} // namespace engine::diagnostics