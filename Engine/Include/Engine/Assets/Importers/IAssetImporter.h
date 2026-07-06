// ============================================================================
// File: Engine/Include/Engine/Assets/Importers/IAssetImporter.h
// Abstract interface for asset importers.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"

#include <memory>
#include <string>
#include <vector>

namespace engine::assets {

    class IAsset;

    // ========================================================================
    // ImportSettings — base class for importer-specific settings.
    // ========================================================================

    /// @brief Base class for import settings.  Concrete importers
    ///        subclass this to add their own settings.
    struct ImportSettings
    {
        virtual ~ImportSettings() = default;

        /// If true, the imported asset will be re-imported automatically
        /// when the source file changes.
        bool AutoReimport{true};

        /// If true, the imported data is cached on disk.
        bool UseCache{true};

        /// Quality level (0-100) for lossy formats.
        core::u32 Quality{100};
    };

    // ========================================================================
    // ImportResult — result of an import operation.
    // ========================================================================

    struct ImportResult
    {
        bool Success{false};
        std::string ErrorMessage;
        std::vector<AssetMetadata> ImportedAssets;
        std::vector<std::string>   Warnings;

        [[nodiscard]] bool IsSuccess() const noexcept { return Success; }
    };

    // ========================================================================
    // ImportContext — context passed to an importer.
    // ========================================================================

    struct ImportContext
    {
        std::string         SourceFilePath;
        std::string         OutputDirectory;
        const ImportSettings* Settings{nullptr};
        AssetType           TargetType{AssetType::Unknown};
    };

    // ========================================================================
    // IAssetImporter — abstract interface for all importers.
    // ========================================================================

    /// @brief Abstract interface for an asset importer.  Each file
    ///        format has a concrete importer implementation registered
    ///        with ImporterRegistry.
    class IAssetImporter
    {
    public:
        virtual ~IAssetImporter() = default;

        /// @brief Returns the importer's display name (e.g. "PNG Importer").
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        /// @brief Returns the importer version.  Increment when the
        ///        import logic changes so cached imports are invalidated.
        [[nodiscard]] virtual core::u32 GetVersion() const noexcept = 0;

        /// @brief Returns the file extensions this importer supports
        ///        (e.g. {".png", ".jpg"}).
        [[nodiscard]] virtual std::vector<std::string> GetSupportedExtensions() const = 0;

        /// @brief Returns the asset type this importer produces.
        [[nodiscard]] virtual AssetType GetAssetType() const = 0;

        /// @brief Returns true if this importer can handle @p filePath.
        [[nodiscard]] virtual bool CanImport(const std::string& filePath) const = 0;

        /// @brief Imports the asset at @p context.SourceFilePath.
        /// @return Import result with metadata and error info.
        [[nodiscard]] virtual ImportResult Import(const ImportContext& context) = 0;

        /// @brief Returns the default import settings for this importer.
        [[nodiscard]] virtual std::unique_ptr<ImportSettings> CreateDefaultSettings() const = 0;
    };

} // namespace engine::assets
