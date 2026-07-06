// ============================================================================
// File: Engine/Source/Renderer/Queue/RenderQueue.cpp
// ============================================================================
#include "Engine/Renderer/Queue/RenderQueue.h"
#include <algorithm>

namespace engine::renderer {

    void RenderQueue::Sort()
    {
        std::sort(m_Entries.begin(), m_Entries.end());
    }

    void RenderQueue::ForEach(
        const std::function<void(const RenderQueueEntry&)>& callback) const
    {
        for (const auto& entry : m_Entries)
            callback(entry);
    }

    // ========================================================================
    // RenderQueueManager
    // ========================================================================

    RenderQueueManager::RenderQueueManager()
    {
        // Queues are default-constructed.
    }

    void RenderQueueManager::ClearAll()
    {
        for (int i = 0; i < 6; ++i)
            m_Queues[i].Clear();
    }

    RenderQueue& RenderQueueManager::GetQueue(RenderQueueType type)
    {
        u32 idx = static_cast<u32>(type);
        if (idx >= 6) idx = 0; // fallback to opaque
        return m_Queues[idx];
    }

    void RenderQueueManager::SortAll()
    {
        for (int i = 0; i < 6; ++i)
            m_Queues[i].Sort();
    }

    void RenderQueueManager::Submit(RenderQueueType type, const RenderQueueEntry& entry)
    {
        GetQueue(type).Submit(entry);
    }

    u32 RenderQueueManager::GetTotalCount() const noexcept
    {
        u32 total = 0;
        for (int i = 0; i < 6; ++i)
            total += m_Queues[i].GetCount();
        return total;
    }

} // namespace engine::renderer
