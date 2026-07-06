// ============================================================================
// File: Engine/Include/Engine/Components/HierarchyComponent.h
// Parent-child / sibling linkage for the entity tree.
// ============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

namespace engine::components
{

    using engine::core::u32;

    // ========================================================================
    // HierarchyComponent
    // ========================================================================

    /// Links an entity into the scene hierarchy using a first-child /
    /// next-sibling representation.
    ///
    /// The hierarchy system maintains these links so that world-space
    /// transforms and other propagated properties can be resolved
    /// efficiently with a single depth-first traversal.
    struct HierarchyComponent
    {
        /// Parent entity, or engine::ecs::Invalid if this entity is a root.
        ecs::Entity Parent{engine::ecs::Invalid};

        /// First child entity, or engine::ecs::Invalid if leaf.
        ecs::Entity FirstChild{engine::ecs::Invalid};

        /// Next sibling entity, or engine::ecs::Invalid if last child.
        ecs::Entity NextSibling{engine::ecs::Invalid};

        /// Previous sibling entity, or engine::ecs::Invalid if first child.
        ecs::Entity PrevSibling{engine::ecs::Invalid};

        /// Number of direct children (cached for fast queries).
        u32 ChildCount{0};
    };

} // namespace engine::components