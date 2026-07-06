// ============================================================================
// File: Engine/Include/Engine/ECS/Registry.h
// Core ECS registry — wraps entt::registry with a type-safe engine API.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Log.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Events/EventBus.h"
#include "Engine/SceneEvents/SceneEvents.h"

#include <entt/entt.hpp>
#include <string>
#include <type_traits>

namespace engine::ecs {

    using engine::core::usize;
    using engine::events::EventBus;

    // ========================================================================
    // Registry — central ECS container for entities and components.
    // ========================================================================

    /// @brief Wraps an entt::registry and exposes a type-safe engine API.
    ///
    /// All entity lifetime and component storage is managed by the underlying
    /// entt::registry instance. This class provides a thin wrapper that
    /// uses engine types throughout and optionally dispatches component
    /// events through a bound EventBus.
    class Registry
    {
    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&)            = delete;
        Registry& operator=(const Registry&) = delete;
        Registry(Registry&&)                 = delete;
        Registry& operator=(Registry&&)      = delete;

        // -- EventBus binding --------------------------------------------------

        /// @brief Binds an EventBus so that ComponentAdded / ComponentRemoved
        ///        events are dispatched on every Add / Remove call.
        ///        Pass nullptr to disable event dispatch.
        void SetEventBus(EventBus* bus) noexcept { m_EventBus = bus; }

        /// @brief Returns the bound EventBus, or nullptr if none is bound.
        [[nodiscard]] EventBus* GetEventBus() const noexcept { return m_EventBus; }

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
            if (entity == engine::ecs::Invalid || !m_registry.valid(entity))
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

            if (m_EventBus)
            {
                scene_events::ComponentAddedEvent event(entity,
                    std::string(demangleTypeName<T>()));
                m_EventBus->Dispatch(event);
            }

            return m_registry.get<T>(entity);
        }

        /// @brief Removes a component of type T from the given entity.
        template <typename T>
        void RemoveComponent(Entity entity)
        {
            if (m_EventBus)
            {
                scene_events::ComponentRemovedEvent event(entity,
                    std::string(demangleTypeName<T>()));
                m_EventBus->Dispatch(event);
            }

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

        // -- Groups -------------------------------------------------------------

        /// @brief Returns an entt::group for the specified owned and observed
        ///        component types.  Groups provide sorted, cache-friendly
        ///        iteration when multiple systems query the same components.
        template <typename... Owned, typename... Observed>
        [[nodiscard]] auto Group(Observed... observed)
        {
            return m_registry.group<Owned...>(observed...);
        }

        /// @brief Const group accessor.
        template <typename... Owned, typename... Observed>
        [[nodiscard]] auto Group(Observed... observed) const
        {
            return m_registry.group<Owned...>(observed...);
        }

        // -- Observers (signals) ------------------------------------------------

        /// @brief Returns the entt::registry's on_construct signal for type T.
        ///        Connect a free function or lambda to be notified whenever
        ///        a component of type T is added to any entity.
        template <typename T>
        [[nodiscard]] auto& OnConstruct()
        {
            return m_registry.on_construct<T>();
        }

        /// @brief Returns the entt::registry's on_destroy signal for type T.
        template <typename T>
        [[nodiscard]] auto& OnDestroy()
        {
            return m_registry.on_destroy<T>();
        }

        /// @brief Returns the entt::registry's on_update signal for type T.
        template <typename T>
        [[nodiscard]] auto& OnUpdate()
        {
            return m_registry.on_update<T>();
        }

        // -- Bulk operations ----------------------------------------------------

        /// @brief Destroys all entities and their components.
        void Clear()
        {
            m_registry.clear();
        }

        // -- Advanced access ----------------------------------------------------

        /// @brief Returns a mutable reference to the underlying entt::registry.
        ///        Intended for advanced use cases (custom views, signals, etc.).
        [[nodiscard]] entt::registry& GetInner() { return m_registry; }

        /// @brief Returns a const reference to the underlying entt::registry.
        [[nodiscard]] const entt::registry& GetInner() const { return m_registry; }

    private:
        // -- Helper: produce a human-readable type name -------------------------
        template <typename T>
        static const char* demangleTypeName() noexcept
        {
#if defined(__GNUC__) || defined(__clang__)
            return __PRETTY_FUNCTION__;
#else
            return typeid(T).name();
#endif
        }

        entt::registry m_registry;
        EventBus*      m_EventBus{nullptr};
    };

} // namespace engine::ecs
