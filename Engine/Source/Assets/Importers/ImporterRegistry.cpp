// ============================================================================
// File: Engine/Source/Assets/Importers/ImporterRegistry.cpp
// ============================================================================
#include "Engine/Assets/Importers/ImporterRegistry.h"
#include "Engine/Core/Log.h"

#include <algorithm>
#include <filesystem>

namespace engine::assets {

    namespace fs = std::filesystem;

    ImporterRegistry& ImporterRegistry::Get()
    {
        static ImporterRegistry instance;
        return instance;
    }

    void ImporterRegistry::Register(std::unique_ptr<IAssetImporter> importer)
    {
        auto* raw = importer.get();
        auto name = std::string(raw->GetName());
        auto exts = raw->GetSupportedExtensions();

        m_Importers.push_back(std::move(importer));
        m_ByName[name] = raw;

        for (const auto& ext : exts)
            m_ByExtension[ext] = raw;

        ENGINE_LOG_INFO("ImporterRegistry — registered '{}' ({} extensions)",
                       name, exts.size());
    }

    IAssetImporter* ImporterRegistry::FindByExtension(const std::string& extension) const
    {
        auto it = m_ByExtension.find(extension);
        return (it != m_ByExtension.end()) ? it->second : nullptr;
    }

    IAssetImporter* ImporterRegistry::FindByName(const std::string& name) const
    {
        auto it = m_ByName.find(name);
        return (it != m_ByName.end()) ? it->second : nullptr;
    }

    std::vector<IAssetImporter*> ImporterRegistry::GetAllImporters() const
    {
        std::vector<IAssetImporter*> result;
        result.reserve(m_Importers.size());
        for (const auto& i : m_Importers)
            result.push_back(i.get());
        return result;
    }

    IAssetImporter* ImporterRegistry::FindForFile(const std::string& filePath) const
    {
        auto ext = fs::path(filePath).extension().string();
        return FindByExtension(ext);
    }

    core::usize ImporterRegistry::GetCount() const noexcept
    {
        return m_Importers.size();
    }

    void ImporterRegistry::Clear()
    {
        m_Importers.clear();
        m_ByExtension.clear();
        m_ByName.clear();
    }

} // namespace engine::assets
