// ============================================================================
// File: Engine/Source/Diagnostics/CrashHandler.cpp
// Full implementation of CrashHandler — installs platform-specific signal /
// exception handlers that log crash details and capture a stack trace before
// the process terminates.
//
// POSIX:   sigaction() for SIGSEGV, SIGABRT, SIGFPE, SIGILL
// Windows: SetUnhandledExceptionFilter()
// ============================================================================

#include "Engine/Diagnostics/CrashHandler.h"
#include "Engine/Diagnostics/StackTrace.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/BuildConfig.h"
#include "Engine/Core/Platform.h"

#include <csignal>
#include <cstdlib>

// ============================================================================
// Platform-specific includes
// ============================================================================

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

namespace engine::diagnostics
{

using engine::core::usize;

// ===========================================================================
// Static storage
// ===========================================================================

static CrashHandler::CrashCallback s_crashCallback;

// ===========================================================================
// Helper — translate a signal number to a human-readable description
// ===========================================================================

static std::string SignalDescription(int sig)
{
#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS
    switch (sig)
    {
        case SIGSEGV: return "Segmentation fault (SIGSEGV)";
        case SIGABRT: return "Abort (SIGABRT)";
        case SIGFPE:  return "Floating-point exception (SIGFPE)";
        case SIGILL:  return "Illegal instruction (SIGILL)";
        case SIGBUS:  return "Bus error (SIGBUS)";
        case SIGTRAP: return "Trace/breakpoint trap (SIGTRAP)";
        default:      return "Unknown signal (" + std::to_string(sig) + ")";
    }
#elif ENGINE_PLATFORM_WINDOWS
    // Windows exception codes passed through the callback.
    switch (sig)
    {
        case EXCEPTION_ACCESS_VIOLATION:       return "Access violation (EXCEPTION_ACCESS_VIOLATION)";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:  return "Array bounds exceeded (EXCEPTION_ARRAY_BOUNDS_EXCEEDED)";
        case EXCEPTION_DATATYPE_MISALIGNMENT:  return "Data type misalignment (EXCEPTION_DATATYPE_MISALIGNMENT)";
        case EXCEPTION_FLT_DENORMAL_OPERAND:   return "Float denormal operand (EXCEPTION_FLT_DENORMAL_OPERAND)";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:     return "Float divide by zero (EXCEPTION_FLT_DIVIDE_BY_ZERO)";
        case EXCEPTION_FLT_INEXACT_RESULT:     return "Float inexact result (EXCEPTION_FLT_INEXACT_RESULT)";
        case EXCEPTION_FLT_INVALID_OPERATION:  return "Float invalid operation (EXCEPTION_FLT_INVALID_OPERATION)";
        case EXCEPTION_FLT_OVERFLOW:           return "Float overflow (EXCEPTION_FLT_OVERFLOW)";
        case EXCEPTION_FLT_STACK_CHECK:        return "Float stack check (EXCEPTION_FLT_STACK_CHECK)";
        case EXCEPTION_FLT_UNDERFLOW:          return "Float underflow (EXCEPTION_FLT_UNDERFLOW)";
        case EXCEPTION_ILLEGAL_INSTRUCTION:    return "Illegal instruction (EXCEPTION_ILLEGAL_INSTRUCTION)";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:     return "Integer divide by zero (EXCEPTION_INT_DIVIDE_BY_ZERO)";
        case EXCEPTION_INT_OVERFLOW:           return "Integer overflow (EXCEPTION_INT_OVERFLOW)";
        case EXCEPTION_PRIV_INSTRUCTION:       return "Privileged instruction (EXCEPTION_PRIV_INSTRUCTION)";
        case EXCEPTION_STACK_OVERFLOW:         return "Stack overflow (EXCEPTION_STACK_OVERFLOW)";
        default:                               return "Unknown exception (0x" + std::to_string(sig) + ")";
    }
#else
    return "Unknown signal (" + std::to_string(sig) + ")";
#endif
}

// ===========================================================================
// POSIX implementation
// ===========================================================================

#if ENGINE_PLATFORM_LINUX || ENGINE_PLATFORM_MACOS

/// Stores the previous signal handlers so we can restore them on shutdown.
static struct sigaction s_oldActions[4];
static bool s_initialized = false;

/// Indexes into s_oldActions for each handled signal.
static constexpr int kSignalIndices[] = { 0, 1, 2, 3 };
static constexpr int kSignals[]       = { SIGSEGV, SIGABRT, SIGFPE, SIGILL };

/// The actual signal handler. Uses only async-signal-safe operations where
/// possible; the stack trace capture may allocate memory (unavoidable for
/// meaningful output).
static void CrashSignalHandler(int sig, siginfo_t* /*info*/, void* /*ctx*/)
{
    std::string description = SignalDescription(sig);

    ENGINE_LOG_CRITICAL("========== FATAL SIGNAL ==========");
    ENGINE_LOG_CRITICAL("{}", description);
    ENGINE_LOG_CRITICAL("==================================");

    StackTrace::Print();

    // Invoke user callback if one was registered.
    if (s_crashCallback)
    {
        s_crashCallback(sig, description);
    }

    // Restore default handler and re-raise so the process terminates with
    // the correct signal disposition (core dump, etc.).
    struct sigaction defaultAction {};
    defaultAction.sa_handler = SIG_DFL;
    sigaction(sig, &defaultAction, nullptr);
    std::raise(sig);
}

// ---------------------------------------------------------------------------

void CrashHandler::Initialize()
{
    if (s_initialized)
    {
        return;
    }

    struct sigaction newAction {};
    newAction.sa_sigaction = CrashSignalHandler;
    // SA_SIGINFO provides the siginfo_t argument to the handler.
    // SA_RESETHAND resets the handler to SIG_DLF after the first trigger,
    // preventing recursive faults inside the handler.
    newAction.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&newAction.sa_mask);

    for (int i = 0; i < 4; ++i)
    {
        sigaction(kSignals[i], &newAction, &s_oldActions[kSignalIndices[i]]);
    }

    s_initialized = true;
    ENGINE_LOG_INFO("CrashHandler: installed signal handlers (SIGSEGV, SIGABRT, SIGFPE, SIGILL)");
}

void CrashHandler::Shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        sigaction(kSignals[i], &s_oldActions[kSignalIndices[i]], nullptr);
    }

    s_initialized = false;
    ENGINE_LOG_INFO("CrashHandler: restored default signal handlers");
}

void CrashHandler::SetCallback(CrashCallback cb)
{
    s_crashCallback = std::move(cb);
}

#endif // POSIX

// ===========================================================================
// Windows implementation
// ===========================================================================

#if ENGINE_PLATFORM_WINDOWS

static LPTOP_LEVEL_EXCEPTION_FILTER s_previousFilter = nullptr;
static bool s_initialized = false;

static LONG WINAPI CrashExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;
    std::string description = SignalDescription(static_cast<int>(code));

    ENGINE_LOG_CRITICAL("========== FATAL EXCEPTION ==========");
    ENGINE_LOG_CRITICAL("{}", description);
    ENGINE_LOG_CRITICAL("Address: 0x{:016X}",
        reinterpret_cast<std::uintptr_t>(exceptionInfo->ExceptionRecord->ExceptionAddress));
    ENGINE_LOG_CRITICAL("======================================");

    StackTrace::Print();

    // Invoke user callback if one was registered.
    if (s_crashCallback)
    {
        s_crashCallback(static_cast<int>(code), description);
    }

    // Pass to the previous filter or let the OS handle it (typically WER).
    if (s_previousFilter != nullptr)
    {
        return s_previousFilter(exceptionInfo);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

// ---------------------------------------------------------------------------

void CrashHandler::Initialize()
{
    if (s_initialized)
    {
        return;
    }

    s_previousFilter = SetUnhandledExceptionFilter(CrashExceptionHandler);
    s_initialized = true;
    ENGINE_LOG_INFO("CrashHandler: installed unhandled exception filter");
}

void CrashHandler::Shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    SetUnhandledExceptionFilter(s_previousFilter);
    s_previousFilter = nullptr;
    s_initialized = false;
    ENGINE_LOG_INFO("CrashHandler: restored default exception filter");
}

void CrashHandler::SetCallback(CrashCallback cb)
{
    s_crashCallback = std::move(cb);
}

#endif // _WIN32

// ===========================================================================
// Fallback for unknown platforms
// ===========================================================================

#if !ENGINE_PLATFORM_LINUX && !ENGINE_PLATFORM_MACOS && !ENGINE_PLATFORM_WINDOWS

static bool s_initialized = false;

void CrashHandler::Initialize()
{
    if (s_initialized)
    {
        return;
    }
    s_initialized = true;
    ENGINE_LOG_WARN("CrashHandler: signal handling not supported on this platform");
}

void CrashHandler::Shutdown()
{
    s_initialized = false;
}

void CrashHandler::SetCallback(CrashCallback cb)
{
    s_crashCallback = std::move(cb);
}

#endif

} // namespace engine::diagnostics