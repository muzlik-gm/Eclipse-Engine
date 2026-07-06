// ============================================================================
// File: Engine/Include/Engine/Assets/Metadata/AssetMetadata.h
// Metadata describing an asset — stored in the asset database.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/UUID.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace engine::assets {

    using engine::core::u32;
    using engine::core::u64;

    // ========================================================================
    // AssetMetadata — persistent metadata for a single asset.
    // ========================================================================

    /// @brief Persistent metadata describing an asset.  Stored in the
    ///        asset database and serialized alongside the asset data.
    struct AssetMetadata
    {
        /// Stable UUID for this asset.
        AssetUUID UUID{};

        /// Human-readable name (e.g. "Brick Texture").
        std::string Name;

        /// Virtual path (e.g. "assets://textures/brick.png").
        AssetPath Path;

        /// Asset type (Texture, Mesh, Audio, etc.).
        AssetType Type{AssetType::Unknown};

        /// For custom asset types, the type name registered with
        /// AssetTypeRegistry.
        std::string CustomTypeName;

        /// File system path to the source file (for re-import).
        std::string SourceFilePath;

        /// File system path to the imported/processed file.
        std::string ImportedFilePath;

        /// Schema version of the asset data.
        u32 Version{1};

        /// Importer version that produced the current imported data.
        u32 ImporterVersion{0};

        /// File size of the source file in bytes (for display).
        u64 SourceFileSize{0};

        /// Last modified timestamp of the source file (Unix epoch).
        u64 SourceLastModified{0};

        /// UUIDs of assets this asset depends on.
        std::vector<AssetUUID> Dependencies;

        /// UUIDs of assets that depend on this asset (reverse deps).
        std::vector<AssetUUID> ReferencedBy;

        /// Arbitrary key-value tags for categorization and search.
        std::vector<std::pair<std::string, std::string>> Tags;

        /// True if the asset should be included in packaged builds.
        bool IsIncludedInBuild{true};

        /// True if the asset is automatically streamed at runtime.
        bool IsStreamed{false};

        /// Optional priority for streaming (lower = higher priority).
        u32 StreamingPriority{1000};

        /// Returns true if the metadata is valid (has a UUID and path).
        [[nodiscard]] bool IsValid() const noexcept
        {
            return UUID.IsValid() && Path.IsValid();
        }
    };

    // ========================================================================
    // AssetStatistics — summary statistics for the asset database.
    // ========================================================================

    struct AssetStatistics
    {
        u32 TotalAssets{0};
        u32 LoadedAssets{0};
        u32 ImportedAssets{0};
        u32 ErrorAssets{0};
        u32 MissingAssets{0};
        u32 TotalDependencies{0};
        u64 TotalImportedSize{0};
        u64 TotalSourceSize{0};
    };

} // namespace engine::assets
