#include <gtest/gtest.h>

#include "Engine/Core/Timing.h"

#include <chrono>
#include <thread>

using namespace engine::core;

TEST(TimingTest, TimerBasic)
{
    Timer timer;
    timer.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    timer.Stop();

    f64 elapsedMs = timer.ElapsedMilliseconds();
    // Allow some slack for scheduler jitter; 45 ms is a safe lower bound.
    EXPECT_GE(elapsedMs, 45.0);
}

TEST(TimingTest, TimerReset)
{
    Timer timer;
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.Stop();

    EXPECT_GT(timer.ElapsedMilliseconds(), 0.0);

    timer.Reset();
    EXPECT_DOUBLE_EQ(timer.ElapsedMilliseconds(), 0.0);
}

TEST(TimingTest, ClockNow)
{
    TimePoint now = Clock::Now();
    // A default-constructed time_point is the epoch; the current time should
    // be well past that.
    TimePoint epoch{};
    EXPECT_GT(now, epoch);
}

TEST(TimingTest, ClockTimestamp)
{
    f64 ts = Clock::Timestamp();
    // The timestamp (seconds since epoch) should be a large positive value.
    EXPECT_GT(ts, 1'000'000'000.0);  // After ~Sep 2001
}