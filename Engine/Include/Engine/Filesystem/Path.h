#pragma once

/**
 * @file Path.h
 * @brief Engine path wrapper around std::filesystem::path with game-engine
 *        convenience utilities.
 *
 * Provides construction from common sources, static factories for system
 * directories, path inspection / modification, and directory listing.  All
 * public methods are documented and the class is fully comparable for use
 * in ordered containers.
 */

#include "Engine/Core/Types.h"

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace engine::fs
{

    using engine::core::usize;

    /// Wraps std::filesystem::path with engine-specific convenience methods.
    class Path
    {
    public:
        // ----------------------------------------------------------------
        //  Construction
        // ----------------------------------------------------------------

        /// Default-constructs an empty path.
        Path() = default;

        /// Constructs from a UTF-8 string view.
        Path(std::string_view path);

        /// Constructs from an existing std::filesystem::path.
        Path(const std::filesystem::path& path);

        /// Constructs from a C string literal.
        Path(const char* path);

        // ----------------------------------------------------------------
        //  Static factories — system / special directories
        // ----------------------------------------------------------------

        /// Returns the current working directory.
        [[nodiscard]] static Path CurrentWorkingDirectory();

        /// Returns the absolute path of the running executable.
        /// Reads /proc/self/exe on Linux; uses GetModuleFileNameW on Windows.
        [[nodiscard]] static Path ExecutablePath();

        /// Returns the directory containing the running executable.
        [[nodiscard]] static Path ExecutableDirectory();

        /// Returns the system temporary directory.
        [[nodiscard]] static Path TempDirectory();

        /// Returns the user's home directory.
        /// Uses $HOME on Unix, %USERPROFILE% on Windows.
        [[nodiscard]] static Path UserDirectory();

        /// Returns the root directory of the engine installation.
        /// Derived from ExecutableDirectory() by navigating up from the binary directory.
        [[nodiscard]] static Path EngineDirectory();

        /// Returns the root directory of the currently loaded project.
        /// Defaults to CurrentWorkingDirectory() but can be overridden via SetProjectDirectory().
        [[nodiscard]] static Path ProjectDirectory();

        /// Overrides the project directory for the current session.
        static void SetProjectDirectory(const Path& path);

        // ----------------------------------------------------------------
        //  Conversion
        // ----------------------------------------------------------------

        /// Implicit conversion to the underlying std::filesystem::path.
        operator std::filesystem::path() const;

        /// Returns the path as a narrow string (native encoding).
        [[nodiscard]] std::string String() const;

        /// Returns the path as a UTF-8 string.
        [[nodiscard]] std::string StringUTF8() const;

        /// Returns a C string pointer to a cached string representation.
        /// The pointer is valid only as long as this Path object is not
        /// modified or destroyed.
        [[nodiscard]] const char* CStr() const;

        // ----------------------------------------------------------------
        //  Inspection
        // ----------------------------------------------------------------

        /// True when the path refers to an existing filesystem entry.
        [[nodiscard]] bool Exists() const;

        /// True when the path refers to a regular file.
        [[nodiscard]] bool IsFile() const;

        /// True when the path refers to a directory.
        [[nodiscard]] bool IsDirectory() const;

        /// True when the path is empty or the directory it refers to is empty.
        [[nodiscard]] bool IsEmpty() const;

        /// True when the path is absolute.
        [[nodiscard]] bool IsAbsolute() const;

        /// True when the path is relative.
        [[nodiscard]] bool IsRelative() const;

        /// Returns the file extension including the leading dot, or an
        /// empty string if there is none.
        [[nodiscard]] std::string Extension() const;

        /// Returns the filename without the extension.
        [[nodiscard]] std::string Stem() const;

        /// Returns the filename component (including the extension).
        [[nodiscard]] std::string Filename() const;

        /// Returns the parent directory as a new Path.
        [[nodiscard]] Path Parent() const;

        // ----------------------------------------------------------------
        //  Modification
        // ----------------------------------------------------------------

        /// Appends a path segment in-place and returns *this.
        Path& Append(std::string_view segment);

        /// Returns a new Path with the segment appended (this is unmodified).
        [[nodiscard]] Path Appended(std::string_view segment) const;

        /// Replaces the extension in-place and returns *this.
        Path& ReplaceExtension(std::string_view newExt);

        /// Returns a new Path with the extension replaced (this is unmodified).
        [[nodiscard]] Path WithExtension(std::string_view newExt) const;

        /// Replaces the filename in-place and returns *this.
        Path& ReplaceFilename(std::string_view newFilename);

        /// Returns a new Path with the filename replaced (this is unmodified).
        [[nodiscard]] Path WithFilename(std::string_view newFilename) const;

        /// Returns an absolute copy of this path.
        [[nodiscard]] Path Absolute() const;

        /// Returns a lexically normalised copy (resolves . and ..).
        [[nodiscard]] Path Normalized() const;

        /// Returns this path made relative to @p base.
        [[nodiscard]] Path RelativeTo(const Path& base) const;

        /// Returns the file size in bytes, or 0 if not a file.
        [[nodiscard]] usize FileSize() const;

        // ----------------------------------------------------------------
        //  Comparison (for use in ordered containers)
        // ----------------------------------------------------------------

        bool operator==(const Path& other) const;
        bool operator!=(const Path& other) const;
        bool operator<(const Path& other) const;

        // ----------------------------------------------------------------
        //  Path joining operators
        // ----------------------------------------------------------------

        /// Returns a new Path formed by appending @p segment.
        [[nodiscard]] Path operator/(std::string_view segment) const;

        /// Appends @p segment in-place.
        Path& operator/=(std::string_view segment);

        // ----------------------------------------------------------------
        //  Directory listing
        // ----------------------------------------------------------------

        /// Lists regular files in the directory (non-recursive).
        /// Returns an empty vector if this is not a directory.
        [[nodiscard]] std::vector<Path> ListFiles() const;

        /// Lists subdirectories (non-recursive).
        /// Returns an empty vector if this is not a directory.
        [[nodiscard]] std::vector<Path> ListDirectories() const;

        /// Lists all entries — files and subdirectories (non-recursive).
        /// Returns an empty vector if this is not a directory.
        [[nodiscard]] std::vector<Path> ListAll() const;

        // ----------------------------------------------------------------
        //  Stream output
        // ----------------------------------------------------------------

        /// Writes the path to an output stream.
        friend std::ostream& operator<<(std::ostream& os, const Path& path);

    private:
        inline static std::string s_projectDirectoryOverride;

        std::filesystem::path m_path;
        mutable std::string   m_cachedString;
        mutable bool          m_cacheDirty = true;

        /// Rebuilds m_cachedString from m_path.
        void RebuildCache() const;
    };

} // namespace engine::fs