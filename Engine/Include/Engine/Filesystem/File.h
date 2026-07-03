#pragma once

/**
 * @file File.h
 * @brief Static utility class for common file operations.
 *
 * Provides existence checks, copy / move / rename, text and binary I/O,
 * line-level reading, and modification-time queries.  All methods are
 * stateless and operate on engine::fs::Path values.
 */

#include "Engine/Core/Types.h"
#include "Engine/Filesystem/Path.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine::fs
{

    using engine::core::i64;
    using engine::core::u64;
    using engine::core::u8;
    using engine::core::usize;

    /// Stateless utility class providing common file I/O operations.
    class File
    {
    public:
        File() = delete;

        // ----------------------------------------------------------------
        //  File existence and management
        // ----------------------------------------------------------------

        /// Returns true if the file at @p path exists on disk.
        [[nodiscard]] static bool Exists(const Path& path);

        /// Deletes the file at @p path.  Returns true on success.
        static bool Delete(const Path& path);

        /// Copies the file from @p src to @p dst.
        /// If @p overwrite is true and @p dst already exists it is replaced.
        static bool Copy(const Path& src, const Path& dst, bool overwrite = true);

        /// Moves (renames) the file from @p src to @p dst.
        static bool Move(const Path& src, const Path& dst);

        /// Renames the file at @p path to @p newName (same directory).
        static bool Rename(const Path& path, const Path& newName);

        // ----------------------------------------------------------------
        //  File metadata
        // ----------------------------------------------------------------

        /// Returns the size of the file at @p path in bytes, or 0 on failure.
        [[nodiscard]] static u64 Size(const Path& path);

        /// Returns the last modification time as a Unix-epoch timestamp,
        /// or -1 on failure.
        [[nodiscard]] static i64 LastModifiedTime(const Path& path);

        /// Creates the file if it does not exist, or updates its
        /// modification time if it does.  Returns true on success.
        static bool Touch(const Path& path);

        // ----------------------------------------------------------------
        //  Text I/O
        // ----------------------------------------------------------------

        /// Reads the entire file at @p path as a UTF-8 string.
        /// Returns an empty string on failure.
        [[nodiscard]] static std::string ReadText(const Path& path);

        /// Writes @p content as text to @p path (overwrites existing).
        /// Returns true on success.
        static bool WriteText(const Path& path, std::string_view content);

        /// Appends @p content as text to @p path.
        /// Returns true on success.
        static bool AppendText(const Path& path, std::string_view content);

        // ----------------------------------------------------------------
        //  Binary I/O
        // ----------------------------------------------------------------

        /// Reads the entire file at @p path as raw bytes.
        /// Returns an empty vector on failure.
        [[nodiscard]] static std::vector<u8> ReadBinary(const Path& path);

        /// Writes @p data as raw bytes to @p path (overwrites existing).
        /// Returns true on success.
        static bool WriteBinary(const Path& path, const std::vector<u8>& data);

        /// Appends @p data as raw bytes to @p path.
        /// Returns true on success.
        static bool AppendBinary(const Path& path, const std::vector<u8>& data);

        // ----------------------------------------------------------------
        //  Line-level reading
        // ----------------------------------------------------------------

        /// Returns a specific line (0-indexed) from the file.
        /// Returns an empty string if the line is out of range or on error.
        [[nodiscard]] static std::string ReadLine(const Path& path, usize lineIndex);

        /// Reads all lines from the file and returns them as a vector.
        [[nodiscard]] static std::vector<std::string> ReadLines(const Path& path);
    };

} // namespace engine::fs