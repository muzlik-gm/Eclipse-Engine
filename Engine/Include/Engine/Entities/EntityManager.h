// ============================================================================
// File: Engine/Include/Engine/Entities/EntityManager.h
// Higher-level entity management facade on top of Registry.
// ============================================================================
#pragma once

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Core/UUID.h"

#include <string>
#include <vector>

namespace engine::entities {

    using engine::core::UUID;
    using engine::core::usize;

    /// @brief Convenience layer on top of ecs::Registry.
    class EntityManager
    {
    public:
        explicit EntityManager(ecs::Registry& registry) noexcept
            : m_registry(&registry) {}

        [[nodiscard]] ecs::Entity Create(const std::string& tag = "Entity");
        [[nodiscard]] ecs::Entity CreateNamed(const std::string& tag, const std::string& name);
        void Destroy(ecs::Entity entity);
        [[nodiscard]] bool IsValid(ecs::Entity entity) const;
        [[nodiscard]] ecs::Entity FindByTag(const std::string& tag) const;
        [[nodiscard]] ecs::Entity FindByUUID(const UUID& uuid) const;
        [[nodiscard]] std::vector<ecs::Entity> GetAll() const;

    private:
        ecs::Registry* m_registry;
    };

} // namespace engine::entities
