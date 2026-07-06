// ============================================================================
// File: Engine/Source/Hierarchy/HierarchyManager.cpp
// ============================================================================
#include "Engine/Hierarchy/HierarchyManager.h"
#include "Engine/Hierarchy/HierarchyUtils.h"
#include "Engine/Core/Log.h"

namespace engine::hierarchy {

    using components::HierarchyComponent;
    using components::RelationshipComponent;

    // ========================================================================
    // Queries
    // ========================================================================

    Entity HierarchyManager::GetParent(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return engine::ecs::Invalid;
        return registry.GetComponent<HierarchyComponent>(entity).Parent;
    }

    Entity HierarchyManager::GetFirstChild(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return engine::ecs::Invalid;
        return registry.GetComponent<HierarchyComponent>(entity).FirstChild;
    }

    std::vector<Entity> HierarchyManager::GetChildren(Registry& registry, Entity entity)
    {
        if (registry.HasComponent<RelationshipComponent>(entity))
            return registry.GetComponent<RelationshipComponent>(entity).Children;
        return hierarchy::GetChildren(registry, entity);
    }

    std::vector<Entity> HierarchyManager::GetRoots(Registry& registry)
    {
        return hierarchy::GetRoots(registry);
    }

    u32 HierarchyManager::GetDepth(Registry& registry, Entity entity)
    {
        if (registry.HasComponent<RelationshipComponent>(entity))
            return registry.GetComponent<RelationshipComponent>(entity).Depth;

        u32 depth = 0;
        Entity current = GetParent(registry, entity);
        while (current != engine::ecs::Invalid)
        {
            ++depth;
            current = GetParent(registry, current);
        }
        return depth;
    }

    u32 HierarchyManager::GetDescendantCount(Registry& registry, Entity entity)
    {
        if (registry.HasComponent<RelationshipComponent>(entity))
            return registry.GetComponent<RelationshipComponent>(entity).DescendantCount;

        u32 count = 0;
        auto children = GetChildren(registry, entity);
        for (Entity child : children)
        {
            ++count;
            count += GetDescendantCount(registry, child);
        }
        return count;
    }

    bool HierarchyManager::IsDescendantOf(Registry& registry, Entity entity, Entity ancestor)
    {
        return hierarchy::IsDescendantOf(registry, entity, ancestor);
    }

    bool HierarchyManager::IsRoot(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return true;
        return registry.GetComponent<HierarchyComponent>(entity).Parent == engine::ecs::Invalid;
    }

    bool HierarchyManager::IsLeaf(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return true;
        return registry.GetComponent<HierarchyComponent>(entity).FirstChild == engine::ecs::Invalid;
    }

    // ========================================================================
    // Mutations
    // ========================================================================

    void HierarchyManager::SetParent(Registry& registry, Entity child, Entity parent)
    {
        hierarchy::SetParent(registry, child, parent);
        EnsureComponents(registry, child);
        EnsureComponents(registry, parent);
        UpdateChildrenList(registry, parent);
        RefreshPath(registry, child);
    }

    void HierarchyManager::RemoveFromParent(Registry& registry, Entity entity)
    {
        Entity oldParent = GetParent(registry, entity);
        hierarchy::RemoveFromParent(registry, entity);

        if (registry.HasComponent<RelationshipComponent>(entity))
        {
            auto& rel = registry.GetComponent<RelationshipComponent>(entity);
            rel.Parent = engine::ecs::Invalid;
            rel.IsRoot = true;
            rel.Depth = 0;
        }

        if (oldParent != engine::ecs::Invalid)
            UpdateChildrenList(registry, oldParent);
    }

    void HierarchyManager::RemoveAllChildren(Registry& registry, Entity entity)
    {
        auto children = GetChildren(registry, entity);
        for (Entity child : children)
            RemoveFromParent(registry, child);
    }

    void HierarchyManager::DestroyWithDescendants(Registry& registry, Entity entity)
    {
        auto children = GetChildren(registry, entity);
        for (Entity child : children)
            DestroyWithDescendants(registry, child);

        RemoveFromParent(registry, entity);
        registry.DestroyEntity(entity);
    }

    void HierarchyManager::MoveToFirst(Registry& registry, Entity entity)
    {
        Entity parent = GetParent(registry, entity);
        if (parent == engine::ecs::Invalid)
            return;

        auto& parentHier = registry.GetComponent<HierarchyComponent>(parent);
        if (parentHier.FirstChild == entity)
            return;

        RemoveFromParent(registry, entity);
        EnsureComponents(registry, entity);
        auto& childHier = registry.GetComponent<HierarchyComponent>(entity);

        childHier.Parent = parent;
        childHier.PrevSibling = engine::ecs::Invalid;
        childHier.NextSibling = parentHier.FirstChild;

        if (parentHier.FirstChild != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(parentHier.FirstChild))
        {
            registry.GetComponent<HierarchyComponent>(parentHier.FirstChild).PrevSibling = entity;
        }

        parentHier.FirstChild = entity;
        ++parentHier.ChildCount;
        UpdateChildrenList(registry, parent);
    }

    void HierarchyManager::MoveToLast(Registry& registry, Entity entity)
    {
        Entity parent = GetParent(registry, entity);
        if (parent == engine::ecs::Invalid)
            return;

        RemoveFromParent(registry, entity);
        SetParent(registry, entity, parent);
    }

    void HierarchyManager::MoveBefore(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return;

        auto& hier = registry.GetComponent<HierarchyComponent>(entity);
        Entity prev = hier.PrevSibling;
        if (prev == engine::ecs::Invalid)
            return;

        Entity parent = hier.Parent;
        Entity next = hier.NextSibling;

        hier.PrevSibling = engine::ecs::Invalid;
        hier.NextSibling = engine::ecs::Invalid;

        if (registry.HasComponent<HierarchyComponent>(prev))
            registry.GetComponent<HierarchyComponent>(prev).NextSibling = next;

        if (next != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(next))
        {
            registry.GetComponent<HierarchyComponent>(next).PrevSibling = prev;
        }

        if (parent != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(parent))
        {
            auto& parentHier = registry.GetComponent<HierarchyComponent>(parent);
            if (parentHier.FirstChild == entity)
                parentHier.FirstChild = next;
        }

        Entity prevPrev = engine::ecs::Invalid;
        if (registry.HasComponent<HierarchyComponent>(prev))
            prevPrev = registry.GetComponent<HierarchyComponent>(prev).PrevSibling;

        hier.PrevSibling = prevPrev;
        hier.NextSibling = prev;

        if (registry.HasComponent<HierarchyComponent>(prev))
            registry.GetComponent<HierarchyComponent>(prev).PrevSibling = entity;

        if (prevPrev != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(prevPrev))
        {
            registry.GetComponent<HierarchyComponent>(prevPrev).NextSibling = entity;
        }

        if (parent != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(parent))
        {
            auto& parentHier = registry.GetComponent<HierarchyComponent>(parent);
            if (parentHier.FirstChild == prev)
                parentHier.FirstChild = entity;
        }

        UpdateChildrenList(registry, parent);
    }

    void HierarchyManager::MoveAfter(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            return;

        auto& hier = registry.GetComponent<HierarchyComponent>(entity);
        Entity next = hier.NextSibling;
        if (next == engine::ecs::Invalid)
            return;

        Entity parent = hier.Parent;
        Entity prev = hier.PrevSibling;
        Entity nextNext = engine::ecs::Invalid;
        if (registry.HasComponent<HierarchyComponent>(next))
            nextNext = registry.GetComponent<HierarchyComponent>(next).NextSibling;

        hier.PrevSibling = next;
        hier.NextSibling = nextNext;

        if (registry.HasComponent<HierarchyComponent>(next))
        {
            auto& nextHier = registry.GetComponent<HierarchyComponent>(next);
            nextHier.PrevSibling = prev;
            nextHier.NextSibling = entity;
        }

        if (prev != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(prev))
        {
            registry.GetComponent<HierarchyComponent>(prev).NextSibling = next;
        }

        if (nextNext != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(nextNext))
        {
            registry.GetComponent<HierarchyComponent>(nextNext).PrevSibling = entity;
        }

        if (parent != engine::ecs::Invalid
            && registry.HasComponent<HierarchyComponent>(parent))
        {
            auto& parentHier = registry.GetComponent<HierarchyComponent>(parent);
            if (parentHier.FirstChild == entity)
                parentHier.FirstChild = next;
        }

        UpdateChildrenList(registry, parent);
    }

    // ========================================================================
    // Batch operations
    // ========================================================================

    void HierarchyManager::RefreshAll(Registry& registry)
    {
        auto roots = GetRoots(registry);
        for (Entity root : roots)
        {
            UpdateDepthRecursive(registry, root, 0);
            UpdateDescendantCountRecursive(registry, root);
        }
    }

    void HierarchyManager::RefreshPath(Registry& registry, Entity entity)
    {
        Entity root = entity;
        while (root != engine::ecs::Invalid)
        {
            Entity parent = GetParent(registry, root);
            if (parent == engine::ecs::Invalid)
                break;
            root = parent;
        }

        if (root != engine::ecs::Invalid)
        {
            UpdateDepthRecursive(registry, root, 0);
            UpdateDescendantCountRecursive(registry, root);
        }
    }

    // ========================================================================
    // Internal helpers
    // ========================================================================

    void HierarchyManager::EnsureComponents(Registry& registry, Entity entity)
    {
        if (!registry.HasComponent<HierarchyComponent>(entity))
            registry.AddComponent<HierarchyComponent>(entity);
        if (!registry.HasComponent<RelationshipComponent>(entity))
            registry.AddComponent<RelationshipComponent>(entity);
    }

    void HierarchyManager::UpdateChildrenList(Registry& registry, Entity parent)
    {
        if (parent == engine::ecs::Invalid)
            return;

        EnsureComponents(registry, parent);
        auto& rel = registry.GetComponent<RelationshipComponent>(parent);
        rel.Children.clear();
        rel.Parent = GetParent(registry, parent);
        rel.IsRoot = (rel.Parent == engine::ecs::Invalid);

        Entity child = GetFirstChild(registry, parent);
        while (child != engine::ecs::Invalid)
        {
            rel.Children.push_back(child);
            if (registry.HasComponent<HierarchyComponent>(child))
                child = registry.GetComponent<HierarchyComponent>(child).NextSibling;
            else
                break;
        }
    }

    void HierarchyManager::UpdateDepthRecursive(Registry& registry, Entity entity, u32 depth)
    {
        EnsureComponents(registry, entity);
        auto& rel = registry.GetComponent<RelationshipComponent>(entity);
        rel.Depth = depth;

        auto children = GetChildren(registry, entity);
        for (Entity child : children)
            UpdateDepthRecursive(registry, child, depth + 1);
    }

    u32 HierarchyManager::UpdateDescendantCountRecursive(Registry& registry, Entity entity)
    {
        EnsureComponents(registry, entity);
        auto& rel = registry.GetComponent<RelationshipComponent>(entity);

        u32 count = 0;
        auto children = GetChildren(registry, entity);
        for (Entity child : children)
        {
            ++count;
            count += UpdateDescendantCountRecursive(registry, child);
        }

        rel.DescendantCount = count;
        return count;
    }

} // namespace engine::hierarchy
