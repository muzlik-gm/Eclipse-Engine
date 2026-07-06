// ============================================================================
// File: Engine/Source/Systems/TransformSystem.cpp
// TransformSystem implementation — BFS world-matrix propagation.
// ============================================================================

#include "Engine/Systems/TransformSystem.h"

#include <queue>

namespace engine::systems {

    // ========================================================================
    // OnAttach
    // ========================================================================

    void TransformSystem::OnAttach(ecs::Registry& registry)
    {
        m_registry = &registry;
    }

    // ========================================================================
    // Update — BFS traversal computing world transforms
    // ========================================================================

    void TransformSystem::Update([[maybe_unused]] f64 deltaTime)
    {
        if (!m_registry)
            return;

        auto view = m_registry->View<components::HierarchyComponent, components::TransformComponent>();

        // Step 1: Identify root entities and compute their world matrices.
        std::queue<ecs::Entity> bfsQueue;

        for (auto entityHandle : view)
        {
            ecs::Entity entity{entityHandle};

            auto& hierarchy  = m_registry->GetComponent<components::HierarchyComponent>(entity);
            auto& transform  = m_registry->GetComponent<components::TransformComponent>(entity);

            if (hierarchy.Parent == entt::null)
            {
                // Root entity — world matrix is the local matrix.
                transform.WorldMatrix = transform.GetLocalMatrix();
                transform.WorldDirty  = false;

                bfsQueue.push(entity);
            }
        }

        // Step 2: BFS — propagate parent world matrix to children.
        while (!bfsQueue.empty())
        {
            ecs::Entity parent = bfsQueue.front();
            bfsQueue.pop();

            // We need the parent's WorldMatrix.
            if (!m_registry->HasComponent<components::TransformComponent>(parent))
                continue;

            const auto& parentTransform = m_registry->GetComponent<components::TransformComponent>(parent);
            const math::Mat4& parentWorld = parentTransform.WorldMatrix;

            // Walk the child chain of this parent.
            auto& parentHierarchy = m_registry->GetComponent<components::HierarchyComponent>(parent);
            ecs::Entity child = parentHierarchy.FirstChild;

            while (child != entt::null)
            {
                if (m_registry->HasComponent<components::HierarchyComponent>(child)
                    && m_registry->HasComponent<components::TransformComponent>(child))
                {
                    auto& childHierarchy = m_registry->GetComponent<components::HierarchyComponent>(child);
                    auto& childTransform = m_registry->GetComponent<components::TransformComponent>(child);

                    childTransform.WorldMatrix = parentWorld * childTransform.GetLocalMatrix();
                    childTransform.WorldDirty  = false;

                    // Enqueue child so its own children are processed next.
                    bfsQueue.push(child);
                }

                // Advance to next sibling.
                child = m_registry->GetComponent<components::HierarchyComponent>(child).NextSibling;
            }
        }

        // Step 3: Process standalone entities (TransformComponent, no HierarchyComponent).
        // These are flat, non-hierarchical entities created by the editor or loaded
        // from scenes that don't use the hierarchy system.
        // Always update these — the editor may modify Translation/Rotation/Scale
        // every frame via the Inspector, so we can't rely on WorldDirty alone.
        auto standaloneView = m_registry->View<components::TransformComponent>();
        for (auto entityHandle : standaloneView)
        {
            ecs::Entity entity{entityHandle};

            // Skip entities that also have HierarchyComponent (already handled above).
            if (m_registry->HasComponent<components::HierarchyComponent>(entity))
                continue;

            auto& transform = m_registry->GetComponent<components::TransformComponent>(entity);
            // Always recalculate — the Inspector may have changed Translation/Rotation/Scale.
            transform.WorldMatrix = transform.GetLocalMatrix();
            transform.WorldDirty  = false;
        }
    }

} // namespace engine::systems