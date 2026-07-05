// ============================================================================
// File: Engine/Source/Entities/EntityManager.cpp
// ============================================================================
#include "Engine/Entities/EntityManager.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/NameComponent.h"

namespace engine::entities {

    ecs::Entity EntityManager::Create(const std::string& tag)
    {
        ecs::Entity entity = m_registry->CreateEntity();
        m_registry->AddComponent<components::TagComponent>(entity, components::TagComponent{tag});
        m_registry->AddComponent<components::IDComponent>(entity, components::IDComponent{UUID{}});
        return entity;
    }

    ecs::Entity EntityManager::CreateNamed(const std::string& tag, const std::string& name)
    {
        ecs::Entity entity = Create(tag);
        m_registry->AddComponent<components::NameComponent>(entity, components::NameComponent{name});
        return entity;
    }

    void EntityManager::Destroy(ecs::Entity entity)
    {
        m_registry->DestroyEntity(entity);
    }

    bool EntityManager::IsValid(ecs::Entity entity) const
    {
        return m_registry->IsValid(entity);
    }

    ecs::Entity EntityManager::FindByTag(const std::string& tag) const
    {
        for (auto eh : m_registry->View<components::TagComponent>())
        {
            ecs::Entity entity{eh};
            auto& tc = m_registry->GetComponent<components::TagComponent>(entity);
            if (tc.Tag == tag)
                return entity;
        }
        return ecs::Invalid;
    }

    ecs::Entity EntityManager::FindByUUID(const UUID& uuid) const
    {
        for (auto eh : m_registry->View<components::IDComponent>())
        {
            ecs::Entity entity{eh};
            auto& idc = m_registry->GetComponent<components::IDComponent>(entity);
            if (idc.ID == uuid)
                return entity;
        }
        return ecs::Invalid;
    }

    std::vector<ecs::Entity> EntityManager::GetAll() const
    {
        std::vector<ecs::Entity> result;
        for (auto eh : m_registry->View<components::TagComponent>())
        {
            result.push_back(ecs::Entity{eh});
        }
        return result;
    }

} // namespace engine::entities
