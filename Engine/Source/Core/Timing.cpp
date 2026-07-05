/// @file Timing.cpp
/// @brief Implementation of Timer and ScopedTimer.

#include "Engine/Core/Timing.h"

#include <spdlog/spdlog.h>

namespace engine::core {

// ===========================================================================
// Clock
// ===========================================================================

TimePoint Clock::Now()
{
    return std::chrono::high_resolution_clock::now();
}

f64 Clock::Timestamp()
{
    // Use system_clock for wall-clock epoch time (Unix timestamp).
    // high_resolution_clock may be an alias for steady_clock on some
    // platforms (e.g. macOS), which measures time since boot, not epoch.
    const auto now    = std::chrono::system_clock::now();
    const auto epoch  = std::chrono::system_clock::time_point{};
    return std::chrono::duration<f64>(now - epoch).count();
}

f64 Clock::SecondsSince(const TimePoint& point)
{
    const auto now = Now();
    return std::chrono::duration<f64>(now - point).count();
}

// ===========================================================================
// Timer
// ===========================================================================

void Timer::Start()
{
    m_start   = Clock::Now();
    m_end     = TimePoint{};
    m_running = true;
}

void Timer::Stop()
{
    if (m_running)
    {
        m_end     = Clock::Now();
        m_running = false;
    }
}

void Timer::Reset()
{
    m_start   = TimePoint{};
    m_end     = TimePoint{};
    m_running = false;
}

f64 Timer::ElapsedSeconds() const
{
    const TimePoint end = m_running ? Clock::Now() : m_end;
    return std::chrono::duration<f64>(end - m_start).count();
}

f64 Timer::ElapsedMilliseconds() const
{
    return ElapsedSeconds() * 1000.0;
}

f64 Timer::ElapsedMicroseconds() const
{
    return ElapsedSeconds() * 1'000'000.0;
}

// ===========================================================================
// ScopedTimer
// ===========================================================================

ScopedTimer::ScopedTimer(std::string_view name)
    : m_name(name)
{
    m_timer.Start();
}

ScopedTimer::~ScopedTimer()
{
    m_timer.Stop();
    spdlog::info("ScopedTimer '{}': {:.3f} ms", m_name, m_timer.ElapsedMilliseconds());
}

} // namespace engine::core