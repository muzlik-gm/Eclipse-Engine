// ============================================================================
// File: Engine/Include/Engine/Assets/Filesystem/AssetFilesystem.h
// Virtual filesystem abstraction for asset paths.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <filesystem>
#include <string>
#include <vector>

namespace engine::assets {

    namespace fs = std::filesystem;

    // ========================================================================
    // AssetFilesystem — maps virtual asset paths to real filesystem paths.
    // ========================================================================

    /// @brief Maps virtual asset paths (e.g. "assets://textures/brick.png")
    ///        to real filesystem paths.  Supports multiple root schemes:
    ///          assets://  — project assets directory
    ///          engine://  — engine built-in assets
    ///          package:// — packaged asset bundle (future)
    class AssetFilesystem
    {
    public:
        AssetFilesystem() = default;
        ~AssetFilesystem() = default;

        /// @brief Registers a root directory for a scheme.
        ///        Example: Mount("assets", "/home/user/project/assets")
        void Mount(const std::string& scheme, const std::string& rootPath);

        /// @brief Removes a mounted scheme.
        void Unmount(const std::string& scheme);

        /// @brief Resolves a virtual @p path to a real filesystem path.
        ///        Returns empty string if the scheme is not mounted.
        [[nodiscard]] std::string Resolve(const AssetPath& path) const;

        /// @brief Returns true if the file at @p path exists.
        [[nodiscard]] bool Exists(const AssetPath& path) const;

        /// @brief Returns the file size at @p path, or 0 if not found.
        [[nodiscard]] core::u64 GetFileSize(const AssetPath& path) const;

        /// @brief Returns the last modified time of @p path (Unix epoch).
        [[nodiscard]] core::u64 GetLastModified(const AssetPath& path) const;

        /// @brief Returns all files in @p directoryPath with the given
        ///        extension (or all files if extension is empty).
        [[nodiscard]] std::vector<AssetPath> ScanDirectory(
            const AssetPath& directoryPath,
            const std::string& extension = "") const;

        /// @brief Recursively scans @p directoryPath and returns all files.
        [[nodiscard]] std::vector<AssetPath> ScanDirectoryRecursive(
            const AssetPath& directoryPath,
            const std::string& extension = "") const;

        /// @brief Creates a virtual path from a real filesystem path.
        ///        Returns an invalid path if the real path is not under
        ///        any mounted root.
        [[nodiscard]] AssetPath MakeVirtualPath(const std::string& realPath) const;

        /// @brief Returns the absolute filesystem path for a scheme.
        [[nodiscard]] std::string GetRoot(const std::string& scheme) const;

        /// @brief Returns all mounted schemes.
        [[nodiscard]] std::vector<std::string> GetSchemes() const;

        /// @brief Creates a directory at the virtual path.
        bool CreateDirectory(const AssetPath& path) const;

        /// @brief Deletes a file at the virtual path.
        bool DeleteFile(const AssetPath& path) const;

        /// @brief Moves/renames a file.
        bool MoveFile(const AssetPath& oldPath, const AssetPath& newPath) const;

        /// @brief Copies a file.
        bool CopyFile(const AssetPath& srcPath, const AssetPath& dstPath) const;

    private:
        std::unordered_map<std::string, std::string> m_Roots;
    };

} // namespace engine::assets
