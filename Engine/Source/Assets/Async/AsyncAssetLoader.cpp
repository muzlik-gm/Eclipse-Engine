// ============================================================================
// File: Engine/Source/Assets/Async/AsyncAssetLoader.cpp
// ============================================================================
#include "Engine/Assets/Async/AsyncAssetLoader.h"
#include "Engine/Assets/Core/AssetManager.h"
#include "Engine/Core/Log.h"

namespace engine::assets {

    using engine::core::u32;

    AsyncAssetLoader::AsyncAssetLoader() = default;

    AsyncAssetLoader::~AsyncAssetLoader()
    {
        Stop();
    }

    void AsyncAssetLoader::Start(u32 threadCount)
    {
        if (m_Running)
            return;

        m_Running = true;
        m_Threads.reserve(threadCount);
        for (u32 i = 0; i < threadCount; ++i)
        {
            m_Threads.emplace_back(&AsyncAssetLoader::WorkerThread, this);
        }

        ENGINE_LOG_INFO("AsyncAssetLoader — started with {} threads", threadCount);
    }

    void AsyncAssetLoader::Stop()
    {
        if (!m_Running)
            return;

        m_Running = false;
        m_Condition.notify_all();

        for (auto& t : m_Threads)
        {
            if (t.joinable())
                t.join();
        }
        m_Threads.clear();

        ENGINE_LOG_INFO("AsyncAssetLoader — stopped");
    }

    void AsyncAssetLoader::RequestLoad(const AsyncLoadRequest& request)
    {
        {
            std::lock_guard lock(m_QueueMutex);
            m_PendingQueue.push(request);
            m_Active[request.UUID] = true;
        }
        m_Condition.notify_one();
    }

    void AsyncAssetLoader::Cancel(const AssetUUID& uuid)
    {
        std::lock_guard lock(m_QueueMutex);
        m_Active.erase(uuid);
        // Note: we cannot remove from the priority_queue easily; the
        // worker will check m_Active before processing.
    }

    u32 AsyncAssetLoader::GetPendingCount() const noexcept
    {
        std::lock_guard lock(m_QueueMutex);
        return static_cast<u32>(m_PendingQueue.size());
    }

    u32 AsyncAssetLoader::GetActiveCount() const noexcept
    {
        std::lock_guard lock(m_QueueMutex);
        return static_cast<u32>(m_Active.size());
    }

    u32 AsyncAssetLoader::GetCompletedCount() const noexcept
    {
        std::lock_guard lock(m_CompletedMutex);
        return static_cast<u32>(m_Completed.size());
    }

    void AsyncAssetLoader::ProcessCompleted()
    {
        std::vector<std::pair<AsyncLoadRequest, AsyncLoadResult>> completed;
        {
            std::lock_guard lock(m_CompletedMutex);
            completed.swap(m_Completed);
        }

        for (const auto& [request, result] : completed)
        {
            if (request.Callback)
                request.Callback(result);
        }
    }

    void AsyncAssetLoader::WorkerThread()
    {
        while (m_Running)
        {
            AsyncLoadRequest request;

            {
                std::unique_lock lock(m_QueueMutex);
                m_Condition.wait(lock, [this] {
                    return !m_Running || !m_PendingQueue.empty();
                });

                if (!m_Running)
                    return;

                if (m_PendingQueue.empty())
                    continue;

                request = m_PendingQueue.top();
                m_PendingQueue.pop();

                // Check if still active (not cancelled).
                auto activeIt = m_Active.find(request.UUID);
                if (activeIt == m_Active.end())
                    continue;
            }

            // Perform the load on this background thread.
            AsyncLoadResult result;
            result.UUID = request.UUID;

            try
            {
                if (m_Manager)
                {
                    auto ref = m_Manager->LoadAsset(request.UUID);
                    result.Success = ref.IsValid();
                    if (!result.Success)
                        result.ErrorMessage = "Failed to load asset";
                }
                else
                {
                    result.Success = false;
                    result.ErrorMessage = "No asset manager";
                }
            }
            catch (const std::exception& e)
            {
                result.Success = false;
                result.ErrorMessage = e.what();
            }

            {
                std::lock_guard lock(m_QueueMutex);
                m_Active.erase(request.UUID);
            }

            {
                std::lock_guard lock(m_CompletedMutex);
                m_Completed.emplace_back(request, result);
            }
        }
    }

} // namespace engine::assets
