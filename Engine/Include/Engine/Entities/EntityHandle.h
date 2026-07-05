// ============================================================================
// File: Engine/Include/Engine/Entities/EntityHandle.h
// Lightweight opaque entity handle with fluent API.
// ============================================================================
#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Registry.h"

namespace engine::entities {

    // ========================================================================
    // EntityHandle — fluent, value-like handle that binds an entity to its
    //                registry for convenient component access.
    // ========================================================================

    /// @brief Combines an Entity with a reference to its owning Registry,
    ///        providing a fluent interface for component manipulation.
    ///
    /// EntityHandle is designed to be cheaply copied.  It does not manage
    /// entity lifetime — it simply forwards calls to the underlying registry.
    class EntityHandle
    {
    public:
        /// @brief Constructs a handle from an entity and its owning registry.
        EntityHandle(ecs::Entity entity, ecs::Registry& registry) noexcept
            : m_entity(entity)
            , m_registry(&registry)
        {
        }

        // -- Observers ----------------------------------------------------------

        /// @brief Returns true if the underlying entity is still valid/alive.
        [[nodiscard]] bool IsValid() const noexcept
        {
            return ecs::IsValid(m_entity) && m_registry && m_registry->IsValid(m_entity);
        }

        /// @brief Returns the raw Entity identifier.
        [[nodiscard]] ecs::Entity GetId() const noexcept { return m_entity; }

        // -- Fluent component API -----------------------------------------------

        /// @brief Adds (or replaces) a component of type T.  Returns *this
        ///        for chaining.
        template <typename T, typename... Args>
        EntityHandle& AddComponent(Args&&... args)
        {
            m_registry->AddComponent<T>(m_entity, std::forward<Args>(args)...);
            return *this;
        }

        /// @brief Removes the component of type T.  Returns *this for chaining.
        template <typename T>
        EntityHandle& RemoveComponent()
        {
            m_registry->RemoveComponent<T>(m_entity);
            return *this;
        }

        /// @brief Returns a reference to the component of type T.
        template <typename T>
        [[nodiscard]] T& GetComponent()
        {
            return m_registry->GetComponent<T>(m_entity);
        }

        /// @brief Returns true if the entity has a component of type T.
        template <typename T>
        [[nodiscard]] bool HasComponent() const
        {
            return m_registry->HasComponent<T>(m_entity);
        }

        // -- Lifetime -----------------------------------------------------------

        /// @brief Destroys the underlying entity and all its components.
        void Destroy()
        {
            m_registry->DestroyEntity(m_entity);
        }

        // -- Operators ----------------------------------------------------------

        /// @brief Truthy when the handle refers to a valid, alive entity.
        [[nodiscard]] explicit operator bool() const noexcept { return IsValid(); }

        /// @brief Equality comparison — same entity in the same registry.
        [[nodiscard]] bool operator==(const EntityHandle& other) const noexcept
        {
            return m_entity == other.m_entity && m_registry == other.m_registry;
        }

        [[nodiscard]] bool operator!=(const EntityHandle& other) const noexcept
        {
            return !(*this == other);
        }

    private:
        ecs::Entity    m_entity;
        ecs::Registry* m_registry;
    };

} // namespace engine::entities