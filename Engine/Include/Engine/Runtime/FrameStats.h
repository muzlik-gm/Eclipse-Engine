// ============================================================================
// File: Engine/Include/Engine/Runtime/FrameStats.h
// Frame timing, fixed-timestep accumulator, and runtime statistics.
// ============================================================================

#pragma once

#include "Engine/Core/Timing.h"
#include "Engine/Core/Types.h"

#include <chrono>

namespace engine::runtime
{

    using engine::core::f64;
    using engine::core::u64;
    using engine::core::usize;

    // ========================================================================
    // FrameStats
    // ========================================================================

    /// Collects per-frame and cumulative runtime statistics.
    ///
    /// Maintains delta time, a fixed-timestep accumulator, frame counters,
    /// and interfaces for frame-rate limiting and time scaling.  The class
    /// is intentionally value-typed and movable so it can be passed by value
    /// or embedded directly into the Engine.
    class FrameStats
    {
    public:
        FrameStats() = default;

        // --------------------------------------------------------------------
        // Per-frame timing
        // --------------------------------------------------------------------

        /// Wall-clock delta time in seconds for the most recent frame.
        [[nodiscard]] f64 DeltaTime() const noexcept { return m_deltaTime; }

        /// Unscaled (real-world) delta time, unaffected by time scale.
        [[nodiscard]] f64 UnscaledDeltaTime() const noexcept { return m_unscaledDeltaTime; }

        /// Total elapsed time in seconds since the engine started running.
        [[nodiscard]] f64 TotalTime() const noexcept { return m_totalTime; }

        /// Total unscaled elapsed time.
        [[nodiscard]] f64 UnscaledTotalTime() const noexcept { return m_unscaledTotalTime; }

        /// Current time scale factor.  1.0 = normal, 0.5 = half speed, 2.0 = double.
        [[nodiscard]] f64 TimeScale() const noexcept { return m_timeScale; }

        /// Sets the time scale factor.  Clamped to [0.0, 100.0].
        void SetTimeScale(f64 scale) noexcept;

        // --------------------------------------------------------------------
        // Frame counting
        // --------------------------------------------------------------------

        /// Sequential frame number (starts at 0 and increments each frame).
        [[nodiscard]] u64 FrameCount() const noexcept { return m_frameCount; }

        /// Total number of fixed-update ticks that have been executed.
        [[nodiscard]] u64 FixedTickCount() const noexcept { return m_fixedTickCount; }

        // --------------------------------------------------------------------
        // Fixed timestep
        // --------------------------------------------------------------------

        /// The fixed timestep in seconds (default 1/60).
        [[nodiscard]] f64 FixedDeltaTime() const noexcept { return m_fixedDeltaTime; }

        /// Sets the fixed timestep.  Must be greater than 0.
        void SetFixedDeltaTime(f64 dt) noexcept;

        /// Maximum number of fixed updates allowed per frame (prevents spiral
        /// of death).  Default is 5.
        [[nodiscard]] usize MaxFixedStepsPerFrame() const noexcept { return m_maxFixedSteps; }

        /// Sets the maximum fixed-update steps per frame.  Must be >= 1.
        void SetMaxFixedStepsPerFrame(usize steps) noexcept;

        /// Returns true when a fixed update should be performed this frame.
        /// If true, the caller should call ConsumeFixedStep() afterwards.
        [[nodiscard]] bool ShouldFixedUpdate() const noexcept;

        /// Consumes one fixed step from the accumulator.
        void ConsumeFixedStep() noexcept;

        // --------------------------------------------------------------------
        // Frame rate limiting
        // --------------------------------------------------------------------

        /// Target frame time in seconds.  0 means no limiting (uncapped).
        [[nodiscard]] f64 TargetFrameTime() const noexcept { return m_targetFrameTime; }

        /// Sets the target frame time.  Pass 0.0 to disable limiting.
        void SetTargetFrameTime(f64 seconds) noexcept;

        /// Returns true when the frame should be yielded to meet the target.
        /// Call after EndFrame() to determine whether to sleep.
        [[nodiscard]] bool ShouldWaitForNextFrame() const noexcept;

        // --------------------------------------------------------------------
        // Pause / Resume
        // --------------------------------------------------------------------

        /// Returns true when the engine is paused.
        [[nodiscard]] bool IsPaused() const noexcept { return m_paused; }

        /// Pauses the runtime loop.  Delta time will accumulate to zero.
        void Pause() noexcept;

        /// Resumes the runtime loop.
        void Resume() noexcept;

        // --------------------------------------------------------------------
        // Tick — called once per frame by the Engine main loop.
        // --------------------------------------------------------------------

        /// Advances the clock by measuring time since the last call.
        /// Must be called exactly once per frame.
        void Tick();

        /// Marks the end of the current frame and increments frame counter.
        /// Called after all updates (update, fixed update, late update).
        void EndFrame();

        /// Resets all counters and timers to their initial state.
        void Reset();

    private:
        // Timing
        core::TimePoint m_lastFrameTime{};
        f64              m_deltaTime          = 0.0;
        f64              m_unscaledDeltaTime  = 0.0;
        f64              m_totalTime          = 0.0;
        f64              m_unscaledTotalTime  = 0.0;
        f64              m_timeScale          = 1.0;
        core::TimePoint m_frameEndTime{};

        // Frame counting
        u64  m_frameCount     = 0;
        u64  m_fixedTickCount = 0;

        // Fixed timestep accumulator
        f64   m_fixedDeltaTime     = 1.0 / 60.0;
        f64   m_fixedAccumulator   = 0.0;
        usize m_maxFixedSteps      = 5;

        // Frame rate limiting
        f64 m_targetFrameTime = 0.0;

        // Pause state
        bool m_paused = false;
    };

} // namespace engine::runtime