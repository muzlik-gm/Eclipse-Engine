// ============================================================================
// File: Engine/Source/Scene/Scene.cpp
// Scene implementation.
// ============================================================================

#include "Engine/Scene/Scene.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"

namespace engine::scene {

    // ========================================================================
    // Constructor
    // ========================================================================

    Scene::Scene(std::string name, EventBus* eventBus)
        : m_uuid{}          // Default-constructs a random UUID v4.
        , m_name(std::move(name))
        , m_EventBus(eventBus)
    {
        m_registry.SetEventBus(m_EventBus);
    }

    // ========================================================================
    // Entity lifetime
    // ========================================================================

    entities::EntityHandle Scene::CreateEntity(const std::string& tag)
    {
        ecs::Entity entity = m_registry.CreateEntity();

        m_registry.AddComponent<components::TagComponent>(entity, components::TagComponent{tag});
        m_registry.AddComponent<components::IDComponent>(entity, components::IDComponent{core::UUID{}});

        if (m_EventBus)
        {
            scene_events::EntityCreatedEvent event(entity, m_uuid);
            m_EventBus->Dispatch(event);
        }

        return entities::EntityHandle{entity, m_registry};
    }

    void Scene::DestroyEntity(ecs::Entity entity)
    {
        if (m_EventBus)
        {
            scene_events::EntityDestroyedEvent event(entity);
            m_EventBus->Dispatch(event);
        }

        m_registry.DestroyEntity(entity);
    }

    // ========================================================================
    // Per-frame updates — delegate to SystemScheduler
    // ========================================================================

    void Scene::OnUpdate(f64 deltaTime)
    {
        m_Scheduler.Update(deltaTime);
    }

    void Scene::OnFixedUpdate(f64 fixedDeltaTime)
    {
        m_Scheduler.FixedUpdate(fixedDeltaTime);
    }

    void Scene::OnLateUpdate(f64 deltaTime)
    {
        m_Scheduler.LateUpdate(deltaTime);
    }

} // namespace engine::scene
