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
        if (child == engine::ecs::Invalid || parent == engine::ecs::Invalid)
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
        if (childHier.Parent != engine::ecs::Invalid
            && registry.HasComponent<components::HierarchyComponent>(childHier.Parent))
        {
            RemoveFromParent(registry, child);
        }

        // Link into new parent's child list (append to end).
        childHier.Parent = parent;

        if (parentHier.FirstChild == engine::ecs::Invalid)
        {
            // Parent has no children yet.
            parentHier.FirstChild = child;
            childHier.PrevSibling = engine::ecs::Invalid;
        }
        else
        {
            // Walk to the last child.
            ecs::Entity sibling = parentHier.FirstChild;
            while (registry.GetComponent<components::HierarchyComponent>(sibling).NextSibling
                   != engine::ecs::Invalid)
            {
                sibling = registry.GetComponent<components::HierarchyComponent>(sibling).NextSibling;
            }

            auto& lastSiblingHier = registry.GetComponent<components::HierarchyComponent>(sibling);
            lastSiblingHier.NextSibling = child;
            childHier.PrevSibling = sibling;
        }

        childHier.NextSibling = engine::ecs::Invalid;
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
        if (entity == engine::ecs::Invalid)
            return;

        if (!registry.HasComponent<components::HierarchyComponent>(entity))
            return;

        auto& hier = registry.GetComponent<components::HierarchyComponent>(entity);

        if (hier.Parent == engine::ecs::Invalid)
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
        if (hier.PrevSibling != engine::ecs::Invalid
            && registry.HasComponent<components::HierarchyComponent>(hier.PrevSibling))
        {
            registry.GetComponent<components::HierarchyComponent>(hier.PrevSibling).NextSibling = hier.NextSibling;
        }

        if (hier.NextSibling != engine::ecs::Invalid
            && registry.HasComponent<components::HierarchyComponent>(hier.NextSibling))
        {
            registry.GetComponent<components::HierarchyComponent>(hier.NextSibling).PrevSibling = hier.PrevSibling;
        }

        --parentHier.ChildCount;

        // Reset entity's links.
        hier.Parent      = engine::ecs::Invalid;
        hier.PrevSibling = engine::ecs::Invalid;
        hier.NextSibling = engine::ecs::Invalid;
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

        if (parent == engine::ecs::Invalid)
            return children;

        if (!registry.HasComponent<components::HierarchyComponent>(parent))
            return children;

        const auto& hier = registry.GetComponent<components::HierarchyComponent>(parent);
        ecs::Entity current = hier.FirstChild;

        while (current != engine::ecs::Invalid)
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

            if (hier.Parent == engine::ecs::Invalid)
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
        if (entity == engine::ecs::Invalid || ancestor == engine::ecs::Invalid)
            return false;

        if (entity == ancestor)
            return false; // An entity is not its own descendant.

        ecs::Entity current = entity;

        while (current != engine::ecs::Invalid)
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