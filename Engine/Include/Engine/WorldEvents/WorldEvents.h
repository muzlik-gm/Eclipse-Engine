// ============================================================================
// File: Engine/Include/Engine/WorldEvents/WorldEvents.h
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/UUID.h"
#include "Engine/Core/Types.h"

#include <string_view>

namespace engine::world_events {

    using engine::core::u32;
    using engine::core::f64;
    using engine::events::Event;
    using engine::events::EventType;
    using engine::events::EventCategory;

    // ========================================================================
    // WorldInitializedEvent
    // ========================================================================
    class WorldInitializedEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WorldInitialized"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }
    };

    // ========================================================================
    // WorldShutdownEvent
    // ========================================================================
    class WorldShutdownEvent final : public Event
    {
    public:
        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WorldShutdown"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }
    };

    // ========================================================================
    // ActiveSceneChangedEvent
    // ========================================================================
    class ActiveSceneChangedEvent final : public Event
    {
    public:
        explicit ActiveSceneChangedEvent(core::UUID oldScene, core::UUID newScene)
            : m_OldScene(oldScene), m_NewScene(newScene) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "ActiveSceneChanged"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr core::UUID GetOldScene() const noexcept { return m_OldScene; }
        [[nodiscard]] constexpr core::UUID GetNewScene() const noexcept { return m_NewScene; }

    private:
        core::UUID m_OldScene;
        core::UUID m_NewScene;
    };

    // ========================================================================
    // WorldTickEvent
    // ========================================================================
    class WorldTickEvent final : public Event
    {
    public:
        explicit WorldTickEvent(f64 deltaTime)
            : m_DeltaTime(deltaTime) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "WorldTick"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr f64 GetDeltaTime() const noexcept { return m_DeltaTime; }

    private:
        f64 m_DeltaTime;
    };

} // namespace engine::world_events