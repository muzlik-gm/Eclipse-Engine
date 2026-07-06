// ============================================================================
// File: Engine/Include/Engine/Assets/Packages/IAssetPackage.h
// Abstract interface for asset packages (bundled assets).
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"

#include <memory>
#include <string>
#include <vector>

namespace engine::assets {

    // ========================================================================
    // IAssetPackage — a bundle of assets for distribution.
    // ========================================================================

    /// @brief A package containing multiple assets bundled together
    ///        for distribution.  Packages can be loaded/unloaded as a
    ///        unit and may be compressed/encrypted.
    class IAssetPackage
    {
    public:
        virtual ~IAssetPackage() = default;

        /// @brief Returns the package's UUID.
        [[nodiscard]] virtual const AssetUUID& GetUUID() const noexcept = 0;

        /// @brief Returns the package's name.
        [[nodiscard]] virtual const std::string& GetName() const noexcept = 0;

        /// @brief Returns the package's version.
        [[nodiscard]] virtual core::u32 GetVersion() const noexcept = 0;

        /// @brief Returns all asset UUIDs contained in this package.
        [[nodiscard]] virtual std::vector<AssetUUID> GetAssetUUIDs() const = 0;

        /// @brief Returns metadata for @p uuid within this package.
        [[nodiscard]] virtual const AssetMetadata* FindAsset(const AssetUUID& uuid) const = 0;

        /// @brief Loads the package's manifest (does not load asset data).
        /// @return True on success.
        virtual bool LoadManifest(const std::string& packagePath) = 0;

        /// @brief Loads the asset data for @p uuid from the package.
        /// @return The raw asset data, or empty on failure.
        [[nodiscard]] virtual std::string LoadAssetData(const AssetUUID& uuid) = 0;

        /// @brief Unloads the package, releasing all resources.
        virtual void Unload() = 0;

        /// @brief Returns true if the package is loaded.
        [[nodiscard]] virtual bool IsLoaded() const = 0;
    };

    // ========================================================================
    // IAssetPackageBuilder — builds packages from assets.
    // ========================================================================

    class IAssetPackageBuilder
    {
    public:
        virtual ~IAssetPackageBuilder() = default;

        /// @brief Adds an asset to the package being built.
        virtual void AddAsset(const AssetMetadata& metadata, const std::string& assetData) = 0;

        /// @brief Builds the package and writes it to @p outputPath.
        /// @return True on success.
        virtual bool Build(const std::string& outputPath) = 0;
    };

} // namespace engine::assets
