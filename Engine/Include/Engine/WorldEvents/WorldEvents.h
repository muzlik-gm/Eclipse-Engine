// ============================================================================
// File: Engine/Include/Engine/WorldEvents/WorldEvents.h
// Runtime events fired by the World subsystem and WorldManager.
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/UUID.h"
#include "Engine/Core/Types.h"

#include <string_view>

namespace engine::world_events {

    using engine::core::f64;
    using engine::core::UUID;
    using engine::events::Event;
    using engine::events::EventType;
    using engine::events::EventCategory;

    // ========================================================================
    // WorldInitializedEvent — fired when World::Initialize completes.
    // ========================================================================
    class WorldInitializedEvent final : public Event
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "WorldInitialized"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::WorldInitialized; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::World | EventCategory::Application; }
    };

    // ========================================================================
    // WorldShutdownEvent — fired when World::Shutdown begins.
    // ========================================================================
    class WorldShutdownEvent final : public Event
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "WorldShutdown"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::WorldShutdown; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::World | EventCategory::Application; }
    };

    // ========================================================================
    // ActiveSceneChangedEvent — fired when the active scene changes.
    // ========================================================================
    class ActiveSceneChangedEvent final : public Event
    {
    public:
        ActiveSceneChangedEvent(UUID oldScene, UUID newScene)
            : m_OldScene(oldScene), m_NewScene(newScene) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ActiveSceneChanged"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::ActiveSceneChanged; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::World | EventCategory::Scene; }

        [[nodiscard]] UUID GetOldScene() const noexcept { return m_OldScene; }
        [[nodiscard]] UUID GetNewScene() const noexcept { return m_NewScene; }

    private:
        UUID m_OldScene;
        UUID m_NewScene;
    };

    // ========================================================================
    // WorldTickEvent — fired at the start of each World::Update.
    // ========================================================================
    class WorldTickEvent final : public Event
    {
    public:
        explicit WorldTickEvent(f64 deltaTime)
            : m_DeltaTime(deltaTime) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "WorldTick"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::WorldTick; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::World; }

        [[nodiscard]] f64 GetDeltaTime() const noexcept { return m_DeltaTime; }

    private:
        f64 m_DeltaTime;
    };

} // namespace engine::world_events
