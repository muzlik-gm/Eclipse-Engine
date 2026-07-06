// ============================================================================
// File: Engine/Include/Engine/Assets/Filesystem/AssetWatcher.h
// Filesystem watcher for hot-reloading.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace engine::assets {

    namespace fs = std::filesystem;

    // ========================================================================
    // FileChangeType — type of filesystem change.
    // ========================================================================

    enum class FileChangeType : core::u32
    {
        Created = 0,
        Modified = 1,
        Deleted = 2,
        Renamed = 3
    };

    // ========================================================================
    // FileChangeEvent — describes a single file change.
    // ========================================================================

    struct FileChangeEvent
    {
        FileChangeType  Type{FileChangeType::Modified};
        std::string     FilePath;
        std::string     OldFilePath; // for Renamed
        core::u64       Timestamp{0};
    };

    /// @brief Callback invoked when a watched file changes.
    using FileChangeCallback = std::function<void(const FileChangeEvent&)>;

    // ========================================================================
    // AssetWatcher — watches directories for file changes.
    // ========================================================================

    /// @brief Watches directories for file changes and notifies
    ///        registered callbacks.  Used by the hot-reload system
    ///        to detect when source files are modified.
    class AssetWatcher
    {
    public:
        AssetWatcher();
        ~AssetWatcher();

        AssetWatcher(const AssetWatcher&)            = delete;
        AssetWatcher& operator=(const AssetWatcher&) = delete;

        /// @brief Starts watching @p directoryPath recursively.
        void Watch(const std::string& directoryPath, bool recursive = true);

        /// @brief Stops watching @p directoryPath.
        void Unwatch(const std::string& directoryPath);

        /// @brief Registers a callback for file changes.
        /// @return An opaque subscription id.
        core::u32 Subscribe(FileChangeCallback callback);

        /// @brief Removes a registered callback.
        void Unsubscribe(core::u32 subscriptionId);

        /// @brief Processes pending file changes on the main thread.
        ///        Call this once per frame.
        void ProcessChanges();

        /// @brief Returns true if the watcher is running.
        [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }

    private:
        void WatcherThread();

        struct WatchedDir
        {
            std::string                     Path;
            bool                            Recursive{true};
            std::unordered_map<std::string, fs::file_time_type> LastModified;
        };

        std::atomic<bool>                           m_Running{false};
        std::thread                                 m_Thread;
        mutable std::mutex                          m_WatchedMutex;
        std::vector<WatchedDir>                     m_Watched;

        mutable std::mutex                          m_CallbackMutex;
        std::unordered_map<core::u32, FileChangeCallback> m_Callbacks;
        core::u32                                    m_NextCallbackId{1};

        mutable std::mutex                          m_EventsMutex;
        std::vector<FileChangeEvent>                m_PendingEvents;
    };

} // namespace engine::assets
