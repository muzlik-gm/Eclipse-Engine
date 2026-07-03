// ============================================================================
// File: Engine/Include/Engine/Runtime/ISubsystem.h
// Pure virtual interface that every engine subsystem must implement.
// ============================================================================

#pragma once

#include "Engine/Core/Timing.h"
#include "Engine/Core/Types.h"

#include <string>
#include <string_view>
#include <vector>

namespace engine::runtime
{

    using engine::core::f64;

    // ========================================================================
    // ISubsystem
    // ========================================================================

    /// Pure virtual interface for all engine subsystems.
    ///
    /// Every subsystem in the engine (rendering, physics, audio, etc.) must
    /// implement this interface.  The SubsystemManager calls these methods
    /// in a well-defined order during the engine lifecycle.
    ///
    /// Subsystems are registered with the SubsystemManager before engine
    /// initialization.  During init they are initialized in dependency
    /// order; during shutdown the order is reversed.
    ///
    /// Only empty lifecycle execution is acceptable in Phase 1 — the
    /// interface must be complete, but subsystem implementations may be
    /// stubs until their respective phases.
    class ISubsystem
    {
    public:
        virtual ~ISubsystem() = default;

        // ----------------------------------------------------------------
        // Identity
        // ----------------------------------------------------------------

        /// Returns the unique name of this subsystem (e.g. "Renderer",
        /// "Physics", "Audio").
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        // ----------------------------------------------------------------
        // Dependencies
        // ----------------------------------------------------------------

        /// Returns the names of subsystems that must be initialized before
        /// this one.  The SubsystemManager uses this to compute a valid
        /// initialization order.  Returns an empty vector by default.
        [[nodiscard]] virtual std::vector<std::string> GetDependencies() const
        {
            return {};
        }

        // ----------------------------------------------------------------
        // Lifecycle
        // ----------------------------------------------------------------

        /// Called once during engine initialization, after all dependencies
        /// have been initialized.  Return true on success.
        virtual bool Initialize() = 0;

        /// Called once during engine shutdown, before dependencies are
        /// shut down.
        virtual void Shutdown() = 0;

        // ----------------------------------------------------------------
        // Per-frame updates
        // ----------------------------------------------------------------

        /// Called every frame with variable delta time (in seconds).
        /// Skipped when the engine is paused.
        virtual void Update(f64 deltaTime) = 0;

        /// Called at a fixed rate determined by the fixed timestep setting.
        /// Always runs even when paused (for physics stability).
        virtual void FixedUpdate(f64 fixedDeltaTime) = 0;

        /// Called every frame after Update, with the same delta time.
        /// Skipped when the engine is paused.
        virtual void LateUpdate(f64 deltaTime) = 0;
    };

} // namespace engine::runtime