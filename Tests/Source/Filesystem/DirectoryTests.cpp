// ============================================================================
// File: Tests/Source/Filesystem/DirectoryTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Core/Log.h"
#include "Engine/Filesystem/Directory.h"
#include "Engine/Filesystem/File.h"
#include "Engine/Filesystem/Path.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace engine::fs;

class DirectoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine::core::Log::Initialize("DirectoryTests");
        m_tempDir = Path(std::filesystem::temp_directory_path() / "engine_dir_tests");
        std::filesystem::create_directories(std::filesystem::path(m_tempDir));
    }

    void TearDown() override
    {
        std::filesystem::remove_all(std::filesystem::path(m_tempDir));
        engine::core::Log::Shutdown();
    }

    Path m_tempDir;

    Path SubDir(std::string_view name) const
    {
        return m_tempDir / name;
    }
};

TEST_F(DirectoryTest, CreateAndExists)
{
    auto dir = SubDir("new_dir");
    EXPECT_FALSE(Directory::Exists(dir));

    ASSERT_TRUE(Directory::Create(dir));
    EXPECT_TRUE(Directory::Exists(dir));
}

TEST_F(DirectoryTest, CreateAll)
{
    auto nested = SubDir("a/b/c/d");
    EXPECT_FALSE(Directory::Exists(nested));

    ASSERT_TRUE(Directory::CreateAll(nested));
    EXPECT_TRUE(Directory::Exists(nested));
}

TEST_F(DirectoryTest, Remove)
{
    auto dir = SubDir("to_remove");
    ASSERT_TRUE(Directory::Create(dir));
    EXPECT_TRUE(Directory::Exists(dir));

    ASSERT_TRUE(Directory::Remove(dir));
    EXPECT_FALSE(Directory::Exists(dir));
}

TEST_F(DirectoryTest, RemoveAll)
{
    auto dir = SubDir("remove_all_dir");
    ASSERT_TRUE(Directory::Create(dir));

    // Populate with a file.
    File::WriteText(dir / "inner.txt", "data");

    EXPECT_TRUE(Directory::Exists(dir));
    ASSERT_TRUE(Directory::RemoveAll(dir));
    EXPECT_FALSE(Directory::Exists(dir));
}

TEST_F(DirectoryTest, Files)
{
    auto dir = SubDir("files_dir");
    ASSERT_TRUE(Directory::Create(dir));

    File::WriteText(dir / "a.txt", "a");
    File::WriteText(dir / "b.txt", "b");

    auto files = Directory::Files(dir);
    ASSERT_EQ(files.size(), 2u);
}

TEST_F(DirectoryTest, Subdirectories)
{
    auto parent = SubDir("subdirs_parent");
    ASSERT_TRUE(Directory::Create(parent));
    ASSERT_TRUE(Directory::Create(parent / "child1"));
    ASSERT_TRUE(Directory::Create(parent / "child2"));

    auto subs = Directory::Subdirectories(parent);
    ASSERT_EQ(subs.size(), 2u);
}

TEST_F(DirectoryTest, IsEmpty)
{
    auto dir = SubDir("empty_dir");
    ASSERT_TRUE(Directory::Create(dir));

    EXPECT_TRUE(Directory::IsEmpty(dir));

    File::WriteText(dir / "file.txt", "data");
    EXPECT_FALSE(Directory::IsEmpty(dir));
}

TEST_F(DirectoryTest, FilesRecursive)
{
    auto root = SubDir("recursive_root");
    ASSERT_TRUE(Directory::Create(root));

    auto sub = root / "sub";
    ASSERT_TRUE(Directory::Create(sub));

    File::WriteText(root / "top.txt", "top");
    File::WriteText(sub / "deep.txt", "deep");

    auto files = Directory::FilesRecursive(root);
    ASSERT_EQ(files.size(), 2u);
}

TEST_F(DirectoryTest, CurrentDirectory)
{
    Path cwd = Directory::Current();
    EXPECT_FALSE(cwd.String().empty());
    EXPECT_TRUE(Directory::Exists(cwd));
}