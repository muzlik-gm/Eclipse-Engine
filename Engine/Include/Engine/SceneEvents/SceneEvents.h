// ============================================================================
// File: Engine/Include/Engine/SceneEvents/SceneEvents.h
// Runtime events fired by the Scene, ECS Registry, and hierarchy utilities.
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/UUID.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Core/Types.h"

#include <string>
#include <string_view>

namespace engine::scene_events {

    using engine::core::UUID;
    using engine::ecs::Entity;
    using engine::events::Event;
    using engine::events::EventType;
    using engine::events::EventCategory;

    // ========================================================================
    // Entity events
    // ========================================================================

    class EntityCreatedEvent final : public Event
    {
    public:
        EntityCreatedEvent(Entity entity, UUID sceneUUID)
            : m_Entity(entity), m_SceneUUID(sceneUUID) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "EntityCreated"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::EntityCreated; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Entity | EventCategory::Scene; }

        [[nodiscard]] Entity GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] UUID   GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        Entity m_Entity;
        UUID   m_SceneUUID;
    };

    class EntityDestroyedEvent final : public Event
    {
    public:
        explicit EntityDestroyedEvent(Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "EntityDestroyed"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::EntityDestroyed; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Entity | EventCategory::Scene; }

        [[nodiscard]] Entity GetEntity() const noexcept { return m_Entity; }

    private:
        Entity m_Entity;
    };

    // ========================================================================
    // Component events
    // ========================================================================

    class ComponentAddedEvent final : public Event
    {
    public:
        ComponentAddedEvent(Entity entity, std::string componentName)
            : m_Entity(entity), m_ComponentName(std::move(componentName)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ComponentAdded"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::ComponentAdded; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Component | EventCategory::Entity; }

        [[nodiscard]] Entity             GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] const std::string& GetComponentName() const noexcept { return m_ComponentName; }

    private:
        Entity      m_Entity;
        std::string m_ComponentName;
    };

    class ComponentRemovedEvent final : public Event
    {
    public:
        ComponentRemovedEvent(Entity entity, std::string componentName)
            : m_Entity(entity), m_ComponentName(std::move(componentName)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ComponentRemoved"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::ComponentRemoved; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Component | EventCategory::Entity; }

        [[nodiscard]] Entity             GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] const std::string& GetComponentName() const noexcept { return m_ComponentName; }

    private:
        Entity      m_Entity;
        std::string m_ComponentName;
    };

    class ComponentModifiedEvent final : public Event
    {
    public:
        ComponentModifiedEvent(Entity entity, std::string componentName)
            : m_Entity(entity), m_ComponentName(std::move(componentName)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ComponentModified"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::ComponentModified; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Component | EventCategory::Entity; }

        [[nodiscard]] Entity             GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] const std::string& GetComponentName() const noexcept { return m_ComponentName; }

    private:
        Entity      m_Entity;
        std::string m_ComponentName;
    };

    // ========================================================================
    // Hierarchy / Transform / Visibility events
    // ========================================================================

    class TransformChangedEvent final : public Event
    {
    public:
        explicit TransformChangedEvent(Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "TransformChanged"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::TransformChanged; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Entity; }

        [[nodiscard]] Entity GetEntity() const noexcept { return m_Entity; }

    private:
        Entity m_Entity;
    };

    class HierarchyChangedEvent final : public Event
    {
    public:
        explicit HierarchyChangedEvent(Entity entity)
            : m_Entity(entity) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "HierarchyChanged"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::HierarchyChanged; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Entity; }

        [[nodiscard]] Entity GetEntity() const noexcept { return m_Entity; }

    private:
        Entity m_Entity;
    };

    class VisibilityChangedEvent final : public Event
    {
    public:
        VisibilityChangedEvent(Entity entity, bool visible)
            : m_Entity(entity), m_Visible(visible) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "VisibilityChanged"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::VisibilityChanged; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Entity; }

        [[nodiscard]] Entity GetEntity() const noexcept { return m_Entity; }
        [[nodiscard]] bool   IsVisible() const noexcept { return m_Visible; }

    private:
        Entity m_Entity;
        bool   m_Visible;
    };

    // ========================================================================
    // Scene lifecycle events
    // ========================================================================

    class SceneLoadedEvent final : public Event
    {
    public:
        explicit SceneLoadedEvent(UUID sceneUUID)
            : m_SceneUUID(sceneUUID) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "SceneLoaded"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::SceneLoaded; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Scene | EventCategory::World; }

        [[nodiscard]] UUID GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        UUID m_SceneUUID;
    };

    class SceneUnloadedEvent final : public Event
    {
    public:
        explicit SceneUnloadedEvent(UUID sceneUUID)
            : m_SceneUUID(sceneUUID) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "SceneUnloaded"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::SceneUnloaded; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Scene | EventCategory::World; }

        [[nodiscard]] UUID GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        UUID m_SceneUUID;
    };

    class SceneActivatedEvent final : public Event
    {
    public:
        explicit SceneActivatedEvent(UUID sceneUUID)
            : m_SceneUUID(sceneUUID) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "SceneActivated"; }
        [[nodiscard]] EventType       GetEventType() const noexcept override { return EventType::SceneActivated; }
        [[nodiscard]] EventCategory   GetCategoryFlags() const noexcept override { return EventCategory::Scene | EventCategory::World; }

        [[nodiscard]] UUID GetSceneUUID() const noexcept { return m_SceneUUID; }

    private:
        UUID m_SceneUUID;
    };

} // namespace engine::scene_events
