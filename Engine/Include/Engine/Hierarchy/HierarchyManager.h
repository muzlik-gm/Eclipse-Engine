// ============================================================================
// File: Engine/Include/Engine/Hierarchy/HierarchyManager.h
// Stateful manager for entity parent-child hierarchies.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Components/RelationshipComponent.h"

#include <vector>

namespace engine::hierarchy {

    using engine::core::u32;
    using engine::ecs::Entity;
    using engine::ecs::Registry;

    // ========================================================================
    // HierarchyManager
    // ========================================================================

    /// @brief Stateful manager that maintains both HierarchyComponent
    ///        (first-child / next-sibling linkage) and RelationshipComponent
    ///        (ordered children list, depth, descendant count) in sync.
    ///
    /// All hierarchy mutations should go through this manager rather than
    /// calling the free functions in HierarchyUtils directly, so that
    /// RelationshipComponent stays consistent.
    class HierarchyManager
    {
    public:
        HierarchyManager() = default;
        ~HierarchyManager() = default;

        HierarchyManager(const HierarchyManager&)            = delete;
        HierarchyManager& operator=(const HierarchyManager&) = delete;
        HierarchyManager(HierarchyManager&&)                 = delete;
        HierarchyManager& operator=(HierarchyManager&&)      = delete;

        // ----------------------------------------------------------------
        // Structure queries
        // ----------------------------------------------------------------

        /// @brief Returns the parent of @p entity, or Invalid if root.
        [[nodiscard]] static Entity GetParent(Registry& registry, Entity entity);

        /// @brief Returns the first child of @p entity, or Invalid if leaf.
        [[nodiscard]] static Entity GetFirstChild(Registry& registry, Entity entity);

        /// @brief Returns all direct children of @p entity.
        [[nodiscard]] static std::vector<Entity> GetChildren(Registry& registry, Entity entity);

        /// @brief Returns all root entities (Parent == Invalid).
        [[nodiscard]] static std::vector<Entity> GetRoots(Registry& registry);

        /// @brief Returns the depth of @p entity in the hierarchy.
        ///        Roots have depth 0.
        [[nodiscard]] static u32 GetDepth(Registry& registry, Entity entity);

        /// @brief Returns the total number of descendants of @p entity.
        [[nodiscard]] static u32 GetDescendantCount(Registry& registry, Entity entity);

        /// @brief Returns true if @p entity is a descendant of @p ancestor.
        [[nodiscard]] static bool IsDescendantOf(Registry& registry,
                                                  Entity entity, Entity ancestor);

        /// @brief Returns true if @p entity is a root (no parent).
        [[nodiscard]] static bool IsRoot(Registry& registry, Entity entity);

        /// @brief Returns true if @p entity is a leaf (no children).
        [[nodiscard]] static bool IsLeaf(Registry& registry, Entity entity);

        // ----------------------------------------------------------------
        // Structure mutations
        // ----------------------------------------------------------------

        /// @brief Makes @p child a direct child of @p parent.
        ///        If @p child already has a parent it is unlinked first.
        static void SetParent(Registry& registry, Entity child, Entity parent);

        /// @brief Unlinks @p entity from its parent.  The entity becomes
        ///        a root.
        static void RemoveFromParent(Registry& registry, Entity entity);

        /// @brief Removes all children from @p entity, making them roots.
        static void RemoveAllChildren(Registry& registry, Entity entity);

        /// @brief Destroys @p entity and all its descendants recursively.
        static void DestroyWithDescendants(Registry& registry, Entity entity);

        /// @brief Moves @p entity to be the first child of its parent.
        static void MoveToFirst(Registry& registry, Entity entity);

        /// @brief Moves @p entity to be the last child of its parent.
        static void MoveToLast(Registry& registry, Entity entity);

        /// @brief Moves @p entity one position before its previous sibling.
        static void MoveBefore(Registry& registry, Entity entity);

        /// @brief Moves @p entity one position after its next sibling.
        static void MoveAfter(Registry& registry, Entity entity);

        // ----------------------------------------------------------------
        // Batch operations
        // ----------------------------------------------------------------

        /// @brief Recomputes RelationshipComponent metadata (Depth,
        ///        DescendantCount, Children list) for all entities that
        ///        have a HierarchyComponent.
        static void RefreshAll(Registry& registry);

        /// @brief Recomputes RelationshipComponent metadata for @p entity
        ///        and all its ancestors.
        static void RefreshPath(Registry& registry, Entity entity);

    private:
        // -- Internal helpers ----------------------------------------------

        static void EnsureComponents(Registry& registry, Entity entity);
        static void UpdateChildrenList(Registry& registry, Entity parent);
        static void UpdateDepthRecursive(Registry& registry, Entity entity, u32 depth);
        static u32  UpdateDescendantCountRecursive(Registry& registry, Entity entity);
    };

} // namespace engine::hierarchy
