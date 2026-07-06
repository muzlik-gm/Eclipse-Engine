// ============================================================================
// File: Engine/Include/Engine/Components/EnabledComponent.h
// Master on/off toggle for an entity and its systems.
// ============================================================================

#pragma once

namespace engine::components
{

    // ========================================================================
    // EnabledComponent
    // ========================================================================

    /// When false, the entity is excluded from most system processing:
    /// script updates, physics simulation, audio playback, etc.
    ///
    /// Disabling an entity does not automatically hide it; pair with
    /// VisibilityComponent for full control.
    struct EnabledComponent
    {
        /// True if the entity is active in all systems.
        bool IsEnabled{true};
    };

} // namespace engine::components