#include "Engine/Filesystem/File.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
#endif

namespace engine::fs
{

    using engine::core::i64;
    using engine::core::u64;
    using engine::core::u8;
    using engine::core::usize;

    // ========================================================================
    //  File existence and management
    // ========================================================================

    bool File::Exists(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::is_regular_file(
            static_cast<std::filesystem::path>(path), ec);
    }

    bool File::Delete(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::remove(
            static_cast<std::filesystem::path>(path), ec);
    }

    bool File::Copy(const Path& src, const Path& dst, bool overwrite)
    {
        std::error_code ec;
        auto fsDst = static_cast<std::filesystem::path>(dst);

        if (!overwrite && std::filesystem::exists(fsDst, ec))
        {
            return false;
        }

        std::filesystem::copy_options options =
            std::filesystem::copy_options::overwrite_existing;

        return std::filesystem::copy_file(
            static_cast<std::filesystem::path>(src), fsDst, options, ec);
    }

    bool File::Move(const Path& src, const Path& dst)
    {
        std::error_code ec;
        std::filesystem::rename(
            static_cast<std::filesystem::path>(src),
            static_cast<std::filesystem::path>(dst), ec);
        return !ec;
    }

    bool File::Rename(const Path& path, const Path& newName)
    {
        const Path newPath = path.Parent() / newName.String();
        return Move(path, newPath);
    }

    // ========================================================================
    //  File metadata
    // ========================================================================

    u64 File::Size(const Path& path)
    {
        std::error_code ec;
        const auto size = std::filesystem::file_size(
            static_cast<std::filesystem::path>(path), ec);
        if (ec)
        {
            return 0;
        }
        return static_cast<u64>(size);
    }

    i64 File::LastModifiedTime(const Path& path)
    {
        std::error_code ec;
        auto ftime = std::filesystem::last_write_time(
            static_cast<std::filesystem::path>(path), ec);
        if (ec)
        {
            return -1;
        }

        // Convert file_time_type to a Unix timestamp.
        // On C++20, file_time_type is typically time_point with seconds since
        // the filesystem clock epoch.  We convert to system_clock and then
        // to time_t.
        const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now()
                + std::chrono::system_clock::now());

        return static_cast<i64>(
            std::chrono::duration_cast<std::chrono::seconds>(
                sctp.time_since_epoch())
                .count());
    }

    bool File::Touch(const Path& path)
    {
        if (!Exists(path))
        {
            // Create an empty file.
            std::ofstream ofs(path.String(), std::ios::binary | std::ios::out);
            return ofs.good();
        }

        // Update modification time by reading and writing the first byte.
        // A simpler approach: use the filesystem library to set the write
        // time to "now" via a copy-overwrite trick with a temp file, or
        // just open in append mode which updates mtime on most platforms.
        {
            std::ofstream ofs(path.String(),
                              std::ios::binary | std::ios::app);
            if (!ofs.good())
            {
                return false;
            }
            // Opening in append mode already updates mtime on POSIX.
            // On Windows, we write nothing and the open itself may not
            // update the time — use a platform-specific path instead.
        }

#if ENGINE_PLATFORM_WINDOWS
        // On Windows, use the Win32 API to set the file time.
        HANDLE hFile = ::CreateFileA(
            path.CStr(),
            FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        SYSTEMTIME st;
        ::GetSystemTime(&st);

        FILETIME ft;
        ::SystemTimeToFileTime(&st, &ft);

        BOOL result = ::SetFileTime(hFile, nullptr, nullptr, &ft);
        ::CloseHandle(hFile);
        return result != FALSE;
#else
        // On POSIX, opening with O_WRONLY and calling futimens sets mtime.
        // The append-mode open above already suffices on most systems.
        return true;
#endif
    }

    // ========================================================================
    //  Text I/O
    // ========================================================================

    std::string File::ReadText(const Path& path)
    {
        std::ifstream ifs(path.String(), std::ios::in | std::ios::binary);
        if (!ifs.good())
        {
            return {};
        }

        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
    }

    bool File::WriteText(const Path& path, std::string_view content)
    {
        std::ofstream ofs(path.String(),
                          std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofs.good())
        {
            return false;
        }

        ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
        return ofs.good();
    }

    bool File::AppendText(const Path& path, std::string_view content)
    {
        std::ofstream ofs(path.String(),
                          std::ios::out | std::ios::binary | std::ios::app);
        if (!ofs.good())
        {
            return false;
        }

        ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
        return ofs.good();
    }

    // ========================================================================
    //  Binary I/O
    // ========================================================================

    std::vector<u8> File::ReadBinary(const Path& path)
    {
        std::ifstream ifs(path.String(),
                          std::ios::in | std::ios::binary | std::ios::ate);
        if (!ifs.good())
        {
            return {};
        }

        const auto size = ifs.tellg();
        if (size <= 0)
        {
            return {};
        }

        std::vector<u8> buffer(static_cast<usize>(size));
        ifs.seekg(0, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(buffer.data()), size);
        if (!ifs.good() && !ifs.eof())
        {
            return {};
        }

        return buffer;
    }

    bool File::WriteBinary(const Path& path, const std::vector<u8>& data)
    {
        std::ofstream ofs(path.String(),
                          std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofs.good())
        {
            return false;
        }

        ofs.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
        return ofs.good();
    }

    bool File::AppendBinary(const Path& path, const std::vector<u8>& data)
    {
        std::ofstream ofs(path.String(),
                          std::ios::out | std::ios::binary | std::ios::app);
        if (!ofs.good())
        {
            return false;
        }

        ofs.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
        return ofs.good();
    }

    // ========================================================================
    //  Line-level reading
    // ========================================================================

    std::string File::ReadLine(const Path& path, usize lineIndex)
    {
        std::ifstream ifs(path.String(), std::ios::in | std::ios::binary);
        if (!ifs.good())
        {
            return {};
        }

        std::string line;
        for (usize i = 0; i <= lineIndex; ++i)
        {
            if (!std::getline(ifs, line))
            {
                return {};
            }
        }

        // Strip trailing \r if present (Windows line endings).
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        return line;
    }

    std::vector<std::string> File::ReadLines(const Path& path)
    {
        std::vector<std::string> lines;

        std::ifstream ifs(path.String(), std::ios::in | std::ios::binary);
        if (!ifs.good())
        {
            return lines;
        }

        std::string line;
        while (std::getline(ifs, line))
        {
            // Strip trailing \r for consistency.
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            lines.push_back(std::move(line));
        }

        return lines;
    }

} // namespace engine::fs