#include "Engine/Filesystem/Path.h"

#include <array>
#include <sstream>

#if ENGINE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
#elif ENGINE_PLATFORM_LINUX
    #include <unistd.h>
#elif ENGINE_PLATFORM_MACOS
    #include <mach-o/dyld.h>
    #include <unistd.h>
#endif

namespace engine::fs
{

    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    //  Construction
    // ========================================================================

    Path::Path(std::string_view path)
        : m_path(path)
    {
    }

    Path::Path(const std::string& path)
        : m_path(path)
    {
    }

    Path::Path(const std::filesystem::path& path)
        : m_path(path)
    {
    }

    Path::Path(const char* path)
        : m_path(path ? path : "")
    {
    }

    // ========================================================================
    //  Static factories
    // ========================================================================

    Path Path::CurrentWorkingDirectory()
    {
        return Path(std::filesystem::current_path());
    }

    Path Path::ExecutablePath()
    {
#if ENGINE_PLATFORM_LINUX
        std::array<char, 4096> buffer{};
        const ssize_t len = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
        if (len > 0)
        {
            buffer[static_cast<usize>(len)] = '\0';
            return Path(buffer.data());
        }
        return Path{};
#elif ENGINE_PLATFORM_MACOS
        std::array<char, 4096> buffer{};
        u32 bufSize = static_cast<u32>(buffer.size());
        if (_NSGetExecutablePath(buffer.data(), &bufSize) == 0)
        {
            return Path(buffer.data());
        }
        return Path{};
#elif ENGINE_PLATFORM_WINDOWS
        std::array<wchar_t, MAX_PATH> buffer{};
        const DWORD len = ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (len > 0 && len < buffer.size())
        {
            return Path(std::filesystem::path(buffer.data()));
        }
        return Path{};
#else
        return Path{};
#endif
    }

    Path Path::ExecutableDirectory()
    {
        return ExecutablePath().Parent();
    }

    Path Path::TempDirectory()
    {
        return Path(std::filesystem::temp_directory_path());
    }

    Path Path::UserDirectory()
    {
#if ENGINE_PLATFORM_WINDOWS
        const char* userProfile = std::getenv("USERPROFILE");
        if (userProfile)
        {
            return Path(userProfile);
        }
        // Fallback: HOMEDRIVE + HOMEPATH
        const char* homeDrive = std::getenv("HOMEDRIVE");
        const char* homePath  = std::getenv("HOMEPATH");
        if (homeDrive && homePath)
        {
            return Path(std::string_view(std::string(homeDrive) + homePath));
        }
        return Path{};
#else
        const char* home = std::getenv("HOME");
        if (home)
        {
            return Path(home);
        }
        return Path{};
#endif
    }

    Path Path::EngineDirectory()
    {
        Path dir = ExecutableDirectory();
        // Navigate up from the binary directory looking for "Engine/Include"
        // as a marker of the engine root.
        Path candidate = dir;
        for (int i = 0; i < 20; ++i) // Safeguard against infinite loop
        {
            Path marker = candidate / "Engine" / "Include";
            if (marker.IsDirectory())
            {
                return candidate;
            }
            Path parent = candidate.Parent();
            if (parent == candidate)
            {
                break; // Reached filesystem root
            }
            candidate = parent;
        }
        // Fallback: return the executable directory if we couldn't locate the marker.
        return dir;
    }

    Path Path::ProjectDirectory()
    {
        if (!s_projectDirectoryOverride.empty())
        {
            return Path(std::string_view(s_projectDirectoryOverride));
        }
        return CurrentWorkingDirectory();
    }

    void Path::SetProjectDirectory(const Path& path)
    {
        s_projectDirectoryOverride = path.String();
    }

    // ========================================================================
    //  Conversion
    // ========================================================================

    Path::operator std::filesystem::path() const
    {
        return m_path;
    }

    std::string Path::String() const
    {
        RebuildCache();
        return m_cachedString;
    }

    std::string Path::StringUTF8() const
    {
        return m_path.string();
    }

    const char* Path::CStr() const
    {
        RebuildCache();
        return m_cachedString.c_str();
    }

    void Path::RebuildCache() const
    {
        if (m_cacheDirty)
        {
            m_cachedString = m_path.string();
            m_cacheDirty   = false;
        }
    }

    // ========================================================================
    //  Inspection
    // ========================================================================

    bool Path::Exists() const
    {
        std::error_code ec;
        return std::filesystem::exists(m_path, ec);
    }

    bool Path::IsFile() const
    {
        std::error_code ec;
        return std::filesystem::is_regular_file(m_path, ec);
    }

    bool Path::IsDirectory() const
    {
        std::error_code ec;
        return std::filesystem::is_directory(m_path, ec);
    }

    bool Path::IsEmpty() const
    {
        if (m_path.empty())
        {
            return true;
        }

        std::error_code ec;
        if (std::filesystem::is_directory(m_path, ec))
        {
            return std::filesystem::is_empty(m_path, ec);
        }

        return m_path.empty();
    }

    bool Path::IsAbsolute() const
    {
        return m_path.is_absolute();
    }

    bool Path::IsRelative() const
    {
        return m_path.is_relative();
    }

    std::string Path::Extension() const
    {
        return m_path.extension().string();
    }

    std::string Path::Stem() const
    {
        return m_path.stem().string();
    }

    std::string Path::Filename() const
    {
        return m_path.filename().string();
    }

    Path Path::Parent() const
    {
        return Path(m_path.parent_path());
    }

    // ========================================================================
    //  Modification
    // ========================================================================

    Path& Path::Append(std::string_view segment)
    {
        m_path /= segment;
        m_cacheDirty = true;
        return *this;
    }

    Path Path::Appended(std::string_view segment) const
    {
        return Path(m_path / segment);
    }

    Path& Path::ReplaceExtension(std::string_view newExt)
    {
        m_path.replace_extension(newExt);
        m_cacheDirty = true;
        return *this;
    }

    Path Path::WithExtension(std::string_view newExt) const
    {
        auto copy = m_path;
        copy.replace_extension(newExt);
        return Path(copy);
    }

    Path& Path::ReplaceFilename(std::string_view newFilename)
    {
        m_path.replace_filename(newFilename);
        m_cacheDirty = true;
        return *this;
    }

    Path Path::WithFilename(std::string_view newFilename) const
    {
        auto copy = m_path;
        copy.replace_filename(newFilename);
        return Path(copy);
    }

    Path Path::Absolute() const
    {
        std::error_code ec;
        auto result = std::filesystem::absolute(m_path, ec);
        if (ec)
        {
            return *this;
        }
        return Path(result);
    }

    Path Path::Normalized() const
    {
        return Path(m_path.lexically_normal());
    }

    Path Path::RelativeTo(const Path& base) const
    {
        std::error_code ec;
        auto result = std::filesystem::relative(m_path, base.m_path, ec);
        if (ec)
        {
            return *this;
        }
        return Path(result);
    }

    usize Path::FileSize() const
    {
        std::error_code ec;
        const auto size = std::filesystem::file_size(m_path, ec);
        if (ec)
        {
            return 0;
        }
        return static_cast<usize>(size);
    }

    // ========================================================================
    //  Comparison
    // ========================================================================

    bool Path::operator==(const Path& other) const
    {
        return m_path == other.m_path;
    }

    bool Path::operator!=(const Path& other) const
    {
        return m_path != other.m_path;
    }

    bool Path::operator<(const Path& other) const
    {
        return m_path < other.m_path;
    }

    // ========================================================================
    //  Path joining operators
    // ========================================================================

    Path Path::operator/(std::string_view segment) const
    {
        return Path(m_path / segment);
    }

    Path& Path::operator/=(std::string_view segment)
    {
        m_path /= segment;
        m_cacheDirty = true;
        return *this;
    }

    // ========================================================================
    //  Directory listing
    // ========================================================================

    std::vector<Path> Path::ListFiles() const
    {
        std::vector<Path> files;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(m_path, ec))
        {
            std::error_code fileEc;
            if (entry.is_regular_file(fileEc))
            {
                files.emplace_back(entry.path());
            }
        }

        return files;
    }

    std::vector<Path> Path::ListDirectories() const
    {
        std::vector<Path> dirs;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(m_path, ec))
        {
            std::error_code dirEc;
            if (entry.is_directory(dirEc))
            {
                dirs.emplace_back(entry.path());
            }
        }

        return dirs;
    }

    std::vector<Path> Path::ListAll() const
    {
        std::vector<Path> entries;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(m_path, ec))
        {
            entries.emplace_back(entry.path());
        }

        return entries;
    }

    // ========================================================================
    //  Stream output
    // ========================================================================

    std::ostream& operator<<(std::ostream& os, const Path& path)
    {
        return os << path.String();
    }

} // namespace engine::fs