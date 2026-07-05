// ============================================================================
// File: Engine/Include/Engine/Systems/ISystem.h
// Pure virtual base class for per-scene ECS systems.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Registry.h"

#include <string>
#include <string_view>

namespace engine::systems {

    using engine::core::f64;

    // ========================================================================
    // ISystem
    // ========================================================================

    /// @brief Pure virtual interface for per-scene ECS systems.
    ///
    /// Each Scene owns a collection of ISystem instances.  Systems are
    /// notified when they are attached to / detached from a scene and
    /// receive per-frame update callbacks.  The default implementations
    /// are no-ops so that derived classes only override what they need.
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        // ----------------------------------------------------------------
        // Identity
        // ----------------------------------------------------------------

        /// @brief Returns the human-readable name of this system.
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// @brief Called when the system is added to a scene.
        ///
        /// @param registry A mutable reference to the scene's ECS registry.
        virtual void OnAttach(ecs::Registry& registry) {}

        /// @brief Called when the system is removed from a scene.
        virtual void OnDetach() {}

        // ----------------------------------------------------------------
        // Per-frame updates
        // ----------------------------------------------------------------

        /// @brief Called every frame with variable delta time.
        virtual void Update(f64 deltaTime) {}

        /// @brief Called at a fixed rate (physics, networking, etc.).
        virtual void FixedUpdate(f64 fixedDeltaTime) {}

        /// @brief Called every frame after Update.
        virtual void LateUpdate(f64 deltaTime) {}

        // ----------------------------------------------------------------
        // Enable / Disable
        // ----------------------------------------------------------------

        /// @brief Enables the system.
        virtual void Enable() {}

        /// @brief Disables the system.
        virtual void Disable() {}

        /// @brief Returns true when the system is enabled.
        [[nodiscard]] virtual bool IsEnabled() const { return true; }
    };

} // namespace engine::systems