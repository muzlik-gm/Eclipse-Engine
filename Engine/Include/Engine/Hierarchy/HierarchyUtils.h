// ============================================================================
// File: Engine/Include/Engine/Hierarchy/HierarchyUtils.h
// Free functions for manipulating the entity parent-child hierarchy.
// ============================================================================
#pragma once

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/HierarchyComponent.h"

#include <vector>

namespace engine::hierarchy {

    // ========================================================================
    // Forward declaration.
    inline void RemoveFromParent(ecs::Registry& registry, ecs::Entity entity);

    // SetParent
    // ========================================================================

    /// @brief Makes @p child a direct child of @p parent.
    ///
    /// If @p child already has a parent it is unlinked first.
    /// A HierarchyComponent is added to @p child if it does not already
    /// have one.
    inline void SetParent(ecs::Registry& registry, ecs::Entity child, ecs::Entity parent)
    {
        if (child == entt::null || parent == entt::null)
            return;

        // Ensure the child has a HierarchyComponent.
        if (!registry.HasComponent<components::HierarchyComponent>(child))
        {
            registry.AddComponent<components::HierarchyComponent>(child);
        }

        // Ensure the parent has a HierarchyComponent.
        if (!registry.HasComponent<components::HierarchyComponent>(parent))
        {
            registry.AddComponent<components::HierarchyComponent>(parent);
        }

        auto& childHier = registry.GetComponent<components::HierarchyComponent>(child);
        auto& parentHier = registry.GetComponent<components::HierarchyComponent>(parent);

        // Unlink from old parent if any.
        if (childHier.Parent != entt::null
            && registry.HasComponent<components::HierarchyComponent>(childHier.Parent))
        {
            RemoveFromParent(registry, child);
        }

        // Link into new parent's child list (append to end).
        childHier.Parent = parent;

        if (parentHier.FirstChild == entt::null)
        {
            // Parent has no children yet.
            parentHier.FirstChild = child;
            childHier.PrevSibling = entt::null;
        }
        else
        {
            // Walk to the last child.
            ecs::Entity sibling = parentHier.FirstChild;
            while (registry.GetComponent<components::HierarchyComponent>(sibling).NextSibling
                   != entt::null)
            {
                sibling = registry.GetComponent<components::HierarchyComponent>(sibling).NextSibling;
            }

            auto& lastSiblingHier = registry.GetComponent<components::HierarchyComponent>(sibling);
            lastSiblingHier.NextSibling = child;
            childHier.PrevSibling = sibling;
        }

        childHier.NextSibling = entt::null;
        ++parentHier.ChildCount;
    }

    // ========================================================================
    // RemoveFromParent
    // ========================================================================

    /// @brief Unlinks @p entity from its parent's child list.
    ///
    /// After this call the entity becomes a root (Parent == Invalid).
    inline void RemoveFromParent(ecs::Registry& registry, ecs::Entity entity)
    {
        if (entity == entt::null)
            return;

        if (!registry.HasComponent<components::HierarchyComponent>(entity))
            return;

        auto& hier = registry.GetComponent<components::HierarchyComponent>(entity);

        if (hier.Parent == entt::null)
            return; // Already a root.

        if (!registry.HasComponent<components::HierarchyComponent>(hier.Parent))
            return;

        auto& parentHier = registry.GetComponent<components::HierarchyComponent>(hier.Parent);

        // Update parent's FirstChild pointer if necessary.
        if (parentHier.FirstChild == entity)
        {
            parentHier.FirstChild = hier.NextSibling;
        }

        // Update sibling pointers.
        if (hier.PrevSibling != entt::null
            && registry.HasComponent<components::HierarchyComponent>(hier.PrevSibling))
        {
            registry.GetComponent<components::HierarchyComponent>(hier.PrevSibling).NextSibling = hier.NextSibling;
        }

        if (hier.NextSibling != entt::null
            && registry.HasComponent<components::HierarchyComponent>(hier.NextSibling))
        {
            registry.GetComponent<components::HierarchyComponent>(hier.NextSibling).PrevSibling = hier.PrevSibling;
        }

        --parentHier.ChildCount;

        // Reset entity's links.
        hier.Parent      = entt::null;
        hier.PrevSibling = entt::null;
        hier.NextSibling = entt::null;
    }

    // ========================================================================
    // GetChildren
    // ========================================================================

    /// @brief Returns all direct children of @p parent by walking the
    ///        FirstChild / NextSibling chain.
    [[nodiscard]] inline std::vector<ecs::Entity> GetChildren(
        ecs::Registry& registry, ecs::Entity parent)
    {
        std::vector<ecs::Entity> children;

        if (parent == entt::null)
            return children;

        if (!registry.HasComponent<components::HierarchyComponent>(parent))
            return children;

        const auto& hier = registry.GetComponent<components::HierarchyComponent>(parent);
        ecs::Entity current = hier.FirstChild;

        while (current != entt::null)
        {
            children.push_back(current);

            if (!registry.HasComponent<components::HierarchyComponent>(current))
                break;

            current = registry.GetComponent<components::HierarchyComponent>(current).NextSibling;
        }

        return children;
    }

    // ========================================================================
    // GetRoots
    // ========================================================================

    /// @brief Returns all entities that are hierarchy roots
    ///        (Parent == Invalid).
    [[nodiscard]] inline std::vector<ecs::Entity> GetRoots(ecs::Registry& registry)
    {
        std::vector<ecs::Entity> roots;

        auto view = registry.View<components::HierarchyComponent>();
        for (auto entityHandle : view)
        {
            ecs::Entity entity{entityHandle};
            const auto& hier = registry.GetComponent<components::HierarchyComponent>(entity);

            if (hier.Parent == entt::null)
            {
                roots.push_back(entity);
            }
        }

        return roots;
    }

    // ========================================================================
    // IsDescendantOf
    // ========================================================================

    /// @brief Returns true if @p entity is a descendant of @p ancestor
    ///        (walks up the parent chain).
    [[nodiscard]] inline bool IsDescendantOf(
        ecs::Registry& registry, ecs::Entity entity, ecs::Entity ancestor)
    {
        if (entity == entt::null || ancestor == entt::null)
            return false;

        if (entity == ancestor)
            return false; // An entity is not its own descendant.

        ecs::Entity current = entity;

        while (current != entt::null)
        {
            if (!registry.HasComponent<components::HierarchyComponent>(current))
                break;

            const auto& hier = registry.GetComponent<components::HierarchyComponent>(current);

            if (hier.Parent == ancestor)
                return true;

            current = hier.Parent;
        }

        return false;
    }

} // namespace engine::hierarchy