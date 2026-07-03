// ============================================================================
// File: Engine/Source/Runtime/FrameStats.cpp
// Implementation of frame timing, fixed-timestep accumulator, and
// runtime statistics.
// ============================================================================

#include "Engine/Runtime/FrameStats.h"

#include <algorithm>

namespace engine::runtime
{

    // ========================================================================
    // Time scale
    // ========================================================================

    void FrameStats::SetTimeScale(f64 scale) noexcept
    {
        m_timeScale = std::clamp(scale, 0.0, 100.0);
    }

    // ========================================================================
    // Fixed timestep
    // ========================================================================

    void FrameStats::SetFixedDeltaTime(f64 dt) noexcept
    {
        if (dt > 0.0)
        {
            m_fixedDeltaTime = dt;
        }
    }

    void FrameStats::SetMaxFixedStepsPerFrame(usize steps) noexcept
    {
        if (steps >= 1)
        {
            m_maxFixedSteps = steps;
        }
    }

    bool FrameStats::ShouldFixedUpdate() const noexcept
    {
        return m_fixedAccumulator >= m_fixedDeltaTime;
    }

    void FrameStats::ConsumeFixedStep() noexcept
    {
        if (m_fixedAccumulator >= m_fixedDeltaTime)
        {
            m_fixedAccumulator -= m_fixedDeltaTime;
            ++m_fixedTickCount;
        }
    }

    // ========================================================================
    // Frame rate limiting
    // ========================================================================

    void FrameStats::SetTargetFrameTime(f64 seconds) noexcept
    {
        m_targetFrameTime = std::max(seconds, 0.0);
    }

    bool FrameStats::ShouldWaitForNextFrame() const noexcept
    {
        if (m_targetFrameTime <= 0.0)
        {
            return false;
        }

        const f64 elapsed = core::Clock::SecondsSince(m_frameEndTime);
        return elapsed < m_targetFrameTime;
    }

    // ========================================================================
    // Pause / Resume
    // ========================================================================

    void FrameStats::Pause() noexcept
    {
        m_paused = true;
    }

    void FrameStats::Resume() noexcept
    {
        if (m_paused)
        {
            // Reset the last-frame time so the first frame after resume
            // does not get a huge delta.
            m_lastFrameTime = core::Clock::Now();
            m_paused = false;
        }
    }

    // ========================================================================
    // Tick
    // ========================================================================

    void FrameStats::Tick()
    {
        const core::TimePoint now = core::Clock::Now();

        if (m_frameCount == 0)
        {
            // First frame — establish the baseline.
            m_lastFrameTime = now;
            m_frameEndTime  = now;
            m_deltaTime         = 0.0;
            m_unscaledDeltaTime = 0.0;
            return;
        }

        const f64 rawDelta = core::Clock::SecondsSince(m_lastFrameTime);
        m_lastFrameTime = now;

        // Clamp to prevent huge spikes (e.g. after debugger pause).
        const f64 maxDelta = 0.25; // 250 ms
        m_unscaledDeltaTime = std::min(rawDelta, maxDelta);
        m_deltaTime = m_unscaledDeltaTime * m_timeScale;

        // Advance totals.
        m_unscaledTotalTime += m_unscaledDeltaTime;
        if (!m_paused)
        {
            m_totalTime += m_deltaTime;
            m_fixedAccumulator += m_unscaledDeltaTime;
        }
    }

    void FrameStats::EndFrame()
    {
        m_frameEndTime = core::Clock::Now();
        ++m_frameCount;
    }

    void FrameStats::Reset()
    {
        m_lastFrameTime      = {};
        m_frameEndTime       = {};
        m_deltaTime          = 0.0;
        m_unscaledDeltaTime  = 0.0;
        m_totalTime          = 0.0;
        m_unscaledTotalTime  = 0.0;
        m_timeScale          = 1.0;
        m_frameCount         = 0;
        m_fixedTickCount     = 0;
        m_fixedAccumulator   = 0.0;
        m_paused             = false;
    }

} // namespace engine::runtime