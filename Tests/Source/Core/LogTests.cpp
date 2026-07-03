#include <gtest/gtest.h>

#include "Engine/Core/Log.h"

using namespace engine::core;

TEST(LogTest, InitializeAndShutdown)
{
    Log::Initialize("LogTest");
    // If we get here without crashing, initialization succeeded.
    EXPECT_TRUE(Log::GetCoreLogger() != nullptr);
    Log::Shutdown();
}

TEST(LogTest, SetLevel)
{
    Log::Initialize("LogTest");

    Log::SetLevel(LogLevel::Trace);
    EXPECT_EQ(Log::GetLevel(), LogLevel::Trace);

    Log::SetLevel(LogLevel::Warning);
    EXPECT_EQ(Log::GetLevel(), LogLevel::Warning);

    Log::SetLevel(LogLevel::Off);
    EXPECT_EQ(Log::GetLevel(), LogLevel::Off);

    Log::Shutdown();
}

TEST(LogTest, MultipleInitialize)
{
    // Calling Initialize twice should be safe (idempotent).
    Log::Initialize("LogTest");
    Log::Initialize("LogTest");
    EXPECT_TRUE(Log::GetCoreLogger() != nullptr);
    Log::Shutdown();
}