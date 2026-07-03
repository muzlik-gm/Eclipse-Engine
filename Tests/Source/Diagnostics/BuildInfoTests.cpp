// ============================================================================
// File: Tests/Source/Diagnostics/BuildInfoTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Diagnostics/BuildInfo.h"

#include <string>

using namespace engine::diagnostics;

TEST(BuildInfoTest, EngineName)
{
    EXPECT_EQ(BuildInfo::EngineName(), "Engine");
}

TEST(BuildInfoTest, VersionString)
{
    EXPECT_EQ(BuildInfo::VersionString(), "0.1.0");
}

TEST(BuildInfoTest, BuildDateNotEmpty)
{
    std::string_view date = BuildInfo::BuildDate();
    EXPECT_FALSE(date.empty());
}

TEST(BuildInfoTest, BuildTimeNotEmpty)
{
    std::string_view time = BuildInfo::BuildTime();
    EXPECT_FALSE(time.empty());
}

TEST(BuildInfoTest, CompilerNameNotEmpty)
{
    std::string_view compiler = BuildInfo::CompilerName();
    EXPECT_FALSE(compiler.empty());
}

TEST(BuildInfoTest, PlatformNameNotEmpty)
{
    std::string_view platform = BuildInfo::PlatformName();
    EXPECT_FALSE(platform.empty());
}

TEST(BuildInfoTest, FullBuildInfoString)
{
    std::string info = BuildInfo::GetFullBuildInfoString();
    EXPECT_FALSE(info.empty());

    // The full string should contain the engine name and version.
    EXPECT_NE(info.find("Engine"), std::string::npos);
    EXPECT_NE(info.find("0.1.0"), std::string::npos);
}