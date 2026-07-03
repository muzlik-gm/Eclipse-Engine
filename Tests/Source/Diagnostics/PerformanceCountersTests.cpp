// ============================================================================
// File: Tests/Source/Diagnostics/PerformanceCountersTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Diagnostics/PerformanceCounters.h"

using namespace engine::diagnostics;
using CounterId = PerformanceCounters::CounterId;

TEST(PerformanceCountersTest, RegisterAndIncrement)
{
    CounterId id = PerformanceCounters::RegisterCounter("TestCounter");
    PerformanceCounters::ResetAll(); // ensure clean state

    PerformanceCounters::Increment(id, 42);
    EXPECT_EQ(PerformanceCounters::GetValue(id), 42u);

    PerformanceCounters::ResetAll();
}

TEST(PerformanceCountersTest, MultipleCounters)
{
    CounterId a = PerformanceCounters::RegisterCounter("CounterA");
    CounterId b = PerformanceCounters::RegisterCounter("CounterB");
    CounterId c = PerformanceCounters::RegisterCounter("CounterC");
    PerformanceCounters::ResetAll();

    PerformanceCounters::Increment(a, 10);
    PerformanceCounters::Increment(b, 20);
    PerformanceCounters::Increment(c, 30);

    EXPECT_EQ(PerformanceCounters::GetValue(a), 10u);
    EXPECT_EQ(PerformanceCounters::GetValue(b), 20u);
    EXPECT_EQ(PerformanceCounters::GetValue(c), 30u);

    PerformanceCounters::ResetAll();
}

TEST(PerformanceCountersTest, ResetCounter)
{
    CounterId id = PerformanceCounters::RegisterCounter("ResetMe");
    PerformanceCounters::ResetAll();

    PerformanceCounters::Increment(id, 100);
    EXPECT_EQ(PerformanceCounters::GetValue(id), 100u);

    PerformanceCounters::Reset(id);
    EXPECT_EQ(PerformanceCounters::GetValue(id), 0u);
    EXPECT_EQ(PerformanceCounters::GetHitCount(id), 0u);

    PerformanceCounters::ResetAll();
}

TEST(PerformanceCountersTest, ResetAll)
{
    CounterId x = PerformanceCounters::RegisterCounter("ResetAll_X");
    CounterId y = PerformanceCounters::RegisterCounter("ResetAll_Y");
    PerformanceCounters::ResetAll();

    PerformanceCounters::Increment(x, 5);
    PerformanceCounters::Increment(y, 15);

    PerformanceCounters::ResetAll();

    EXPECT_EQ(PerformanceCounters::GetValue(x), 0u);
    EXPECT_EQ(PerformanceCounters::GetValue(y), 0u);
}

TEST(PerformanceCountersTest, DuplicateName)
{
    CounterId first  = PerformanceCounters::RegisterCounter("DupeName");
    CounterId second = PerformanceCounters::RegisterCounter("DupeName");

    EXPECT_EQ(first, second);
}

TEST(PerformanceCountersTest, HitCount)
{
    CounterId id = PerformanceCounters::RegisterCounter("HitCounter");
    PerformanceCounters::ResetAll();

    PerformanceCounters::Increment(id);
    PerformanceCounters::Increment(id);
    PerformanceCounters::Increment(id);

    EXPECT_EQ(PerformanceCounters::GetHitCount(id), 3u);

    PerformanceCounters::ResetAll();
}

TEST(PerformanceCountersTest, Report)
{
    CounterId id = PerformanceCounters::RegisterCounter("ReportCounter");
    PerformanceCounters::ResetAll();

    PerformanceCounters::Increment(id, 7);

    std::string report = PerformanceCounters::Report();
    EXPECT_FALSE(report.empty());
    // The report should contain the counter name.
    EXPECT_NE(report.find("ReportCounter"), std::string::npos);

    PerformanceCounters::ResetAll();
}