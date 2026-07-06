// ============================================================================
// File: Engine/Include/Engine/Scene/Scene.h
// Owns an ECS registry, a system scheduler, and entity lifetime helpers.
// ============================================================================
#pragma once

#include "Engine/ECS/Registry.h"
#include "Engine/Core/UUID.h"
#include "Engine/Systems/ISystem.h"
#include "Engine/Systems/SystemScheduler.h"
#include "Engine/Entities/EntityHandle.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Events/EventBus.h"
#include "Engine/SceneEvents/SceneEvents.h"

#include <memory>
#include <string>

namespace engine::scene {

    using engine::core::f64;
    using engine::core::usize;
    using engine::events::EventBus;

    // ========================================================================
    // Scene
    // ========================================================================

    /// @brief A self-contained ECS world with its own registry, systems,
    ///        and entity management.
    ///
    /// Each Scene is identified by a UUID and a human-readable name.  The
    /// World subsystem (engine::world::World) manages the lifetime of
    /// multiple scenes and selects one as the active scene for simulation.
    ///
    /// Systems are managed by a SystemScheduler that respects named groups,
    /// integer priorities, and declared dependencies.
    class Scene
    {
    public:
        // -- Constructors / Destructor -----------------------------------------

        /// @brief Constructs a scene with the given name and a random UUID.
        /// @param eventBus Optional EventBus for dispatching entity and
        ///                 component events.  May be nullptr.
        explicit Scene(std::string name, EventBus* eventBus = nullptr);

        ~Scene() = default;

        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&)                 = delete;
        Scene& operator=(Scene&&)      = delete;

        // -- Observers ---------------------------------------------------------

        [[nodiscard]] const core::UUID& GetUUID() const noexcept { return m_uuid; }
        [[nodiscard]] const std::string& GetName() const noexcept { return m_name; }
        [[nodiscard]] ecs::Registry& GetRegistry() noexcept { return m_registry; }
        [[nodiscard]] const ecs::Registry& GetRegistry() const noexcept { return m_registry; }
        [[nodiscard]] bool IsActive() const noexcept { return m_isActive; }
        void SetActive(bool active) noexcept { m_isActive = active; }

        /// @brief Returns the system scheduler that owns this scene's systems.
        [[nodiscard]] systems::SystemScheduler& GetSystemScheduler() noexcept { return m_Scheduler; }
        [[nodiscard]] const systems::SystemScheduler& GetSystemScheduler() const noexcept { return m_Scheduler; }

        // -- Entity lifetime ---------------------------------------------------

        /// @brief Creates a new entity with a TagComponent and IDComponent.
        ///
        /// @param tag Human-readable label (default "Entity").
        /// @return An EntityHandle bound to the new entity and this scene's
        ///         registry.
        entities::EntityHandle CreateEntity(const std::string& tag = "Entity");

        /// @brief Destroys the given entity and all its components.
        void DestroyEntity(ecs::Entity entity);

        // -- System management -------------------------------------------------

        /// @brief Constructs a system of type @p T, calls OnAttach, and
        ///        registers it with the system scheduler.
        ///
        /// @tparam T  Concrete system type (must derive from ISystem).
        /// @tparam Args Constructor argument types for @p T.
        /// @param args Arguments forwarded to @p T's constructor.
        /// @return A reference to the newly created system.
        template <typename T, typename... Args>
        T& AddSystem(Args&&... args)
        {
            return m_Scheduler.Add<T>(m_registry, "Default", 0,
                                      std::forward<Args>(args)...);
        }

        /// @brief Registers a system with explicit group and priority.
        template <typename T, typename... Args>
        T& AddSystemWithGroup(const std::string& group,
                              engine::core::i32 priority,
                              Args&&... args)
        {
            return m_Scheduler.Add<T>(m_registry, group, priority,
                                      std::forward<Args>(args)...);
        }

        // -- Per-frame updates -------------------------------------------------

        /// @brief Calls Update on every enabled system via the scheduler.
        void OnUpdate(f64 deltaTime);

        /// @brief Calls FixedUpdate on every enabled system via the scheduler.
        void OnFixedUpdate(f64 fixedDeltaTime);

        /// @brief Calls LateUpdate on every enabled system via the scheduler.
        void OnLateUpdate(f64 deltaTime);

        // -- Queries -----------------------------------------------------------

        /// @brief Returns the number of alive entities in the registry.
        ///        Counts entities that have a TagComponent (added by CreateEntity).
        [[nodiscard]] usize EntityCount() const
        {
            usize count = 0;
            for ([[maybe_unused]] auto entity : m_registry.View<components::TagComponent>())
            {
                (void)entity;
                ++count;
            }
            return count;
        }

    private:
        core::UUID                            m_uuid;
        std::string                           m_name;
        ecs::Registry                         m_registry;
        systems::SystemScheduler              m_Scheduler;
        EventBus*                             m_EventBus{nullptr};
        bool                                  m_isActive{false};
    };

} // namespace engine::scene
