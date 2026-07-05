// ============================================================================
// File: Engine/Include/Engine/SceneEvents/SceneEvents.h
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/UUID.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Core/Types.h"

#include <string>
#include <string_view>

namespace engine::scene_events {

    using engine::core::u32;
    using engine::events::Event;
    using engine::events::EventType;
    using engine::events::EventCategory;

    // ========================================================================
    // EntityCreatedEvent
    // ========================================================================
    class EntityCreatedEvent final : public Event
    {
    public:
        explicit EntityCreatedEvent(ecs::Entity entity, core::UUID sceneUUID)
            : m_Entity(entity), m_SceneUUID(sceneUUID) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "EntityCreated"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity   GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] constexpr core::UUID    GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        ecs::Entity m_Entity;
        core::UUID  m_SceneUUID;
    };

    // ========================================================================
    // EntityDestroyedEvent
    // ========================================================================
    class EntityDestroyedEvent final : public Event
    {
    public:
        explicit EntityDestroyedEvent(ecs::Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "EntityDestroyed"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity GetEntity() const noexcept { return m_Entity; }

    private:
        ecs::Entity m_Entity;
    };

    // ========================================================================
    // ComponentAddedEvent
    // ========================================================================
    class ComponentAddedEvent final : public Event
    {
    public:
        explicit ComponentAddedEvent(ecs::Entity entity, std::string componentName)
            : m_Entity(entity), m_ComponentName(std::move(componentName)) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "ComponentAdded"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity          GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] constexpr const std::string&   GetComponentName() const noexcept { return m_ComponentName; }

    private:
        ecs::Entity m_Entity;
        std::string m_ComponentName;
    };

    // ========================================================================
    // ComponentRemovedEvent
    // ========================================================================
    class ComponentRemovedEvent final : public Event
    {
    public:
        explicit ComponentRemovedEvent(ecs::Entity entity, std::string componentName)
            : m_Entity(entity), m_ComponentName(std::move(componentName)) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "ComponentRemoved"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity          GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] constexpr const std::string&   GetComponentName() const noexcept { return m_ComponentName; }

    private:
        ecs::Entity m_Entity;
        std::string m_ComponentName;
    };

    // ========================================================================
    // TransformChangedEvent
    // ========================================================================
    class TransformChangedEvent final : public Event
    {
    public:
        explicit TransformChangedEvent(ecs::Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "TransformChanged"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity GetEntity() const noexcept { return m_Entity; }

    private:
        ecs::Entity m_Entity;
    };

    // ========================================================================
    // HierarchyChangedEvent
    // ========================================================================
    class HierarchyChangedEvent final : public Event
    {
    public:
        explicit HierarchyChangedEvent(ecs::Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "HierarchyChanged"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr ecs::Entity GetEntity() const noexcept { return m_Entity; }

    private:
        ecs::Entity m_Entity;
    };

    // ========================================================================
    // SceneLoadedEvent
    // ========================================================================
    class SceneLoadedEvent final : public Event
    {
    public:
        explicit SceneLoadedEvent(core::UUID sceneUUID)
            : m_SceneUUID(sceneUUID) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "SceneLoaded"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr core::UUID GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        core::UUID m_SceneUUID;
    };

    // ========================================================================
    // SceneUnloadedEvent
    // ========================================================================
    class SceneUnloadedEvent final : public Event
    {
    public:
        explicit SceneUnloadedEvent(core::UUID sceneUUID)
            : m_SceneUUID(sceneUUID) {}

        [[nodiscard]] constexpr std::string_view GetName() const noexcept override { return "SceneUnloaded"; }
        [[nodiscard]] constexpr EventType       GetEventType() const noexcept override { return EventType::None; }
        [[nodiscard]] constexpr EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] constexpr core::UUID GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        core::UUID m_SceneUUID;
    };

} // namespace engine::scene_events