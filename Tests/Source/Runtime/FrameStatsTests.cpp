// ============================================================================
// File: Tests/Source/Runtime/FrameStatsTests.cpp
// Tests for frame timing, fixed-timestep accumulator, and frame stats.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Runtime/FrameStats.h"
#include "Engine/Core/Timing.h"

#include <thread>

using namespace engine::runtime;
using namespace engine::core;

// ============================================================================
// Defaults
// ============================================================================

TEST(FrameStatsTest, DefaultValues)
{
    FrameStats stats;

    EXPECT_DOUBLE_EQ(stats.DeltaTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.UnscaledDeltaTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.TotalTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.UnscaledTotalTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.TimeScale(), 1.0);
    EXPECT_DOUBLE_EQ(stats.FixedDeltaTime(), 1.0 / 60.0);
    EXPECT_EQ(stats.FrameCount(), 0u);
    EXPECT_EQ(stats.FixedTickCount(), 0u);
    EXPECT_EQ(stats.MaxFixedStepsPerFrame(), 5u);
    EXPECT_DOUBLE_EQ(stats.TargetFrameTime(), 0.0);
    EXPECT_FALSE(stats.IsPaused());
}

// ============================================================================
// Time scale
// ============================================================================

TEST(FrameStatsTest, SetTimeScale)
{
    FrameStats stats;
    stats.SetTimeScale(2.0);
    EXPECT_DOUBLE_EQ(stats.TimeScale(), 2.0);

    // Clamped to upper bound.
    stats.SetTimeScale(200.0);
    EXPECT_DOUBLE_EQ(stats.TimeScale(), 100.0);

    // Clamped to lower bound.
    stats.SetTimeScale(-1.0);
    EXPECT_DOUBLE_EQ(stats.TimeScale(), 0.0);
}

// ============================================================================
// Fixed timestep
// ============================================================================

TEST(FrameStatsTest, SetFixedDeltaTime)
{
    FrameStats stats;
    stats.SetFixedDeltaTime(1.0 / 30.0);
    EXPECT_DOUBLE_EQ(stats.FixedDeltaTime(), 1.0 / 30.0);

    // Non-positive values are rejected.
    stats.SetFixedDeltaTime(0.0);
    EXPECT_DOUBLE_EQ(stats.FixedDeltaTime(), 1.0 / 30.0);

    stats.SetFixedDeltaTime(-1.0);
    EXPECT_DOUBLE_EQ(stats.FixedDeltaTime(), 1.0 / 30.0);
}

TEST(FrameStatsTest, SetMaxFixedSteps)
{
    FrameStats stats;
    stats.SetMaxFixedStepsPerFrame(10);
    EXPECT_EQ(stats.MaxFixedStepsPerFrame(), 10u);

    // Values < 1 are rejected.
    stats.SetMaxFixedStepsPerFrame(0);
    EXPECT_EQ(stats.MaxFixedStepsPerFrame(), 10u);
}

// ============================================================================
// Frame rate limiting
// ============================================================================

TEST(FrameStatsTest, TargetFrameTime)
{
    FrameStats stats;
    EXPECT_DOUBLE_EQ(stats.TargetFrameTime(), 0.0);

    stats.SetTargetFrameTime(1.0 / 60.0);
    EXPECT_DOUBLE_EQ(stats.TargetFrameTime(), 1.0 / 60.0);

    // Negative values are clamped to 0 (disabled).
    stats.SetTargetFrameTime(-1.0);
    EXPECT_DOUBLE_EQ(stats.TargetFrameTime(), 0.0);
}

// ============================================================================
// Pause / Resume
// ============================================================================

TEST(FrameStatsTest, PauseResume)
{
    FrameStats stats;
    EXPECT_FALSE(stats.IsPaused());

    stats.Pause();
    EXPECT_TRUE(stats.IsPaused());

    stats.Resume();
    EXPECT_FALSE(stats.IsPaused());
}

// ============================================================================
// Tick / EndFrame
// ============================================================================

TEST(FrameStatsTest, TickEndFrameBasic)
{
    FrameStats stats;

    // First tick should have zero delta.
    stats.Tick();
    EXPECT_DOUBLE_EQ(stats.DeltaTime(), 0.0);
    stats.EndFrame();
    EXPECT_EQ(stats.FrameCount(), 1u);

    // Sleep briefly, then tick again.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stats.Tick();
    stats.EndFrame();
    EXPECT_EQ(stats.FrameCount(), 2u);
    EXPECT_GT(stats.DeltaTime(), 0.0);
    EXPECT_GT(stats.UnscaledDeltaTime(), 0.0);
}

TEST(FrameStatsTest, Reset)
{
    FrameStats stats;
    stats.Tick();
    stats.EndFrame();
    stats.SetTimeScale(5.0);

    stats.Reset();

    EXPECT_EQ(stats.FrameCount(), 0u);
    EXPECT_EQ(stats.FixedTickCount(), 0u);
    EXPECT_DOUBLE_EQ(stats.DeltaTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.TotalTime(), 0.0);
    EXPECT_DOUBLE_EQ(stats.TimeScale(), 1.0);
    EXPECT_FALSE(stats.IsPaused());
}