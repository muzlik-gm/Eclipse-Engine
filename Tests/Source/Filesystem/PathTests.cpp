#include <gtest/gtest.h>

#include "Engine/Filesystem/Path.h"

using namespace engine::fs;

TEST(PathTest, DefaultConstructor)
{
    Path p;
    EXPECT_TRUE(p.IsEmpty());
}

TEST(PathTest, FromString)
{
    Path p("foo/bar");
    EXPECT_FALSE(p.IsEmpty());
}

TEST(PathTest, Append)
{
    Path p("a");
    Path result = p / "b";
    EXPECT_EQ(result.String(), "a/b");
}

TEST(PathTest, Extension)
{
    Path p("test.cpp");
    EXPECT_EQ(p.Extension(), ".cpp");
}

TEST(PathTest, Stem)
{
    Path p("test.cpp");
    EXPECT_EQ(p.Stem(), "test");
}

TEST(PathTest, Parent)
{
    Path p("a/b/c");
    Path parent = p.Parent();
    EXPECT_EQ(parent.String(), "a/b");
}

TEST(PathTest, CurrentWorkingDirectory)
{
    Path cwd = Path::CurrentWorkingDirectory();
    EXPECT_TRUE(cwd.Exists());
    EXPECT_TRUE(cwd.IsDirectory());
}

TEST(PathTest, Equality)
{
    Path a("foo/bar");
    Path b("foo/bar");
    Path c("foo/baz");

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}