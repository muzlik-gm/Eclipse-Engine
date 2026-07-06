// ============================================================================
// File: Engine/Source/Assets/Filesystem/AssetWatcher.cpp
// ============================================================================
#include "Engine/Assets/Filesystem/AssetWatcher.h"
#include "Engine/Core/Log.h"

#include <chrono>
#include <filesystem>
#include <unordered_set>

namespace engine::assets {

    namespace fs = std::filesystem;

    AssetWatcher::AssetWatcher() = default;

    AssetWatcher::~AssetWatcher()
    {
        m_Running = false;
        if (m_Thread.joinable())
            m_Thread.join();
    }

    void AssetWatcher::Watch(const std::string& directoryPath, bool recursive)
    {
        {
            std::lock_guard lock(m_WatchedMutex);

            // Check if already watching.
            for (const auto& d : m_Watched)
            {
                if (d.Path == directoryPath)
                    return;
            }

            WatchedDir dir;
            dir.Path = directoryPath;
            dir.Recursive = recursive;

            // Snapshot current file timestamps.
            if (fs::exists(directoryPath))
            {
                if (recursive)
                {
                    for (const auto& entry : fs::recursive_directory_iterator(directoryPath))
                    {
                        if (entry.is_regular_file())
                        {
                            dir.LastModified[entry.path().string()] =
                                fs::last_write_time(entry.path());
                        }
                    }
                }
                else
                {
                    for (const auto& entry : fs::directory_iterator(directoryPath))
                    {
                        if (entry.is_regular_file())
                        {
                            dir.LastModified[entry.path().string()] =
                                fs::last_write_time(entry.path());
                        }
                    }
                }
            }

            m_Watched.push_back(std::move(dir));
        }

        // Start the watcher thread if not already running.
        if (!m_Running)
        {
            m_Running = true;
            m_Thread = std::thread(&AssetWatcher::WatcherThread, this);
        }

        ENGINE_LOG_INFO("AssetWatcher — watching '{}'", directoryPath);
    }

    void AssetWatcher::Unwatch(const std::string& directoryPath)
    {
        std::lock_guard lock(m_WatchedMutex);
        m_Watched.erase(
            std::remove_if(m_Watched.begin(), m_Watched.end(),
                           [&](const WatchedDir& d) { return d.Path == directoryPath; }),
            m_Watched.end());
    }

    u32 AssetWatcher::Subscribe(FileChangeCallback callback)
    {
        std::lock_guard lock(m_CallbackMutex);
        u32 id = m_NextCallbackId++;
        m_Callbacks[id] = std::move(callback);
        return id;
    }

    void AssetWatcher::Unsubscribe(u32 subscriptionId)
    {
        std::lock_guard lock(m_CallbackMutex);
        m_Callbacks.erase(subscriptionId);
    }

    void AssetWatcher::ProcessChanges()
    {
        std::vector<FileChangeEvent> events;
        {
            std::lock_guard lock(m_EventsMutex);
            events.swap(m_PendingEvents);
        }

        if (events.empty())
            return;

        std::unordered_map<u32, FileChangeCallback> callbacks;
        {
            std::lock_guard lock(m_CallbackMutex);
            callbacks = m_Callbacks;
        }

        for (const auto& event : events)
        {
            for (const auto& [_, cb] : callbacks)
            {
                if (cb)
                    cb(event);
            }
        }
    }

    void AssetWatcher::WatcherThread()
    {
        using namespace std::chrono_literals;

        while (m_Running)
        {
            std::this_thread::sleep_for(500ms); // Poll every 500ms.

            std::lock_guard lock(m_WatchedMutex);

            for (auto& dir : m_Watched)
            {
                if (!fs::exists(dir.Path))
                    continue;

                std::unordered_set<std::string> currentFiles;

                auto processEntry = [&](const fs::directory_entry& entry)
                {
                    if (!entry.is_regular_file())
                        return;

                    auto path = entry.path().string();
                    currentFiles.insert(path);

                    auto current_time = fs::last_write_time(entry.path());
                    auto it = dir.LastModified.find(path);

                    if (it == dir.LastModified.end())
                    {
                        FileChangeEvent event;
                        event.Type = FileChangeType::Created;
                        event.FilePath = path;
                        event.Timestamp = static_cast<engine::core::u64>(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count());

                        {
                            std::lock_guard elock(m_EventsMutex);
                            m_PendingEvents.push_back(event);
                        }

                        dir.LastModified[path] = current_time;
                    }
                    else if (it->second != current_time)
                    {
                        FileChangeEvent event;
                        event.Type = FileChangeType::Modified;
                        event.FilePath = path;
                        event.Timestamp = static_cast<engine::core::u64>(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count());

                        {
                            std::lock_guard elock(m_EventsMutex);
                            m_PendingEvents.push_back(event);
                        }

                        it->second = current_time;
                    }
                };

                if (dir.Recursive)
                {
                    for (const auto& entry : fs::recursive_directory_iterator(dir.Path,
                        fs::directory_options::skip_permission_denied))
                    {
                        processEntry(entry);
                    }
                }
                else
                {
                    for (const auto& entry : fs::directory_iterator(dir.Path))
                    {
                        processEntry(entry);
                    }
                }

                // Check for deleted files.
                std::vector<std::string> deleted;
                for (const auto& [path, _] : dir.LastModified)
                {
                    if (currentFiles.find(path) == currentFiles.end())
                        deleted.push_back(path);
                }

                for (const auto& path : deleted)
                {
                    FileChangeEvent event;
                    event.Type = FileChangeType::Deleted;
                    event.FilePath = path;
                    event.Timestamp = static_cast<engine::core::u64>(
                        std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count());

                    {
                        std::lock_guard elock(m_EventsMutex);
                        m_PendingEvents.push_back(event);
                    }

                    dir.LastModified.erase(path);
                }
            }
        }
    }

} // namespace engine::assets
