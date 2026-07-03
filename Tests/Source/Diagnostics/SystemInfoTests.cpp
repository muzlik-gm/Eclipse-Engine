// ============================================================================
// File: Tests/Source/Diagnostics/SystemInfoTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Diagnostics/SystemInfo.h"

using namespace engine::diagnostics;

TEST(SystemInfoTest, HostNameNotEmpty)
{
    std::string host = SystemInfo::HostName();
    EXPECT_FALSE(host.empty());
}

TEST(SystemInfoTest, UserNameNotEmpty)
{
    std::string user = SystemInfo::UserName();
    EXPECT_FALSE(user.empty());
}

TEST(SystemInfoTest, LogicalCoreCountPositive)
{
    u32 cores = SystemInfo::LogicalCoreCount();
    EXPECT_GT(cores, 0u);
}

TEST(SystemInfoTest, TotalPhysicalMemoryPositive)
{
    u64 mem = SystemInfo::TotalPhysicalMemoryBytes();
    EXPECT_GT(mem, 0u);
}

TEST(SystemInfoTest, OSNameNotEmpty)
{
    std::string os = SystemInfo::OSName();
    EXPECT_FALSE(os.empty());
}

TEST(SystemInfoTest, ArchitectureNotEmpty)
{
    std::string arch = SystemInfo::Architecture();
    EXPECT_FALSE(arch.empty());
}