// ============================================================================
// File: Engine/Include/Engine/Assets/Core/IAsset.h
// Base interface for all runtime assets.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"

#include <memory>
#include <string>
#include <string_view>

namespace engine::assets {

    // ========================================================================
    // IAsset — base interface for all runtime assets.
    // ========================================================================

    /// @brief Base interface for every runtime asset.  Concrete asset
    ///        types (TextureAsset, MeshAsset, etc.) implement this
    ///        interface and provide type-specific data accessors.
    class IAsset
    {
    public:
        virtual ~IAsset() = default;

        /// @brief Returns the asset's UUID.
        [[nodiscard]] virtual const AssetUUID& GetUUID() const noexcept = 0;

        /// @brief Returns the asset's human-readable name.
        [[nodiscard]] virtual const std::string& GetName() const noexcept = 0;

        /// @brief Returns the asset's type.
        [[nodiscard]] virtual AssetType GetType() const noexcept = 0;

        /// @brief Returns the asset's metadata.
        [[nodiscard]] virtual const AssetMetadata& GetMetadata() const noexcept = 0;

        /// @brief Returns the current lifecycle state.
        [[nodiscard]] virtual AssetState GetState() const noexcept = 0;

        /// @brief Returns the size of the asset's runtime data in bytes.
        [[nodiscard]] virtual core::usize GetSize() const noexcept = 0;

        /// @brief Loads the asset's runtime data into memory.
        /// @return True on success.
        virtual bool Load() = 0;

        /// @brief Unloads the asset's runtime data, freeing memory.
        virtual void Unload() = 0;

        /// @brief Returns true if the asset's runtime data is loaded.
        [[nodiscard]] virtual bool IsLoaded() const noexcept = 0;

        /// @brief Reloads the asset from disk, preserving the UUID.
        /// @return True on success.
        virtual bool Reload() = 0;

        /// @brief Returns the last error message, or empty if none.
        [[nodiscard]] virtual const std::string& GetLastError() const noexcept = 0;
    };

    // ========================================================================
    // IAssetFactory — factory that creates IAsset instances.
    // ========================================================================

    /// @brief Factory interface for creating asset instances of a
    ///        specific type.  Each asset type registers a factory with
    ///        AssetTypeRegistry.
    class IAssetFactory
    {
    public:
        virtual ~IAssetFactory() = default;

        /// @brief Returns the asset type this factory creates.
        [[nodiscard]] virtual AssetType GetAssetType() const noexcept = 0;

        /// @brief Returns the type name (for custom types).
        [[nodiscard]] virtual std::string_view GetTypeName() const noexcept = 0;

        /// @brief Creates a new asset instance with the given metadata.
        [[nodiscard]] virtual std::unique_ptr<IAsset> Create(
            const AssetMetadata& metadata) = 0;
    };

} // namespace engine::assets
