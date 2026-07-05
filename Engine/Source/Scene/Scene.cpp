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

    Scene::Scene(std::string name)
        : m_uuid{}          // Default-constructs a random UUID v4.
        , m_name(std::move(name))
    {
    }

    // ========================================================================
    // Entity lifetime
    // ========================================================================

    entities::EntityHandle Scene::CreateEntity(const std::string& tag)
    {
        ecs::Entity entity = m_registry.CreateEntity();

        m_registry.AddComponent<components::TagComponent>(entity, components::TagComponent{tag});
        m_registry.AddComponent<components::IDComponent>(entity, components::IDComponent{core::UUID{}});

        return entities::EntityHandle{entity, m_registry};
    }

    void Scene::DestroyEntity(ecs::Entity entity)
    {
        m_registry.DestroyEntity(entity);
    }

    // ========================================================================
    // Per-frame updates
    // ========================================================================

    void Scene::OnUpdate(f64 deltaTime)
    {
        for (auto& system : m_systems)
        {
            if (system && system->IsEnabled())
            {
                system->Update(deltaTime);
            }
        }
    }

    void Scene::OnFixedUpdate(f64 fixedDeltaTime)
    {
        for (auto& system : m_systems)
        {
            if (system && system->IsEnabled())
            {
                system->FixedUpdate(fixedDeltaTime);
            }
        }
    }

} // namespace engine::scene