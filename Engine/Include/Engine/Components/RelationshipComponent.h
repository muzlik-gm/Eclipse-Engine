// ============================================================================
// File: Engine/Include/Engine/Components/RelationshipComponent.h
// Logical relationship metadata for entities — distinct from the
// first-child/next-sibling linkage stored in HierarchyComponent.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

#include <vector>

namespace engine::components {

    using engine::core::u32;
    using engine::ecs::Entity;

    // ========================================================================
    // RelationshipComponent
    // ========================================================================

    /// @brief Stores logical relationship metadata for an entity.
    ///
    /// While HierarchyComponent stores the first-child / next-sibling /
    /// prev-sibling linkage used for transform propagation, this component
    /// stores higher-level relationship information: the ordered list of
    /// children, optional sibling grouping tags, and a "root" flag that
    /// marks entities that should be treated as top-level scene objects
    /// by the editor and serialization system.
    struct RelationshipComponent
    {
        /// Ordered list of direct children.  Kept in sync with the
        /// HierarchyComponent's first-child / next-sibling chain by the
        /// HierarchyManager.
        std::vector<Entity> Children;

        /// Optional parent of this entity.  Mirrors
        /// HierarchyComponent::Parent but is stored here for quick
        /// access without requiring a HierarchyComponent lookup.
        Entity Parent{};

        /// Total number of descendants (recursive child count).
        /// Updated by HierarchyManager on structure changes.
        u32 DescendantCount{0};

        /// True if this entity is a root-level scene object (no parent).
        bool IsRoot{true};

        /// Depth in the hierarchy (0 = root, 1 = child of root, etc.).
        u32 Depth{0};

        /// Optional sibling group tag — entities with the same tag and
        /// the same parent are treated as a logical group by the editor.
        u32 GroupTag{0};

        /// Marks the entity as a prefab instance root.  Prefab instances
        /// have special semantics during destruction and serialization.
        bool IsPrefabInstance{false};
    };

} // namespace engine::components
