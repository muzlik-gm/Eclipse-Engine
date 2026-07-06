// ============================================================================
// File: Engine/Include/Engine/Assets/Core/AssetManager.h
// Top-level asset manager — the public API for the asset system.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Events/EventBus.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/Core/IAsset.h"
#include "Engine/Assets/References/AssetReference.h"
#include "Engine/Assets/Metadata/AssetMetadata.h"
#include "Engine/Assets/Database/AssetDatabase.h"
#include "Engine/Assets/Filesystem/AssetFilesystem.h"
#include "Engine/Assets/Filesystem/AssetWatcher.h"
#include "Engine/Assets/Async/AsyncAssetLoader.h"
#include "Engine/Assets/Serialization/AssetSerializer.h"
#include "Engine/Assets/Importers/IAssetImporter.h"

#include <memory>
#include <string>

namespace engine::assets {

    // ========================================================================
    // AssetManager — top-level asset system facade.
    // ========================================================================

    /// @brief The AssetManager is the single entry point for all asset
    ///        operations.  It owns the database, filesystem, async loader,
    ///        watcher, and serializers.  It is registered as an engine
    ///        subsystem and participates in the standard lifecycle.
    class AssetManager final : public runtime::ISubsystem
    {
    public:
        AssetManager();
        ~AssetManager() override;

        AssetManager(const AssetManager&)            = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        // -- ISubsystem interface ------------------------------------------

        [[nodiscard]] std::string_view GetName() const noexcept override
        { return "AssetManager"; }

        [[nodiscard]] std::vector<std::string> GetDependencies() const override
        { return {}; }

        bool Initialize() override;
        void Shutdown() override;

        void Update(core::f64 deltaTime) override;
        void FixedUpdate(core::f64 fixedDeltaTime) override {}
        void LateUpdate(core::f64 deltaTime) override {}

        // -- EventBus binding ----------------------------------------------

        void SetEventBus(events::EventBus* bus) noexcept { m_EventBus = bus; }

        // -- Filesystem ----------------------------------------------------

        /// @brief Returns the virtual filesystem.
        [[nodiscard]] AssetFilesystem& GetFilesystem() noexcept { return m_Filesystem; }

        /// @brief Mounts a directory for a scheme.
        void MountDirectory(const std::string& scheme, const std::string& path);

        // -- Database ------------------------------------------------------

        /// @brief Returns the asset database.
        [[nodiscard]] AssetDatabase& GetDatabase() noexcept { return m_Database; }

        // -- Asset access --------------------------------------------------

        /// @brief Loads an asset by UUID synchronously.
        /// @return A strong reference, or null if not found.
        [[nodiscard]] AssetRef LoadAsset(const AssetUUID& uuid);

        /// @brief Loads an asset by virtual path synchronously.
        [[nodiscard]] AssetRef LoadAssetByPath(const AssetPath& path);

        /// @brief Queues an async load.  The callback is invoked on the
        ///        main thread when the load completes.
        void LoadAssetAsync(const AssetUUID& uuid,
                            AsyncLoadPriority priority = AsyncLoadPriority::Normal,
                            std::function<void(const AsyncLoadResult&)> callback = {});

        /// @brief Unloads an asset, releasing its runtime data.
        void UnloadAsset(const AssetUUID& uuid);

        /// @brief Returns a weak reference to an asset without loading it.
        [[nodiscard]] AssetWeakRef GetWeakRef(const AssetUUID& uuid);

        /// @brief Returns metadata for an asset.
        [[nodiscard]] const AssetMetadata* GetMetadata(const AssetUUID& uuid) const;

        // -- Importing -----------------------------------------------------

        /// @brief Imports a file into the asset database.
        /// @return The UUID of the imported asset, or empty on failure.
        [[nodiscard]] AssetUUID ImportFile(const std::string& filePath,
                                            const ImportSettings* settings = nullptr);

        /// @brief Reimports an existing asset from its source file.
        bool Reimport(const AssetUUID& uuid);

        // -- Hot reload ----------------------------------------------------

        /// @brief Enables or disables hot-reloading.
        void SetHotReloadEnabled(bool enabled) noexcept { m_HotReloadEnabled = enabled; }

        /// @brief Returns true if hot-reloading is enabled.
        [[nodiscard]] bool IsHotReloadEnabled() const noexcept { return m_HotReloadEnabled; }

        /// @brief Reloads a specific asset from disk.
        bool ReloadAsset(const AssetUUID& uuid);

        // -- Serialization -------------------------------------------------

        /// @brief Returns the serializer for @p format.
        [[nodiscard]] IAssetSerializer* GetSerializer(SerializationFormat format) const;

        /// @brief Saves an asset's metadata to a file.
        bool SaveMetadata(const AssetUUID& uuid, const std::string& filePath,
                          SerializationFormat format = SerializationFormat::JSON);

        /// @brief Loads asset metadata from a file.
        [[nodiscard]] AssetMetadata LoadMetadata(const std::string& filePath,
                                                  SerializationFormat format = SerializationFormat::JSON);

        // -- Statistics ----------------------------------------------------

        [[nodiscard]] AssetStatistics GetStatistics() const;

        // -- Async ---------------------------------------------------------

        /// @brief Returns the async loader.
        [[nodiscard]] AsyncAssetLoader& GetAsyncLoader() noexcept { return m_AsyncLoader; }

    private:
        void OnFileChanged(const FileChangeEvent& event);
        void CreateDefaultSerializers();
        void DestroyDefaultSerializers();

        events::EventBus*                          m_EventBus{nullptr};
        AssetFilesystem                            m_Filesystem;
        AssetDatabase                              m_Database;
        AsyncAssetLoader                           m_AsyncLoader;
        AssetWatcher                               m_Watcher;
        bool                                       m_HotReloadEnabled{true};

        std::unique_ptr<IAssetSerializer>          m_BinarySerializer;
        std::unique_ptr<IAssetSerializer>          m_JsonSerializer;
        std::unique_ptr<IAssetSerializer>          m_YamlSerializer;

        // Loaded assets keyed by UUID.
        std::unordered_map<AssetUUID, std::unique_ptr<IAsset>> m_LoadedAssets;
        std::unordered_map<AssetUUID, std::shared_ptr<AssetControlBlock>> m_ControlBlocks;
        mutable std::mutex                        m_AssetMutex;

        core::u32                                  m_WatcherSubscriptionId{0};
    };

} // namespace engine::assets
