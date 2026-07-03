// ============================================================================
// File: Engine/Include/Engine/Events/EventBus.h
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace engine::events {

    using engine::core::usize;

    /// Central event dispatcher / pub-sub bus.
    /// Listeners subscribe to a specific EventType and are notified in
    /// registration order when a matching event is dispatched.
    class EventBus
    {
    public:
        using Listener = std::function<void(Event&)>;

        EventBus()  = default;
        ~EventBus() = default;

        EventBus(const EventBus&)            = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&)                 = delete;
        EventBus& operator=(EventBus&&)      = delete;

        /// Register a listener for the given event type.
        /// Returns an opaque subscription id that can be used with Unsubscribe.
        [[nodiscard]] usize Subscribe(EventType type, Listener listener);

        /// Remove all listeners for the given event type.
        void Unsubscribe(EventType type);

        /// Dispatch an event to all registered listeners of the matching type.
        /// If a listener sets the event to Handled, subsequent listeners still
        /// receive the event (they may choose to check IsHandled).
        void Dispatch(Event& event);

    private:
        std::unordered_map<EventType, std::vector<Listener>> m_Listeners;
        std::mutex                                             m_Mutex;
    };

} // namespace engine::events