// ============================================================================
// File: Engine/Include/Engine/Components/StaticComponent.h
// Marks an entity as non-moving for rendering / physics optimisations.
// ============================================================================

#pragma once

namespace engine::components
{

    // ========================================================================
    // StaticComponent
    // ========================================================================

    /// When true, signals that the entity's transform will not change
    /// during gameplay.  Systems that cache derived data (broadphase
    /// structures, static batching, light maps, etc.) can use this flag
    /// to skip per-frame recomputation.
    struct StaticComponent
    {
        /// True if the entity is considered immovable / immutable.
        bool IsStatic{false};
    };

} // namespace engine::components