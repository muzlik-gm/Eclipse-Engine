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
        /// Parent entity, or entt::null if this entity is a root.
        ecs::Entity Parent{entt::null};

        /// First child entity, or entt::null if leaf.
        ecs::Entity FirstChild{entt::null};

        /// Next sibling entity, or entt::null if last child.
        ecs::Entity NextSibling{entt::null};

        /// Previous sibling entity, or entt::null if first child.
        ecs::Entity PrevSibling{entt::null};

        /// Number of direct children (cached for fast queries).
        u32 ChildCount{0};
    };

} // namespace engine::components