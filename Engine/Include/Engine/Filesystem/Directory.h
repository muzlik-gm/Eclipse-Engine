#pragma once

/**
 * @file Directory.h
 * @brief Static utility class for directory-level filesystem operations.
 *
 * Provides creation, removal, copying, and listing of directories.
 * Supports both shallow (single-level) and recursive operations.
 */

#include "Engine/Core/Types.h"
#include "Engine/Filesystem/Path.h"

#include <vector>

namespace engine::fs
{

    using engine::core::u64;
    using engine::core::usize;

    /// Stateless utility class providing directory-level filesystem operations.
    class Directory
    {
    public:
        Directory() = delete;

        // ----------------------------------------------------------------
        //  Existence and creation
        // ----------------------------------------------------------------

        /// Returns true if @p path exists and is a directory.
        [[nodiscard]] static bool Exists(const Path& path);

        /// Creates a single directory at @p path.  Parent must exist.
        /// Returns true on success or if the directory already exists.
        static bool Create(const Path& path);

        /// Creates the full directory tree at @p path (equivalent to mkdir -p).
        /// Returns true on success or if the directory already exists.
        static bool CreateAll(const Path& path);

        // ----------------------------------------------------------------
        //  Removal
        // ----------------------------------------------------------------

        /// Removes the directory at @p path only if it is empty.
        /// Returns true on success.
        static bool Remove(const Path& path);

        /// Removes the directory at @p path and all of its contents
        /// recursively.
        static bool RemoveAll(const Path& path);

        // ----------------------------------------------------------------
        //  Copying
        // ----------------------------------------------------------------

        /// Recursively copies the directory tree from @p src to @p dst.
        static bool Copy(const Path& src, const Path& dst);

        // ----------------------------------------------------------------
        //  Current working directory
        // ----------------------------------------------------------------

        /// Returns the current working directory.
        [[nodiscard]] static Path Current();

        /// Changes the current working directory to @p path.
        /// Returns true on success.
        static bool SetCurrent(const Path& path);

        // ----------------------------------------------------------------
        //  Listing (non-recursive)
        // ----------------------------------------------------------------

        /// Returns all regular files in @p path (non-recursive).
        [[nodiscard]] static std::vector<Path> Files(const Path& path);

        /// Returns all immediate subdirectories of @p path (non-recursive).
        [[nodiscard]] static std::vector<Path> Subdirectories(const Path& path);

        /// Returns all entries (files + subdirectories) in @p path
        /// (non-recursive).
        [[nodiscard]] static std::vector<Path> AllEntries(const Path& path);

        // ----------------------------------------------------------------
        //  Listing (recursive)
        // ----------------------------------------------------------------

        /// Returns all regular files under @p path, recursively.
        [[nodiscard]] static std::vector<Path> FilesRecursive(const Path& path);

        // ----------------------------------------------------------------
        //  Queries
        // ----------------------------------------------------------------

        /// Returns true if the directory at @p path exists and is empty.
        [[nodiscard]] static bool IsEmpty(const Path& path);

        /// Returns the total size (in bytes) of all files under @p path,
        /// computed recursively.
        [[nodiscard]] static u64 TotalSize(const Path& path);
    };

} // namespace engine::fs