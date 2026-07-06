// ============================================================================
// File: Engine/Include/Engine/Assets/Registry/AssetTypeRegistry.h
// Registry of asset types and their factories.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Core/IAsset.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine::assets {

    // ========================================================================
    // AssetTypeRegistry — registers asset types and their factories.
    // ========================================================================

    /// @brief Registry of all known asset types.  Each type has an
    ///        IAssetFactory that creates instances, and a set of file
    ///        extensions that map to the type for auto-detection.
    class AssetTypeRegistry
    {
    public:
        struct TypeEntry
        {
            AssetType                     Type{AssetType::Unknown};
            std::string                   TypeName;
            std::unique_ptr<IAssetFactory> Factory;
            std::vector<std::string>      Extensions;
        };

        /// @brief Returns the global registry instance.
        [[nodiscard]] static AssetTypeRegistry& Get();

        /// @brief Registers an asset type with its factory and extensions.
        void RegisterType(std::unique_ptr<IAssetFactory> factory,
                          std::vector<std::string> extensions);

        /// @brief Registers a custom asset type by name.
        void RegisterCustomType(const std::string& typeName,
                                std::unique_ptr<IAssetFactory> factory,
                                std::vector<std::string> extensions);

        /// @brief Returns the entry for @p type, or nullptr.
        [[nodiscard]] const TypeEntry* Find(AssetType type) const;

        /// @brief Returns the entry for @p typeName, or nullptr.
        [[nodiscard]] const TypeEntry* FindByName(const std::string& typeName) const;

        /// @brief Returns the entry for a file @p extension, or nullptr.
        [[nodiscard]] const TypeEntry* FindByExtension(const std::string& extension) const;

        /// @brief Returns all registered type entries.
        [[nodiscard]] std::vector<const TypeEntry*> GetAllEntries() const;

        /// @brief Returns the number of registered types.
        [[nodiscard]] core::usize GetCount() const noexcept;

        /// @brief Clears all registrations.
        void Clear();

        /// @brief Returns the asset type for a file @p extension.
        [[nodiscard]] AssetType GetAssetTypeForExtension(const std::string& extension) const;

    private:
        AssetTypeRegistry() = default;

        std::vector<std::unique_ptr<TypeEntry>>           m_Entries;
        std::unordered_map<AssetType, TypeEntry*>          m_ByType;
        std::unordered_map<std::string, TypeEntry*>        m_ByName;
        std::unordered_map<std::string, TypeEntry*>        m_ByExtension;
    };

    // ========================================================================
    // AssetTypeRegistrar — RAII helper for type registration.
    // ========================================================================

    class AssetTypeRegistrar
    {
    public:
        AssetTypeRegistrar(std::unique_ptr<IAssetFactory> factory,
                           std::vector<std::string> extensions)
        {
            AssetTypeRegistry::Get().RegisterType(std::move(factory), std::move(extensions));
        }
    };

} // namespace engine::assets
