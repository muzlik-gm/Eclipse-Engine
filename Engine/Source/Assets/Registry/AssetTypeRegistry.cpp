// ============================================================================
// File: Engine/Source/Assets/Registry/AssetTypeRegistry.cpp
// ============================================================================
#include "Engine/Assets/Registry/AssetTypeRegistry.h"
#include "Engine/Core/Log.h"

namespace engine::assets {

    AssetTypeRegistry& AssetTypeRegistry::Get()
    {
        static AssetTypeRegistry instance;
        return instance;
    }

    void AssetTypeRegistry::RegisterType(std::unique_ptr<IAssetFactory> factory,
                                          std::vector<std::string> extensions)
    {
        auto entry = std::make_unique<TypeEntry>();
        entry->Type = factory->GetAssetType();
        entry->TypeName = std::string(factory->GetTypeName());
        entry->Factory = std::move(factory);
        entry->Extensions = std::move(extensions);

        auto* raw = entry.get();
        m_Entries.push_back(std::move(entry));

        m_ByType[raw->Type] = raw;
        m_ByName[raw->TypeName] = raw;
        for (const auto& ext : raw->Extensions)
            m_ByExtension[ext] = raw;

        ENGINE_LOG_DEBUG("AssetTypeRegistry — registered '{}' with {} extensions",
                         raw->TypeName, raw->Extensions.size());
    }

    void AssetTypeRegistry::RegisterCustomType(const std::string& typeName,
                                                std::unique_ptr<IAssetFactory> factory,
                                                std::vector<std::string> extensions)
    {
        auto entry = std::make_unique<TypeEntry>();
        entry->Type = AssetType::Custom;
        entry->TypeName = typeName;
        entry->Factory = std::move(factory);
        entry->Extensions = std::move(extensions);

        auto* raw = entry.get();
        m_Entries.push_back(std::move(entry));

        m_ByName[typeName] = raw;
        for (const auto& ext : raw->Extensions)
            m_ByExtension[ext] = raw;
    }

    const AssetTypeRegistry::TypeEntry* AssetTypeRegistry::Find(AssetType type) const
    {
        auto it = m_ByType.find(type);
        return (it != m_ByType.end()) ? it->second : nullptr;
    }

    const AssetTypeRegistry::TypeEntry* AssetTypeRegistry::FindByName(
        const std::string& typeName) const
    {
        auto it = m_ByName.find(typeName);
        return (it != m_ByName.end()) ? it->second : nullptr;
    }

    const AssetTypeRegistry::TypeEntry* AssetTypeRegistry::FindByExtension(
        const std::string& extension) const
    {
        auto it = m_ByExtension.find(extension);
        return (it != m_ByExtension.end()) ? it->second : nullptr;
    }

    std::vector<const AssetTypeRegistry::TypeEntry*> AssetTypeRegistry::GetAllEntries() const
    {
        std::vector<const TypeEntry*> result;
        result.reserve(m_Entries.size());
        for (const auto& e : m_Entries)
            result.push_back(e.get());
        return result;
    }

    core::usize AssetTypeRegistry::GetCount() const noexcept
    {
        return m_Entries.size();
    }

    void AssetTypeRegistry::Clear()
    {
        m_Entries.clear();
        m_ByType.clear();
        m_ByName.clear();
        m_ByExtension.clear();
    }

    AssetType AssetTypeRegistry::GetAssetTypeForExtension(const std::string& extension) const
    {
        auto* entry = FindByExtension(extension);
        return entry ? entry->Type : AssetType::Unknown;
    }

} // namespace engine::assets
