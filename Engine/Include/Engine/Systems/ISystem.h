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

    /// @brief Pure virtual base class for per-scene ECS systems.
    ///
    /// Each Scene owns a collection of ISystem instances.  Systems are
    /// notified when they are attached to / detached from a scene and
    /// receive per-frame update callbacks.  The default implementations
    /// of Update / FixedUpdate / LateUpdate / OnAttach / OnDetach are
    /// no-ops so derived classes override only what they need.
    ///
    /// The enable / disable state is owned by this base class so every
    /// system inherits functional Enable() / Disable() / IsEnabled()
    /// without having to re-implement it.
    class ISystem
    {
    public:
        ISystem() = default;
        virtual ~ISystem() = default;

        ISystem(const ISystem&)            = default;
        ISystem& operator=(const ISystem&) = default;
        ISystem(ISystem&&)                 = default;
        ISystem& operator=(ISystem&&)      = default;

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
        virtual void OnAttach(ecs::Registry& registry) { (void)registry; }

        /// @brief Called when the system is removed from a scene.
        virtual void OnDetach() {}

        // ----------------------------------------------------------------
        // Per-frame updates — only called when IsEnabled() returns true.
        // ----------------------------------------------------------------

        /// @brief Called every frame with variable delta time.
        virtual void Update(f64 deltaTime) { (void)deltaTime; }

        /// @brief Called at a fixed rate (physics, networking, etc.).
        virtual void FixedUpdate(f64 fixedDeltaTime) { (void)fixedDeltaTime; }

        /// @brief Called every frame after Update.
        virtual void LateUpdate(f64 deltaTime) { (void)deltaTime; }

        // ----------------------------------------------------------------
        // Enable / Disable — owned by the base class, not virtual.
        // ----------------------------------------------------------------

        /// @brief Enables the system.  Per-frame updates resume on the
        ///        next frame.
        void Enable() noexcept { m_Enabled = true; }

        /// @brief Disables the system.  Per-frame updates are skipped
        ///        until Enable() is called.
        void Disable() noexcept { m_Enabled = false; }

        /// @brief Returns true when the system is enabled and should
        ///        receive per-frame callbacks.
        [[nodiscard]] bool IsEnabled() const noexcept { return m_Enabled; }

    private:
        bool m_Enabled{true};
    };

} // namespace engine::systems