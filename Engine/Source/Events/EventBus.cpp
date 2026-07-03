// ============================================================================
// File: Engine/Source/Events/EventBus.cpp
// ============================================================================

#include "Engine/Events/EventBus.h"

#include "Engine/Core/Log.h"

#include <cstdint>

namespace engine::events {

    usize EventBus::Subscribe(EventType type, Listener listener)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto& listeners = m_Listeners[type];
        listeners.push_back(std::move(listener));

        const usize index = listeners.size(); // 1-based
        ENGINE_LOG_TRACE("EventBus: Subscribed listener {} for event type {}", index, static_cast<std::uint32_t>(type));

        return index;
    }

    void EventBus::Unsubscribe(EventType type)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        const auto it = m_Listeners.find(type);
        if (it != m_Listeners.end() && !it->second.empty())
        {
            const usize count = it->second.size();
            it->second.clear();
            ENGINE_LOG_TRACE("EventBus: Unsubscribed {} listener(s) for event type {}", count, static_cast<std::uint32_t>(type));
        }
    }

    void EventBus::Dispatch(Event& event)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        const auto it = m_Listeners.find(event.GetEventType());
        if (it == m_Listeners.end() || it->second.empty())
        {
            return;
        }

        ENGINE_LOG_TRACE("EventBus: Dispatching '{}' to {} listener(s)", event.GetName(), it->second.size());

        for (auto& listener : it->second)
        {
            listener(event);
        }
    }

} // namespace engine::events