// ============================================================================
// File: Engine/Include/Engine/Assets/Importers/ImporterRegistry.h
// Registry of asset importers.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Importers/IAssetImporter.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::assets {

    // ========================================================================
    // ImporterRegistry — registry of all importers.
    // ========================================================================

    /// @brief Registry of all asset importers.  Importers register
    ///        themselves at startup; the AssetManager queries this
    ///        registry to find the right importer for a file extension.
    class ImporterRegistry
    {
    public:
        /// @brief Returns the global registry instance.
        [[nodiscard]] static ImporterRegistry& Get();

        /// @brief Registers an importer.
        void Register(std::unique_ptr<IAssetImporter> importer);

        /// @brief Returns the importer that handles @p extension, or nullptr.
        [[nodiscard]] IAssetImporter* FindByExtension(const std::string& extension) const;

        /// @brief Returns the importer with @p name, or nullptr.
        [[nodiscard]] IAssetImporter* FindByName(const std::string& name) const;

        /// @brief Returns all registered importers.
        [[nodiscard]] std::vector<IAssetImporter*> GetAllImporters() const;

        /// @brief Returns the importer for @p filePath based on extension.
        [[nodiscard]] IAssetImporter* FindForFile(const std::string& filePath) const;

        /// @brief Returns the number of registered importers.
        [[nodiscard]] core::usize GetCount() const noexcept;

        /// @brief Clears all registrations.
        void Clear();

    private:
        ImporterRegistry() = default;

        std::vector<std::unique_ptr<IAssetImporter>>                m_Importers;
        std::unordered_map<std::string, IAssetImporter*>            m_ByExtension;
        std::unordered_map<std::string, IAssetImporter*>            m_ByName;
    };

    // ========================================================================
    // ImporterRegistrar — RAII helper for importer registration.
    // ========================================================================

    class ImporterRegistrar
    {
    public:
        explicit ImporterRegistrar(std::unique_ptr<IAssetImporter> importer)
        {
            ImporterRegistry::Get().Register(std::move(importer));
        }
    };

} // namespace engine::assets
