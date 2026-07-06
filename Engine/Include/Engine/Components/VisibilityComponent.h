// ============================================================================
// File: Engine/Include/Engine/Components/VisibilityComponent.h
// Controls whether an entity participates in rendering.
// ============================================================================

#pragma once

namespace engine::components
{

    // ========================================================================
    // VisibilityComponent
    // ========================================================================

    /// When false, the entity (and optionally its subtree) is skipped by
    /// the rendering pipeline.  Visibility is independent of the enabled
    /// state — an entity can be enabled but invisible (e.g. a collider
    /// that should not be drawn).
    struct VisibilityComponent
    {
        /// True if the entity should be rendered.
        bool IsVisible{true};
    };

} // namespace engine::components