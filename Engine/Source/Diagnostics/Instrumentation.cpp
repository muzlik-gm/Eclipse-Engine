/**
 * @file Instrumentation.cpp
 * @brief Profiling instrumentation implementation — Chrome Trace Event Format.
 */

#include "Engine/Diagnostics/Instrumentation.h"

#if ENGINE_ENABLE_PROFILING

#include "Engine/Core/Platform.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

// ===========================================================================
// Platform-specific thread ID retrieval
// ===========================================================================

namespace
{

using engine::core::i64;
using engine::core::u32;
using engine::core::u64;

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

    u32 GetCurrentThreadIdNative()
    {
        return static_cast<u32>(::GetCurrentThreadId());
    }

#elif ENGINE_PLATFORM_MACOS
    #include <pthread.h>

    u32 GetCurrentThreadIdNative()
    {
        u64 tid = 0;
        pthread_threadid_np(nullptr, &tid);
        return static_cast<u32>(tid);
    }

#elif ENGINE_PLATFORM_LINUX
    #include <unistd.h>
    #include <sys/syscall.h>

    u32 GetCurrentThreadIdNative()
    {
        return static_cast<u32>(static_cast<pid_t>(syscall(SYS_gettid)));
    }

#else
    #include <functional>

    u32 GetCurrentThreadIdNative()
    {
        return static_cast<u32>(
            std::hash<std::thread::id>{}(std::this_thread::get_id()));
    }
#endif

/// @brief Returns a monotonic timestamp in microseconds since the clock epoch.
using Clock = std::chrono::high_resolution_clock;
using Microseconds = std::chrono::microseconds;

i64 NowMicroseconds()
{
    return std::chrono::duration_cast<Microseconds>(
               Clock::now().time_since_epoch())
        .count();
}

/// @brief Escapes a string for safe inclusion in a JSON value.
std::string JsonEscape(std::string_view str)
{
    std::string result;
    result.reserve(str.size());

    for (char c : str)
    {
        switch (c)
        {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b";  break;
            case '\f': result += "\\f";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    char buf[8];
                    std::snprintf(
                        buf,
                        sizeof(buf),
                        "\\u%04x",
                        static_cast<unsigned>(static_cast<unsigned char>(c)));
                    result += buf;
                }
                else
                {
                    result += c;
                }
                break;
        }
    }

    return result;
}

} // anonymous namespace

// ===========================================================================
// Session state (file-scope to keep the header clean)
// ===========================================================================

namespace
{

struct SessionState
{
    std::ofstream outputStream;
    std::string   sessionName;
    std::mutex    mutex;
    i64           sessionStartTime = 0;
    bool          active           = false;
    bool          firstEvent       = true;
    bool          initialized      = false;
};

SessionState& GetState()
{
    static SessionState s_state;
    return s_state;
}

} // anonymous namespace

#endif // ENGINE_ENABLE_PROFILING

// ===========================================================================
// ProfilerTimer
// ===========================================================================

namespace engine::diagnostics
{

ProfilerTimer::ProfilerTimer(
    std::string_view  name,
    std::string_view  category,
    std::source_location loc)
{
#if ENGINE_ENABLE_PROFILING
    m_name     = name;
    m_category = category;
    m_location = loc;
    m_startTime = NowMicroseconds() - GetState().sessionStartTime;
#else
    (void)name;
    (void)category;
    (void)loc;
#endif
}

ProfilerTimer::~ProfilerTimer()
{
#if ENGINE_ENABLE_PROFILING
    SessionState& state = GetState();

    std::lock_guard lock(state.mutex);
    if (!state.active)
    {
        return;
    }

    i64 endTime = NowMicroseconds() - state.sessionStartTime;

    ProfileResult result;
    result.name     = std::move(m_name);
    result.category = std::move(m_category);
    result.start    = m_startTime;
    result.end      = endTime;
    result.threadId = GetCurrentThreadIdNative();
    result.location = m_location;

    // Write Chrome Trace Event Format JSON object.
    // Fields: name, cat, ph, pid, tid, ts, dur, args (source location).
    if (!state.firstEvent)
    {
        state.outputStream << ",\n";
    }
    state.firstEvent = false;

    state.outputStream << "{";
    state.outputStream << "\"name\":\"" << JsonEscape(result.name) << "\",";
    state.outputStream << "\"cat\":\""  << JsonEscape(result.category) << "\",";
    state.outputStream << "\"ph\":\"X\",";
    state.outputStream << "\"pid\":0,";
    state.outputStream << "\"tid\":" << result.threadId << ",";
    state.outputStream << "\"ts\":"  << result.start << ",";
    state.outputStream << "\"dur\":" << (result.end - result.start) << ",";
    state.outputStream << "\"args\":{";
    state.outputStream << "\"file\":\"" << JsonEscape(result.location.file_name()) << "\",";
    state.outputStream << "\"line\":" << result.location.line();
    state.outputStream << "}}";
#endif
}

// ===========================================================================
// Instrumentation
// ===========================================================================

void Instrumentation::Initialize()
{
#if ENGINE_ENABLE_PROFILING
    SessionState& state = GetState();

    std::lock_guard lock(state.mutex);
    state.initialized = true;
    state.active      = false;
    state.firstEvent  = true;
#endif
}

void Instrumentation::Shutdown()
{
#if ENGINE_ENABLE_PROFILING
    SessionState& state = GetState();

    std::lock_guard lock(state.mutex);

    if (state.active)
    {
        EndSession();
    }

    state.initialized = false;
#endif
}

void Instrumentation::BeginSession(std::string_view name, std::string_view filepath)
{
#if ENGINE_ENABLE_PROFILING
    SessionState& state = GetState();

    std::lock_guard lock(state.mutex);

    // End any active session first.
    if (state.active)
    {
        state.outputStream << "\n]}";
        state.outputStream.flush();
        state.outputStream.close();
        state.active     = false;
        state.firstEvent = true;
    }

    state.sessionName     = name;
    state.sessionStartTime = NowMicroseconds();
    state.firstEvent      = true;
    state.outputStream.open(std::string(filepath), std::ios::out | std::ios::trunc);

    if (state.outputStream.is_open())
    {
        state.outputStream << "{\"traceEvents\":[";
        state.outputStream.flush();
        state.active = true;
    }
#else
    (void)name;
    (void)filepath;
#endif
}

void Instrumentation::EndSession()
{
#if ENGINE_ENABLE_PROFILING
    SessionState& state = GetState();

    std::lock_guard lock(state.mutex);

    if (!state.active)
    {
        return;
    }

    state.outputStream << "\n]}";
    state.outputStream.flush();
    state.outputStream.close();
    state.active     = false;
    state.firstEvent = true;
#endif
}

} // namespace engine::diagnostics