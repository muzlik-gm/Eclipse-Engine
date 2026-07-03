// ============================================================================
// File: Tests/Source/Filesystem/FileTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Core/Log.h"
#include "Engine/Filesystem/File.h"
#include "Engine/Filesystem/Path.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace engine::fs;

class FileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine::core::Log::Initialize("FileTests");
        m_tempDir = Path(std::filesystem::temp_directory_path() / "engine_file_tests");
        std::filesystem::create_directories(std::filesystem::path(m_tempDir));
    }

    void TearDown() override
    {
        std::filesystem::remove_all(std::filesystem::path(m_tempDir));
        engine::core::Log::Shutdown();
    }

    Path m_tempDir;

    /// Helper to build a test file path within the temp directory.
    Path TestFile(std::string_view name) const
    {
        return m_tempDir / name;
    }
};

TEST_F(FileTest, ReadWriteText)
{
    auto path = TestFile("rw_text.txt");
    std::string content = "Hello, Engine!";

    ASSERT_TRUE(File::WriteText(path, content));
    std::string read = File::ReadText(path);
    EXPECT_EQ(read, content);
}

TEST_F(FileTest, AppendText)
{
    auto path = TestFile("append_text.txt");

    ASSERT_TRUE(File::WriteText(path, "First"));
    ASSERT_TRUE(File::AppendText(path, "Second"));

    std::string read = File::ReadText(path);
    EXPECT_EQ(read, "FirstSecond");
}

TEST_F(FileTest, ReadWriteBinary)
{
    auto path = TestFile("rw_binary.bin");
    std::vector<u8> data = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xAB};

    ASSERT_TRUE(File::WriteBinary(path, data));
    auto read = File::ReadBinary(path);
    EXPECT_EQ(read, data);
}

TEST_F(FileTest, Exists)
{
    auto path = TestFile("exists_check.txt");

    EXPECT_FALSE(File::Exists(path));

    ASSERT_TRUE(File::WriteText(path, "data"));
    EXPECT_TRUE(File::Exists(path));
}

TEST_F(FileTest, Copy)
{
    auto src  = TestFile("copy_src.txt");
    auto dst  = TestFile("copy_dst.txt");
    std::string content = "copy me";

    ASSERT_TRUE(File::WriteText(src, content));
    ASSERT_TRUE(File::Copy(src, dst));

    EXPECT_TRUE(File::Exists(src));
    EXPECT_TRUE(File::Exists(dst));

    std::string copied = File::ReadText(dst);
    EXPECT_EQ(copied, content);
}

TEST_F(FileTest, Move)
{
    auto src = TestFile("move_src.txt");
    auto dst = TestFile("move_dst.txt");
    std::string content = "move me";

    ASSERT_TRUE(File::WriteText(src, content));
    ASSERT_TRUE(File::Move(src, dst));

    EXPECT_FALSE(File::Exists(src));
    EXPECT_TRUE(File::Exists(dst));

    std::string moved = File::ReadText(dst);
    EXPECT_EQ(moved, content);
}

TEST_F(FileTest, Size)
{
    auto path = TestFile("size_check.txt");
    std::string content = "12345"; // 5 bytes

    ASSERT_TRUE(File::WriteText(path, content));
    EXPECT_EQ(File::Size(path), 5u);
}

TEST_F(FileTest, ReadLines)
{
    auto path = TestFile("lines.txt");
    std::string content = "line1\nline2\nline3";

    ASSERT_TRUE(File::WriteText(path, content));

    auto lines = File::ReadLines(path);
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "line1");
    EXPECT_EQ(lines[1], "line2");
    EXPECT_EQ(lines[2], "line3");
}

TEST_F(FileTest, Delete)
{
    auto path = TestFile("delete_me.txt");

    ASSERT_TRUE(File::WriteText(path, "gone soon"));
    EXPECT_TRUE(File::Exists(path));

    ASSERT_TRUE(File::Delete(path));
    EXPECT_FALSE(File::Exists(path));
}