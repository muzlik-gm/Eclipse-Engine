// ============================================================================
// File: Engine/Include/Engine/Scene/Scene.h
// Owns an ECS registry, a list of systems, and entity lifetime helpers.
// ============================================================================
#pragma once

#include "Engine/ECS/Registry.h"
#include "Engine/Core/UUID.h"
#include "Engine/Systems/ISystem.h"
#include "Engine/Entities/EntityHandle.h"
#include "Engine/Components/TagComponent.h"

#include <memory>
#include <string>
#include <vector>

namespace engine::scene {

    using engine::core::f64;
    using engine::core::usize;

    // ========================================================================
    // Scene
    // ========================================================================

    /// @brief A self-contained ECS world with its own registry, systems,
    ///        and entity management.
    ///
    /// Each Scene is identified by a UUID and a human-readable name.  The
    /// World subsystem (engine::world::World) manages the lifetime of
    /// multiple scenes and selects one as the active scene for simulation.
    class Scene
    {
    public:
        // -- Constructors / Destructor -----------------------------------------

        /// @brief Constructs a scene with the given name and a random UUID.
        explicit Scene(std::string name);

        ~Scene() = default;

        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&)                 = delete;
        Scene& operator=(Scene&&)      = delete;

        // -- Observers ---------------------------------------------------------

        [[nodiscard]] const core::UUID& GetUUID() const noexcept { return m_uuid; }
        [[nodiscard]] const std::string& GetName() const noexcept { return m_name; }
        [[nodiscard]] ecs::Registry& GetRegistry() noexcept { return m_registry; }
        [[nodiscard]] bool IsActive() const noexcept { return m_isActive; }
        void SetActive(bool active) noexcept { m_isActive = active; }

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
        ///        adds it to the scene.
        ///
        /// @tparam T  Concrete system type (must derive from ISystem).
        /// @tparam Args Constructor argument types for @p T.
        /// @param args Arguments forwarded to @p T's constructor.
        /// @return A reference to the newly created system.
        template <typename T, typename... Args>
        T& AddSystem(Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->OnAttach(m_registry);
            T& ref = *system;
            m_systems.push_back(std::move(system));
            return ref;
        }

        // -- Per-frame updates -------------------------------------------------

        /// @brief Calls Update on every enabled system.
        void OnUpdate(f64 deltaTime);

        /// @brief Calls FixedUpdate on every enabled system.
        void OnFixedUpdate(f64 fixedDeltaTime);

        // -- Queries -----------------------------------------------------------

        /// @brief Returns the number of alive entities in the registry.
        ///        Counts entities that have a TagComponent (added by CreateEntity).
        [[nodiscard]] usize EntityCount()
        {
            usize count = 0;
            for ([[maybe_unused]] auto entity : m_registry.View<components::TagComponent>()) { (void)entity; ++count; }
            return count;
        }

    private:
        core::UUID                                  m_uuid;
        std::string                                 m_name;
        ecs::Registry                               m_registry;
        std::vector<std::unique_ptr<systems::ISystem>> m_systems;
        bool                                        m_isActive{false};
    };

} // namespace engine::scene