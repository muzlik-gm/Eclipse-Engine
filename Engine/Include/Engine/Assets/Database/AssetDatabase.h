// ============================================================================
// File: Engine/Include/Engine/Assets/Database/AssetDatabase.h
// Authoritative asset database — stores metadata for all known assets.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::assets {

    // ========================================================================
    // AssetDatabase — authoritative source for asset metadata.
    // ========================================================================

    /// @brief The authoritative database for all assets in the project.
    ///        Stores metadata (UUID, path, type, dependencies) for every
    ///        asset and provides lookup by UUID, path, or type.
    ///
    /// Thread-safe: all public methods acquire an internal mutex.
    class AssetDatabase
    {
    public:
        AssetDatabase() = default;
        ~AssetDatabase() = default;

        AssetDatabase(const AssetDatabase&)            = delete;
        AssetDatabase& operator=(const AssetDatabase&) = delete;

        // ----------------------------------------------------------------
        // Registration
        // ----------------------------------------------------------------

        /// @brief Registers a new asset with the given metadata.
        /// @return True if registered, false if UUID already exists.
        bool RegisterAsset(const AssetMetadata& metadata);

        /// @brief Unregisters an asset by UUID.
        /// @return True if removed, false if not found.
        bool UnregisterAsset(const AssetUUID& uuid);

        /// @brief Updates the metadata for an existing asset.
        /// @return True if updated, false if not found.
        bool UpdateMetadata(const AssetUUID& uuid, const AssetMetadata& metadata);

        // ----------------------------------------------------------------
        // Lookup
        // ----------------------------------------------------------------

        /// @brief Returns metadata for @p uuid, or nullptr.
        [[nodiscard]] const AssetMetadata* FindByUUID(const AssetUUID& uuid) const;

        /// @brief Returns metadata for @p path, or nullptr.
        [[nodiscard]] const AssetMetadata* FindByPath(const AssetPath& path) const;

        /// @brief Returns metadata for @p sourceFilePath, or nullptr.
        [[nodiscard]] const AssetMetadata* FindBySourceFile(const std::string& sourceFilePath) const;

        /// @brief Returns all assets of the given type.
        [[nodiscard]] std::vector<const AssetMetadata*> FindByType(AssetType type) const;

        /// @brief Returns all registered asset UUIDs.
        [[nodiscard]] std::vector<AssetUUID> GetAllUUIDs() const;

        /// @brief Returns all asset metadata entries.
        [[nodiscard]] std::vector<const AssetMetadata*> GetAllMetadata() const;

        // ----------------------------------------------------------------
        // Dependency tracking
        // ----------------------------------------------------------------

        /// @brief Returns the UUIDs of assets that @p uuid depends on.
        [[nodiscard]] std::vector<AssetUUID> GetDependencies(const AssetUUID& uuid) const;

        /// @brief Returns the UUIDs of assets that depend on @p uuid.
        [[nodiscard]] std::vector<AssetUUID> GetReferencedBy(const AssetUUID& uuid) const;

        /// @brief Adds a dependency: @p uuid depends on @p dependency.
        void AddDependency(const AssetUUID& uuid, const AssetUUID& dependency);

        /// @brief Removes a dependency.
        void RemoveDependency(const AssetUUID& uuid, const AssetUUID& dependency);

        /// @brief Returns true if @p uuid has any dependencies.
        [[nodiscard]] bool HasDependencies(const AssetUUID& uuid) const;

        // ----------------------------------------------------------------
        // Statistics
        // ----------------------------------------------------------------

        [[nodiscard]] AssetStatistics GetStatistics() const;
        [[nodiscard]] core::usize GetAssetCount() const noexcept;

        // ----------------------------------------------------------------
        // Bulk operations
        // ----------------------------------------------------------------

        /// @brief Clears all metadata.
        void Clear();

        /// @brief Scans a directory and registers assets found within.
        ///        This is a metadata-only scan — it does not import.
        core::usize ScanDirectory(const std::string& directoryPath,
                                   const std::string& scheme);

        // ----------------------------------------------------------------
        // Validation
        // ----------------------------------------------------------------

        /// @brief Returns UUIDs of assets whose source files are missing.
        [[nodiscard]] std::vector<AssetUUID> FindMissingAssets() const;

        /// @brief Returns UUIDs of assets with broken dependencies.
        [[nodiscard]] std::vector<AssetUUID> FindBrokenDependencies() const;

        /// @brief Returns UUIDs of duplicate assets (same source file).
        [[nodiscard]] std::vector<AssetUUID> FindDuplicates() const;

    private:
        mutable std::mutex m_Mutex;

        std::unordered_map<AssetUUID, AssetMetadata>            m_ByUUID;
        std::unordered_map<AssetPath, AssetUUID>                m_ByPath;
        std::unordered_map<std::string, AssetUUID>              m_BySourceFile;
        std::unordered_map<AssetUUID, std::vector<AssetUUID>>   m_Dependencies;
        std::unordered_map<AssetUUID, std::vector<AssetUUID>>   m_ReferencedBy;
    };

} // namespace engine::assets
