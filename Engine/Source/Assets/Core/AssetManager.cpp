// ============================================================================
// File: Engine/Source/Assets/Core/AssetManager.cpp
// ============================================================================
#include "Engine/Assets/Core/AssetManager.h"
#include "Engine/Assets/Registry/AssetTypeRegistry.h"
#include "Engine/Assets/Importers/ImporterRegistry.h"
#include "Engine/Assets/Events/AssetEvents.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/UUID.h"

#include <fstream>
#include <sstream>

namespace engine::assets {

    using namespace engine::core;

    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    AssetManager::AssetManager()
    {
        CreateDefaultSerializers();
    }

    AssetManager::~AssetManager()
    {
        Shutdown();
        DestroyDefaultSerializers();
    }

    // ========================================================================
    // ISubsystem interface
    // ========================================================================

    bool AssetManager::Initialize()
    {
        ENGINE_LOG_INFO("AssetManager — initializing");

        m_AsyncLoader.SetEventBus(m_EventBus);
        m_AsyncLoader.Start(2);
        m_AsyncLoader.m_Manager = this;

        // Start the file watcher.
        m_Watcher.Subscribe([this](const FileChangeEvent& e) { OnFileChanged(e); });
        // m_Watcher starts its thread lazily when Watch() is called.

        ENGINE_LOG_INFO("AssetManager — initialized");
        return true;
    }

    void AssetManager::Shutdown()
    {
        ENGINE_LOG_INFO("AssetManager — shutting down");

        m_Watcher.Unsubscribe(m_WatcherSubscriptionId);
        m_AsyncLoader.Stop();

        std::lock_guard lock(m_AssetMutex);
        m_LoadedAssets.clear();
        m_ControlBlocks.clear();

        ENGINE_LOG_INFO("AssetManager — shut down");
    }

    void AssetManager::Update(f64 /*deltaTime*/)
    {
        // Process completed async loads.
        m_AsyncLoader.ProcessCompleted();

        // Process file change events.
        if (m_HotReloadEnabled)
        {
            m_Watcher.ProcessChanges();
        }
    }

    // ========================================================================
    // Filesystem
    // ========================================================================

    void AssetManager::MountDirectory(const std::string& scheme, const std::string& path)
    {
        m_Filesystem.Mount(scheme, path);
        ENGINE_LOG_INFO("AssetManager — mounted '{}' -> '{}'", scheme, path);
    }

    // ========================================================================
    // Asset access
    // ========================================================================

    AssetRef AssetManager::LoadAsset(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_AssetMutex);

        auto it = m_LoadedAssets.find(uuid);
        if (it != m_LoadedAssets.end())
        {
            // Already loaded — return a strong ref.
            auto& cb = m_ControlBlocks[uuid];
            if (!cb)
                cb = std::make_shared<AssetControlBlock>();
            cb->State.store(AssetState::Loaded, std::memory_order_release);
            return AssetRef(it->second.get(), cb.get());
        }

        // Not loaded — look up metadata and create the asset.
        auto* meta = m_Database.FindByUUID(uuid);
        if (!meta)
        {
            ENGINE_LOG_WARN("AssetManager — asset {} not found in database", uuid.ToString());
            return AssetRef{};
        }

        // Find the factory for this asset type.
        auto* entry = AssetTypeRegistry::Get().Find(meta->Type);
        if (!entry || !entry->Factory)
        {
            ENGINE_LOG_ERROR("AssetManager — no factory for asset type {}",
                             static_cast<u32>(meta->Type));
            return AssetRef{};
        }

        auto asset = entry->Factory->Create(*meta);
        if (!asset)
        {
            ENGINE_LOG_ERROR("AssetManager — factory returned null for {}", uuid.ToString());
            return AssetRef{};
        }

        if (!asset->Load())
        {
            ENGINE_LOG_ERROR("AssetManager — failed to load asset: {}", asset->GetLastError());
            return AssetRef{};
        }

        auto* raw = asset.get();
        m_LoadedAssets[uuid] = std::move(asset);

        auto& cb = m_ControlBlocks[uuid];
        cb = std::make_shared<AssetControlBlock>();
        cb->State.store(AssetState::Loaded, std::memory_order_release);

        if (m_EventBus)
        {
            asset_events::AssetLoadedEvent event(uuid);
            m_EventBus->Dispatch(event);
        }

        return AssetRef(raw, cb.get());
    }

    AssetRef AssetManager::LoadAssetByPath(const AssetPath& path)
    {
        auto* meta = m_Database.FindByPath(path);
        if (!meta)
            return AssetRef{};
        return LoadAsset(meta->UUID);
    }

    void AssetManager::LoadAssetAsync(const AssetUUID& uuid,
                                       AsyncLoadPriority priority,
                                       std::function<void(const AsyncLoadResult&)> callback)
    {
        AsyncLoadRequest request;
        request.UUID = uuid;
        request.Priority = priority;
        request.Callback = std::move(callback);
        m_AsyncLoader.RequestLoad(request);
    }

    void AssetManager::UnloadAsset(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_AssetMutex);

        auto it = m_LoadedAssets.find(uuid);
        if (it == m_LoadedAssets.end())
            return;

        it->second->Unload();

        if (m_EventBus)
        {
            asset_events::AssetUnloadedEvent event(uuid);
            m_EventBus->Dispatch(event);
        }

        m_LoadedAssets.erase(it);
    }

    AssetWeakRef AssetManager::GetWeakRef(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_AssetMutex);
        auto it = m_ControlBlocks.find(uuid);
        if (it == m_ControlBlocks.end() || !it->second)
            return AssetWeakRef{};
        return AssetWeakRef(AssetRef(m_LoadedAssets.count(uuid) ? m_LoadedAssets[uuid].get() : nullptr,
                                       it->second.get()));
    }

    const AssetMetadata* AssetManager::GetMetadata(const AssetUUID& uuid) const
    {
        return m_Database.FindByUUID(uuid);
    }

    // ========================================================================
    // Importing
    // ========================================================================

    AssetUUID AssetManager::ImportFile(const std::string& filePath,
                                        const ImportSettings* settings)
    {
        // Find an importer for this file.
        auto* importer = ImporterRegistry::Get().FindForFile(filePath);
        if (!importer)
        {
            ENGINE_LOG_ERROR("AssetManager — no importer for '{}'", filePath);
            return AssetUUID{};
        }

        ImportContext context;
        context.SourceFilePath = filePath;
        context.Settings = settings ? settings : importer->CreateDefaultSettings().get();
        context.TargetType = importer->GetAssetType();

        if (m_EventBus)
        {
            asset_events::ImportStartedEvent event(AssetUUID{}, filePath);
            m_EventBus->Dispatch(event);
        }

        auto result = importer->Import(context);
        if (!result.Success)
        {
            ENGINE_LOG_ERROR("AssetManager — import failed: {}", result.ErrorMessage);
            if (m_EventBus)
            {
                asset_events::ImportFailedEvent event(AssetUUID{}, result.ErrorMessage);
                m_EventBus->Dispatch(event);
            }
            return AssetUUID{};
        }

        // Register imported assets in the database.
        AssetUUID firstUUID{};
        for (const auto& meta : result.ImportedAssets)
        {
            m_Database.RegisterAsset(meta);
            if (!firstUUID.IsValid())
                firstUUID = meta.UUID;

            if (m_EventBus)
            {
                asset_events::AssetCreatedEvent event(meta.UUID, meta.Type);
                m_EventBus->Dispatch(event);
            }
        }

        if (m_EventBus)
        {
            asset_events::ImportFinishedEvent event(firstUUID, true);
            m_EventBus->Dispatch(event);
        }

        ENGINE_LOG_INFO("AssetManager — imported '{}' ({} assets)", filePath, result.ImportedAssets.size());
        return firstUUID;
    }

    bool AssetManager::Reimport(const AssetUUID& uuid)
    {
        auto* meta = m_Database.FindByUUID(uuid);
        if (!meta)
            return false;

        return ReloadAsset(uuid);
    }

    // ========================================================================
    // Hot reload
    // ========================================================================

    bool AssetManager::ReloadAsset(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_AssetMutex);

        auto it = m_LoadedAssets.find(uuid);
        if (it == m_LoadedAssets.end())
            return false;

        if (!it->second->Reload())
        {
            ENGINE_LOG_ERROR("AssetManager — reload failed for {}", uuid.ToString());
            return false;
        }

        // Increment generation.
        auto cbIt = m_ControlBlocks.find(uuid);
        if (cbIt != m_ControlBlocks.end() && cbIt->second)
        {
            cbIt->second->Generation.fetch_add(1, std::memory_order_release);
        }

        if (m_EventBus)
        {
            asset_events::AssetReloadedEvent event(uuid);
            m_EventBus->Dispatch(event);
        }

        ENGINE_LOG_INFO("AssetManager — reloaded {}", uuid.ToString());
        return true;
    }

    void AssetManager::OnFileChanged(const FileChangeEvent& event)
    {
        if (!m_HotReloadEnabled)
            return;

        if (event.Type == FileChangeType::Modified || event.Type == FileChangeType::Created)
        {
            // Find the asset associated with this file.
            auto* meta = m_Database.FindBySourceFile(event.FilePath);
            if (meta)
            {
                ENGINE_LOG_DEBUG("AssetManager — file changed, reloading {}",
                                 meta->UUID.ToString());
                ReloadAsset(meta->UUID);
            }
        }
    }

    // ========================================================================
    // Serialization
    // ========================================================================

    IAssetSerializer* AssetManager::GetSerializer(SerializationFormat format) const
    {
        switch (format)
        {
            case SerializationFormat::Binary: return m_BinarySerializer.get();
            case SerializationFormat::JSON:   return m_JsonSerializer.get();
            case SerializationFormat::YAML:   return m_YamlSerializer.get();
        }
        return nullptr;
    }

    bool AssetManager::SaveMetadata(const AssetUUID& uuid, const std::string& filePath,
                                     SerializationFormat format)
    {
        auto* meta = m_Database.FindByUUID(uuid);
        if (!meta)
            return false;

        auto* serializer = GetSerializer(format);
        if (!serializer)
            return false;

        auto data = serializer->SerializeMetadata(*meta);
        return serializer->WriteToFile(filePath, data);
    }

    AssetMetadata AssetManager::LoadMetadata(const std::string& filePath,
                                              SerializationFormat format)
    {
        auto* serializer = GetSerializer(format);
        if (!serializer)
            return AssetMetadata{};

        auto data = serializer->ReadFromFile(filePath);
        return serializer->DeserializeMetadata(data);
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    AssetStatistics AssetManager::GetStatistics() const
    {
        return m_Database.GetStatistics();
    }

    // ========================================================================
    // Default serializers
    // ========================================================================

    void AssetManager::CreateDefaultSerializers()
    {
        m_BinarySerializer = std::make_unique<BinarySerializer>();
        m_JsonSerializer   = std::make_unique<JsonSerializer>();
        m_YamlSerializer   = std::make_unique<YamlSerializer>();
    }

    void AssetManager::DestroyDefaultSerializers()
    {
        m_BinarySerializer.reset();
        m_JsonSerializer.reset();
        m_YamlSerializer.reset();
    }

} // namespace engine::assets
