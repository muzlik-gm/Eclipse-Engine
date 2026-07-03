#include "Engine/Filesystem/Directory.h"

#include <filesystem>

namespace engine::fs
{

    using engine::core::u64;

    // ========================================================================
    //  Existence and creation
    // ========================================================================

    bool Directory::Exists(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::is_directory(
            static_cast<std::filesystem::path>(path), ec);
    }

    bool Directory::Create(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::create_directory(
            static_cast<std::filesystem::path>(path), ec);
    }

    bool Directory::CreateAll(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::create_directories(
            static_cast<std::filesystem::path>(path), ec);
    }

    // ========================================================================
    //  Removal
    // ========================================================================

    bool Directory::Remove(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::remove(
            static_cast<std::filesystem::path>(path), ec) != 0;
    }

    bool Directory::RemoveAll(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::remove_all(
            static_cast<std::filesystem::path>(path), ec) != static_cast<u64>(-1);
    }

    // ========================================================================
    //  Copying
    // ========================================================================

    bool Directory::Copy(const Path& src, const Path& dst)
    {
        std::error_code ec;
        std::filesystem::copy(
            static_cast<std::filesystem::path>(src),
            static_cast<std::filesystem::path>(dst),
            std::filesystem::copy_options::recursive
                | std::filesystem::copy_options::copy_symlinks
                | std::filesystem::copy_options::directories_only,
            ec);

        // The directories_only option only copies the directory structure.
        // To get a full copy including files, we need to also copy file
        // contents.  Use a two-pass approach.

        // First pass: copy the directory tree (creates directories).
        std::filesystem::copy(
            static_cast<std::filesystem::path>(src),
            static_cast<std::filesystem::path>(dst),
            std::filesystem::copy_options::recursive
                | std::filesystem::copy_options::copy_symlinks
                | std::filesystem::copy_options::directories_only,
            ec);

        if (ec)
        {
            // Destination parent may not exist; try create_directories first.
            std::filesystem::create_directories(
                static_cast<std::filesystem::path>(dst.Parent()), ec);
        }

        // Second pass: copy all files.
        std::filesystem::copy(
            static_cast<std::filesystem::path>(src),
            static_cast<std::filesystem::path>(dst),
            std::filesystem::copy_options::recursive
                | std::filesystem::copy_options::copy_symlinks
                | std::filesystem::copy_options::overwrite_existing,
            ec);

        return !ec;
    }

    // ========================================================================
    //  Current working directory
    // ========================================================================

    Path Directory::Current()
    {
        return Path(std::filesystem::current_path());
    }

    bool Directory::SetCurrent(const Path& path)
    {
        std::error_code ec;
        std::filesystem::current_path(
            static_cast<std::filesystem::path>(path), ec);
        return !ec;
    }

    // ========================================================================
    //  Listing (non-recursive)
    // ========================================================================

    std::vector<Path> Directory::Files(const Path& path)
    {
        return path.ListFiles();
    }

    std::vector<Path> Directory::Subdirectories(const Path& path)
    {
        return path.ListDirectories();
    }

    std::vector<Path> Directory::AllEntries(const Path& path)
    {
        return path.ListAll();
    }

    // ========================================================================
    //  Listing (recursive)
    // ========================================================================

    std::vector<Path> Directory::FilesRecursive(const Path& path)
    {
        std::vector<Path> files;

        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(
            static_cast<std::filesystem::path>(path),
            std::filesystem::directory_options::skip_permission_denied,
            ec);

        if (ec)
        {
            return files;
        }

        std::error_code iterEc;
        for (auto end = std::filesystem::recursive_directory_iterator{};
             it != end;
             it.increment(iterEc))
        {
            if (iterEc)
            {
                iterEc.clear();
                continue;
            }

            std::error_code fileEc;
            if (it->is_regular_file(fileEc))
            {
                files.emplace_back(it->path());
            }
        }

        return files;
    }

    // ========================================================================
    //  Queries
    // ========================================================================

    bool Directory::IsEmpty(const Path& path)
    {
        std::error_code ec;
        return std::filesystem::is_empty(
            static_cast<std::filesystem::path>(path), ec);
    }

    u64 Directory::TotalSize(const Path& path)
    {
        u64 total = 0;

        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(
            static_cast<std::filesystem::path>(path),
            std::filesystem::directory_options::skip_permission_denied,
            ec);

        if (ec)
        {
            return 0;
        }

        std::error_code iterEc;
        for (auto end = std::filesystem::recursive_directory_iterator{};
             it != end;
             it.increment(iterEc))
        {
            if (iterEc)
            {
                iterEc.clear();
                continue;
            }

            std::error_code sizeEc;
            if (it->is_regular_file(sizeEc))
            {
                std::error_code fileSizeEc;
                auto sz = std::filesystem::file_size(it->path(), fileSizeEc);
                if (!fileSizeEc)
                {
                    total += static_cast<u64>(sz);
                }
            }
        }

        return total;
    }

} // namespace engine::fs