#include <gtest/gtest.h>

#include "Engine/Utilities/CommandLine.h"

using namespace engine::util;

TEST(CommandLineTest, HasFlag)
{
    const char* argv[] = {"program", "--verbose"};
    CommandLine cmd(2, argv);

    EXPECT_TRUE(cmd.Has("verbose"));
    EXPECT_FALSE(cmd.Has("quiet"));
}

TEST(CommandLineTest, GetValue)
{
    const char* argv[] = {"program", "--name", "test", "--count", "5"};
    CommandLine cmd(5, argv);

    EXPECT_EQ(cmd.Get("name"), "test");
    EXPECT_EQ(cmd.GetInt("count"), 5);
}

TEST(CommandLineTest, MissingValue)
{
    const char* argv[] = {"program"};
    CommandLine cmd(1, argv);

    EXPECT_EQ(cmd.Get("missing", "default"), "default");
    EXPECT_EQ(cmd.GetInt("missing", 99), 99);
}