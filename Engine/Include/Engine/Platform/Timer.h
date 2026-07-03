// ============================================================================
// File: Engine/Include/Engine/Platform/Timer.h
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"

#include <functional>

namespace engine::platform
{

    using engine::core::f64;
    using engine::core::u64;

    // ========================================================================
    // ITimer – pure-virtual high-resolution timer interface
    // ========================================================================

    /// Platform-independent timer for measuring elapsed time.
    /// Provides both absolute time queries and relative interval timing.
    class ITimer
    {
    public:
        virtual ~ITimer() = default;

        /// Resets the timer to zero and starts it.
        virtual void Reset() = 0;

        /// Starts (or resumes) the timer.
        virtual void Start() = 0;

        /// Stops the timer, preserving the elapsed time.
        virtual void Stop() = 0;

        /// Returns the total elapsed time in seconds since the last Reset(),
        /// including time while stopped.
        [[nodiscard]] virtual f64 GetElapsed() const = 0;

        /// Returns true if the timer is currently running.
        [[nodiscard]] virtual bool IsRunning() const = 0;

        /// Sets a callback that fires once after the given delay in seconds.
        /// The timer continues running after the callback fires.
        /// Passing a negative or zero delay disables the timer callback.
        virtual void SetCallback(f64 delaySeconds, std::function<void()> callback) = 0;

        /// Returns the current high-resolution time in seconds since an
        /// arbitrary epoch (typically system boot).
        [[nodiscard]] static f64 GetAbsoluteTime();

        /// Returns the current high-resolution time in nanoseconds.
        [[nodiscard]] static u64 GetAbsoluteTimeNanoseconds();
    };

} // namespace engine::platform