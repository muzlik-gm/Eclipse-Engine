#pragma once

/// @file Timing.h
/// @brief High-resolution timing utilities for the Engine.
///
/// Provides three facilities:
///   - Clock:    static free-functions for querying wall-clock time.
///   - Timer:    a start/stop stopwatch for manual measurement.
///   - ScopedTimer: an RAII guard that logs elapsed time on destruction.
///
/// Usage:
///   auto t0 = engine::core::Clock::Now();
///   // ... work ...
///   f64 dt = engine::core::Clock::SecondsSince(t0);
///
///   engine::core::Timer timer;
///   timer.Start();
///   // ... work ...
///   timer.Stop();
///   f64 ms = timer.ElapsedMilliseconds();
///
///   {
///       engine::core::ScopedTimer guard("AssetLoad");
///       // ... work ...  <- elapsed time logged on scope exit
///   }

#include "Engine/Core/Types.h"

#include <chrono>
#include <string>
#include <string_view>

namespace engine::core {

/// @brief Monotonic high-resolution time point used by all timing classes.
using TimePoint = std::chrono::high_resolution_clock::time_point;

// ===========================================================================
// Clock
// ===========================================================================

/// @brief Static utility class for querying the high-resolution clock.
class Clock
{
public:
    Clock() = delete;

    /// @brief Returns the current monotonic time point.
    [[nodiscard]] static TimePoint Now();

    /// @brief Returns the current time as seconds since the clock epoch.
    [[nodiscard]] static f64 Timestamp();

    /// @brief Returns the elapsed seconds since @p point.
    [[nodiscard]] static f64 SecondsSince(const TimePoint& point);
};

// ===========================================================================
// Timer
// ===========================================================================

/// @brief Manual start/stop stopwatch.
///
/// The timer is initially not running.  Call Start() to begin measuring and
/// Stop() to record the end point.  While running, Elapsed*() methods return
/// the time from Start() to Now(); after Stop() they return the fixed
/// interval between Start() and Stop().
class Timer
{
public:
    /// @brief Constructs a stopped timer with no recorded interval.
    Timer() = default;

    /// @brief Starts (or restarts) the timer.
    void Start();

    /// @brief Stops the timer and records the end point.
    ///        Only valid when IsRunning() is true.
    void Stop();

    /// @brief Resets the timer to the initial (stopped, zero-elapsed) state.
    void Reset();

    /// @brief Returns elapsed time in seconds.
    [[nodiscard]] f64 ElapsedSeconds() const;

    /// @brief Returns elapsed time in milliseconds.
    [[nodiscard]] f64 ElapsedMilliseconds() const;

    /// @brief Returns elapsed time in microseconds.
    [[nodiscard]] f64 ElapsedMicroseconds() const;

    /// @brief Returns true if the timer is currently running.
    [[nodiscard]] bool IsRunning() const { return m_running; }

private:
    TimePoint m_start{};
    TimePoint m_end{};
    bool      m_running = false;
};

// ===========================================================================
// ScopedTimer
// ===========================================================================

/// @brief RAII timer that logs the elapsed duration on destruction.
///
/// Useful for quick profiling of scoped blocks:
/// @code
///   {
///       ScopedTimer t("LoadAssets");
///       // ... expensive work ...
///   }  // <-- logs "ScopedTimer 'LoadAssets': 12.345 ms"
/// @endcode
class ScopedTimer
{
public:
    /// @brief Constructs a ScopedTimer that starts immediately.
    /// @param name A human-readable label included in the log message.
    explicit ScopedTimer(std::string_view name);

    /// @brief Stops the timer (if still running) and logs the result.
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&)            = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&)                 = delete;
    ScopedTimer& operator=(ScopedTimer&&)      = delete;

private:
    std::string m_name;
    Timer       m_timer;
};

} // namespace engine::core