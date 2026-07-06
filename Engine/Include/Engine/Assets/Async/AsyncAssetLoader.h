// ============================================================================
// File: Engine/Include/Engine/Assets/Async/AsyncAssetLoader.h
// Asynchronous asset loading framework.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Events/EventBus.h"
#include "Engine/Assets/Core/AssetTypes.h"
#include "Engine/Assets/References/AssetReference.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace engine::assets {

    class AssetManager;

    // ========================================================================
    // AsyncLoadPriority — priority of an async load request.
    // ========================================================================

    enum class AsyncLoadPriority : core::u32
    {
        Low    = 0,
        Normal = 1,
        High   = 2,
        Critical = 3
    };

    // ========================================================================
    // AsyncLoadResult — result of an async load operation.
    // ========================================================================

    struct AsyncLoadResult
    {
        AssetUUID UUID{};
        bool      Success{false};
        std::string ErrorMessage;
    };

    // ========================================================================
    // AsyncLoadRequest — a pending async load request.
    // ========================================================================

    struct AsyncLoadRequest
    {
        AssetUUID          UUID{};
        AsyncLoadPriority   Priority{AsyncLoadPriority::Normal};
        std::function<void(const AsyncLoadResult&)> Callback;

        bool operator<(const AsyncLoadRequest& other) const
        {
            return static_cast<u32>(Priority) < static_cast<u32>(other.Priority);
        }
    };

    // ========================================================================
    // AsyncAssetLoader — background asset loading.
    // ========================================================================

    /// @brief Manages asynchronous asset loading on background threads.
    ///        Requests are queued by priority and processed by worker
    ///        threads.  The main thread polls for completed requests.
    class AsyncAssetLoader
    {
    public:
        AsyncAssetLoader();
        ~AsyncAssetLoader();

        AsyncAssetLoader(const AsyncAssetLoader&)            = delete;
        AsyncAssetLoader& operator=(const AsyncAssetLoader&) = delete;

        /// @brief Starts the loader with @p threadCount worker threads.
        void Start(core::u32 threadCount = 2);

        /// @brief Stops the loader, waiting for in-progress loads.
        void Stop();

        /// @brief Queues an async load request.
        void RequestLoad(const AsyncLoadRequest& request);

        /// @brief Cancels a pending load request.
        void Cancel(const AssetUUID& uuid);

        /// @brief Returns the number of pending requests.
        [[nodiscard]] core::u32 GetPendingCount() const noexcept;

        /// @brief Returns the number of in-progress loads.
        [[nodiscard]] core::u32 GetActiveCount() const noexcept;

        /// @brief Returns the number of completed loads awaiting callback.
        [[nodiscard]] core::u32 GetCompletedCount() const noexcept;

        /// @brief Processes completed loads on the main thread, invoking
        ///        their callbacks.  Call this once per frame.
        void ProcessCompleted();

        /// @brief Returns true if the loader is running.
        [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }

        /// @brief Sets the event bus (for dispatching load events).
        void SetEventBus(events::EventBus* bus) noexcept { m_EventBus = bus; }

    private:
        void WorkerThread();

        std::atomic<bool>                              m_Running{false};
        std::vector<std::thread>                       m_Threads;

        mutable std::mutex                             m_QueueMutex;
        std::priority_queue<AsyncLoadRequest>           m_PendingQueue;
        std::unordered_map<AssetUUID, bool>             m_Active;
        std::condition_variable                         m_Condition;

        mutable std::mutex                             m_CompletedMutex;
        std::vector<std::pair<AsyncLoadRequest, AsyncLoadResult>> m_Completed;

        AssetManager*                                  m_Manager{nullptr};
        events::EventBus*                              m_EventBus{nullptr};

        friend class AssetManager;
    };

} // namespace engine::assets
