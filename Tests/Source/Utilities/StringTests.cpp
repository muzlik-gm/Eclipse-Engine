#include <gtest/gtest.h>

#include "Engine/Utilities/String.h"

using namespace engine::util;

TEST(StringTest, StartsWith)
{
    EXPECT_TRUE(StartsWith("HelloWorld", "Hello"));
    EXPECT_TRUE(StartsWith("HelloWorld", ""));
    EXPECT_FALSE(StartsWith("HelloWorld", "World"));
    EXPECT_FALSE(StartsWith("Hello", "HelloWorld"));
}

TEST(StringTest, EndsWith)
{
    EXPECT_TRUE(EndsWith("HelloWorld", "World"));
    EXPECT_TRUE(EndsWith("HelloWorld", ""));
    EXPECT_FALSE(EndsWith("HelloWorld", "Hello"));
    EXPECT_FALSE(EndsWith("World", "HelloWorld"));
}

TEST(StringTest, Contains)
{
    EXPECT_TRUE(Contains("HelloWorld", "loWo"));
    EXPECT_TRUE(Contains("HelloWorld", ""));
    EXPECT_FALSE(Contains("HelloWorld", "xyz"));
}

TEST(StringTest, EqualsIgnoreCase)
{
    EXPECT_TRUE(EqualsIgnoreCase("Hello", "hello"));
    EXPECT_TRUE(EqualsIgnoreCase("hello", "HELLO"));
    EXPECT_FALSE(EqualsIgnoreCase("Hello", "World"));
    EXPECT_FALSE(EqualsIgnoreCase("Hello", "Hell"));
}

TEST(StringTest, ToLower)
{
    EXPECT_EQ(ToLower("HELLO"), "hello");
    EXPECT_EQ(ToLower("Hello World"), "hello world");
    EXPECT_EQ(ToLower(""), "");
}

TEST(StringTest, ToUpper)
{
    EXPECT_EQ(ToUpper("hello"), "HELLO");
    EXPECT_EQ(ToUpper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(ToUpper(""), "");
}

TEST(StringTest, Trim)
{
    EXPECT_EQ(Trim("  hello  "), "hello");
    EXPECT_EQ(Trim("\thello\n"), "hello");
    EXPECT_EQ(Trim("hello"), "hello");
    EXPECT_EQ(Trim("   "), "");
}

TEST(StringTest, Split)
{
    auto parts = Split("a,b,c", ',');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(StringTest, Join)
{
    std::vector<std::string> parts = {"a", "b", "c"};
    EXPECT_EQ(Join(parts, ","), "a,b,c");
    EXPECT_EQ(Join(parts, "-"), "a-b-c");
}

TEST(StringTest, Replace)
{
    std::string result = Replace("foo is foo", "foo", "bar");
    EXPECT_EQ(result, "bar is bar");
}