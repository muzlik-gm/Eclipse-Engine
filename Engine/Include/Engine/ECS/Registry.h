// ============================================================================
// File: Engine/Include/Engine/ECS/Registry.h
// Core ECS registry — wraps entt::registry with a type-safe engine API.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

#include <entt/entt.hpp>
#include <type_traits>

namespace engine::ecs {

    using engine::core::usize;

    // ========================================================================
    // Registry — central ECS container for entities and components.
    // ========================================================================

    /// @brief Wraps an entt::registry and exposes a type-safe engine API.
    ///
    /// All entity lifetime and component storage is managed by the underlying
    /// entt::registry instance. This class provides a thin wrapper that
    /// uses engine types throughout.
    class Registry
    {
    public:
        // -- Entity lifetime ----------------------------------------------------

        /// @brief Creates a new entity and returns its handle.
        [[nodiscard]] Entity CreateEntity()
        {
            return m_registry.create();
        }

        /// @brief Destroys the given entity and all its components.
        ///        If the entity is null or not alive, this is a no-op.
        void DestroyEntity(Entity entity)
        {
            if (entity == entt::null || !m_registry.valid(entity))
                return;
            m_registry.destroy(entity);
        }

        // -- Queries ------------------------------------------------------------

        /// @brief Returns true if the entity is currently alive in the registry.
        [[nodiscard]] bool IsValid(Entity entity) const
        {
            return m_registry.valid(entity);
        }

        // -- Component management (templated) -----------------------------------

        /// @brief Adds (or replaces) a component of type T on the given entity.
        template <typename T, typename... Args>
        T& AddComponent(Entity entity, Args&&... args)
        {
            m_registry.emplace_or_replace<T>(
                entity, std::forward<Args>(args)...);
            return m_registry.get<T>(entity);
        }

        /// @brief Removes a component of type T from the given entity.
        template <typename T>
        void RemoveComponent(Entity entity)
        {
            m_registry.remove<T>(entity);
        }

        /// @brief Returns a reference to the component of type T on the entity.
        template <typename T>
        [[nodiscard]] T& GetComponent(Entity entity)
        {
            return m_registry.get<T>(entity);
        }

        /// @brief Const access to component of type T.
        template <typename T>
        [[nodiscard]] const T& GetComponent(Entity entity) const
        {
            return m_registry.get<T>(entity);
        }

        /// @brief Returns true if the entity has a component of type T.
        template <typename T>
        [[nodiscard]] bool HasComponent(Entity entity) const
        {
            return m_registry.all_of<T>(entity);
        }

        /// @brief Returns an entt::view over all entities that have every
        ///        specified component type.
        template <typename... Ts>
        [[nodiscard]] auto View()
        {
            return m_registry.view<Ts...>();
        }

        /// @brief Const view over entities with all specified components.
        template <typename... Ts>
        [[nodiscard]] auto View() const
        {
            return m_registry.view<Ts...>();
        }

        // -- Bulk operations ----------------------------------------------------

        /// @brief Destroys all entities and their components.
        void Clear()
        {
            m_registry.clear();
        }

        // -- Advanced access ----------------------------------------------------

        /// @brief Returns a mutable reference to the underlying entt::registry.
        [[nodiscard]] entt::registry& GetInner() { return m_registry; }

        /// @brief Returns a const reference to the underlying entt::registry.
        [[nodiscard]] const entt::registry& GetInner() const { return m_registry; }

    private:
        entt::registry m_registry;
    };

} // namespace engine::ecs