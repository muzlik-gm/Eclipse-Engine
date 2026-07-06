// ============================================================================
// File: Engine/Source/Assets/Database/AssetDatabase.cpp
// ============================================================================
#include "Engine/Assets/Database/AssetDatabase.h"
#include "Engine/Core/Log.h"
#include "Engine/Assets/Registry/AssetTypeRegistry.h"

#include <filesystem>
#include <fstream>

namespace engine::assets {

    namespace fs = std::filesystem;

    using engine::core::usize;

    // ========================================================================
    // Registration
    // ========================================================================

    bool AssetDatabase::RegisterAsset(const AssetMetadata& metadata)
    {
        if (!metadata.IsValid())
        {
            ENGINE_LOG_WARN("AssetDatabase — invalid metadata (UUID={}, path='{}')",
                            metadata.UUID.ToString(), metadata.Path.Path);
            return false;
        }

        std::lock_guard lock(m_Mutex);

        if (m_ByUUID.count(metadata.UUID) > 0)
        {
            ENGINE_LOG_WARN("AssetDatabase — UUID already registered: {}",
                            metadata.UUID.ToString());
            return false;
        }

        m_ByUUID[metadata.UUID] = metadata;
        m_ByPath[metadata.Path] = metadata.UUID;
        if (!metadata.SourceFilePath.empty())
            m_BySourceFile[metadata.SourceFilePath] = metadata.UUID;

        return true;
    }

    bool AssetDatabase::UnregisterAsset(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_Mutex);

        auto it = m_ByUUID.find(uuid);
        if (it == m_ByUUID.end())
            return false;

        m_ByPath.erase(it->second.Path);
        if (!it->second.SourceFilePath.empty())
            m_BySourceFile.erase(it->second.SourceFilePath);

        m_Dependencies.erase(uuid);
        m_ReferencedBy.erase(uuid);

        // Remove from all reverse deps.
        for (auto& [depUUID, refs] : m_ReferencedBy)
        {
            refs.erase(std::remove(refs.begin(), refs.end(), uuid), refs.end());
        }

        m_ByUUID.erase(it);
        return true;
    }

    bool AssetDatabase::UpdateMetadata(const AssetUUID& uuid, const AssetMetadata& metadata)
    {
        std::lock_guard lock(m_Mutex);

        auto it = m_ByUUID.find(uuid);
        if (it == m_ByUUID.end())
            return false;

        // Update path mapping if path changed.
        if (it->second.Path != metadata.Path)
        {
            m_ByPath.erase(it->second.Path);
            m_ByPath[metadata.Path] = uuid;
        }

        // Update source file mapping if changed.
        if (it->second.SourceFilePath != metadata.SourceFilePath)
        {
            if (!it->second.SourceFilePath.empty())
                m_BySourceFile.erase(it->second.SourceFilePath);
            if (!metadata.SourceFilePath.empty())
                m_BySourceFile[metadata.SourceFilePath] = uuid;
        }

        it->second = metadata;
        it->second.UUID = uuid; // preserve UUID
        return true;
    }

    // ========================================================================
    // Lookup
    // ========================================================================

    const AssetMetadata* AssetDatabase::FindByUUID(const AssetUUID& uuid) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_ByUUID.find(uuid);
        return (it != m_ByUUID.end()) ? &it->second : nullptr;
    }

    const AssetMetadata* AssetDatabase::FindByPath(const AssetPath& path) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_ByPath.find(path);
        if (it == m_ByPath.end())
            return nullptr;
        auto metaIt = m_ByUUID.find(it->second);
        return (metaIt != m_ByUUID.end()) ? &metaIt->second : nullptr;
    }

    const AssetMetadata* AssetDatabase::FindBySourceFile(const std::string& sourceFilePath) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_BySourceFile.find(sourceFilePath);
        if (it == m_BySourceFile.end())
            return nullptr;
        auto metaIt = m_ByUUID.find(it->second);
        return (metaIt != m_ByUUID.end()) ? &metaIt->second : nullptr;
    }

    std::vector<const AssetMetadata*> AssetDatabase::FindByType(AssetType type) const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<const AssetMetadata*> result;
        for (const auto& [uuid, meta] : m_ByUUID)
        {
            if (meta.Type == type)
                result.push_back(&meta);
        }
        return result;
    }

    std::vector<AssetUUID> AssetDatabase::GetAllUUIDs() const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<AssetUUID> result;
        result.reserve(m_ByUUID.size());
        for (const auto& [uuid, _] : m_ByUUID)
            result.push_back(uuid);
        return result;
    }

    std::vector<const AssetMetadata*> AssetDatabase::GetAllMetadata() const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<const AssetMetadata*> result;
        result.reserve(m_ByUUID.size());
        for (const auto& [_, meta] : m_ByUUID)
            result.push_back(&meta);
        return result;
    }

    // ========================================================================
    // Dependency tracking
    // ========================================================================

    std::vector<AssetUUID> AssetDatabase::GetDependencies(const AssetUUID& uuid) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_Dependencies.find(uuid);
        if (it == m_Dependencies.end())
            return {};
        return it->second;
    }

    std::vector<AssetUUID> AssetDatabase::GetReferencedBy(const AssetUUID& uuid) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_ReferencedBy.find(uuid);
        if (it == m_ReferencedBy.end())
            return {};
        return it->second;
    }

    void AssetDatabase::AddDependency(const AssetUUID& uuid, const AssetUUID& dependency)
    {
        std::lock_guard lock(m_Mutex);
        m_Dependencies[uuid].push_back(dependency);
        m_ReferencedBy[dependency].push_back(uuid);
    }

    void AssetDatabase::RemoveDependency(const AssetUUID& uuid, const AssetUUID& dependency)
    {
        std::lock_guard lock(m_Mutex);

        auto& deps = m_Dependencies[uuid];
        deps.erase(std::remove(deps.begin(), deps.end(), dependency), deps.end());

        auto& refs = m_ReferencedBy[dependency];
        refs.erase(std::remove(refs.begin(), refs.end(), uuid), refs.end());
    }

    bool AssetDatabase::HasDependencies(const AssetUUID& uuid) const
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_Dependencies.find(uuid);
        return it != m_Dependencies.end() && !it->second.empty();
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    AssetStatistics AssetDatabase::GetStatistics() const
    {
        std::lock_guard lock(m_Mutex);
        AssetStatistics stats;
        stats.TotalAssets = static_cast<u32>(m_ByUUID.size());

        u32 totalDeps = 0;
        for (const auto& [_, meta] : m_ByUUID)
        {
            if (meta.SourceFileSize > 0)
                stats.TotalSourceSize += meta.SourceFileSize;

            switch (meta.Type)
            {
                case AssetType::Texture:   break;
                case AssetType::Shader:    break;
                default: break;
            }
        }

        for (const auto& [_, deps] : m_Dependencies)
            totalDeps += static_cast<u32>(deps.size());
        stats.TotalDependencies = totalDeps;

        return stats;
    }

    usize AssetDatabase::GetAssetCount() const noexcept
    {
        std::lock_guard lock(m_Mutex);
        return m_ByUUID.size();
    }

    // ========================================================================
    // Bulk operations
    // ========================================================================

    void AssetDatabase::Clear()
    {
        std::lock_guard lock(m_Mutex);
        m_ByUUID.clear();
        m_ByPath.clear();
        m_BySourceFile.clear();
        m_Dependencies.clear();
        m_ReferencedBy.clear();
    }

    usize AssetDatabase::ScanDirectory(const std::string& directoryPath,
                                         const std::string& scheme)
    {
        usize count = 0;

        if (!fs::exists(directoryPath))
        {
            ENGINE_LOG_WARN("AssetDatabase — directory does not exist: {}", directoryPath);
            return 0;
        }

        for (const auto& entry : fs::recursive_directory_iterator(directoryPath))
        {
            if (!entry.is_regular_file())
                continue;

            const auto ext = entry.path().extension().string();
            auto* typeEntry = AssetTypeRegistry::Get().FindByExtension(ext);
            if (!typeEntry)
                continue;

            AssetMetadata meta;
            meta.UUID = AssetUUID{};
            meta.Name = entry.path().stem().string();
            meta.Path = AssetPath(scheme, fs::relative(entry.path(), directoryPath).string());
            meta.Type = typeEntry->Type;
            meta.SourceFilePath = entry.path().string();
            meta.SourceFileSize = static_cast<u64>(fs::file_size(entry.path()));

            auto ftime = fs::last_write_time(entry.path());
            meta.SourceLastModified = static_cast<u64>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());

            if (RegisterAsset(meta))
                ++count;
        }

        ENGINE_LOG_INFO("AssetDatabase — scanned '{}', found {} assets", directoryPath, count);
        return count;
    }

    // ========================================================================
    // Validation
    // ========================================================================

    std::vector<AssetUUID> AssetDatabase::FindMissingAssets() const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<AssetUUID> result;
        for (const auto& [uuid, meta] : m_ByUUID)
        {
            if (!meta.SourceFilePath.empty()
                && !fs::exists(meta.SourceFilePath))
            {
                result.push_back(uuid);
            }
        }
        return result;
    }

    std::vector<AssetUUID> AssetDatabase::FindBrokenDependencies() const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<AssetUUID> result;
        for (const auto& [uuid, deps] : m_Dependencies)
        {
            for (const auto& dep : deps)
            {
                if (m_ByUUID.find(dep) == m_ByUUID.end())
                {
                    result.push_back(uuid);
                    break;
                }
            }
        }
        return result;
    }

    std::vector<AssetUUID> AssetDatabase::FindDuplicates() const
    {
        std::lock_guard lock(m_Mutex);
        std::vector<AssetUUID> result;
        std::unordered_map<std::string, std::vector<AssetUUID>> bySource;
        for (const auto& [uuid, meta] : m_ByUUID)
        {
            if (!meta.SourceFilePath.empty())
                bySource[meta.SourceFilePath].push_back(uuid);
        }
        for (const auto& [_, uuids] : bySource)
        {
            if (uuids.size() > 1)
            {
                for (const auto& u : uuids)
                    result.push_back(u);
            }
        }
        return result;
    }

} // namespace engine::assets
