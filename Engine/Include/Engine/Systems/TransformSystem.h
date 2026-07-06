// ============================================================================
// File: Engine/Include/Engine/Systems/TransformSystem.h
// BFS hierarchy traversal that computes world-space transform matrices.
// ============================================================================
#pragma once

#include "Engine/Systems/ISystem.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"

namespace engine::systems {

    // ========================================================================
    // TransformSystem
    // ========================================================================

    /// @brief Computes world-space transformation matrices for all entities
    ///        in the scene hierarchy.
    ///
    /// Uses a breadth-first traversal of the first-child / next-sibling
    /// tree.  Root entities (Parent == Invalid) have their WorldMatrix set
    /// directly from GetLocalMatrix().  Children inherit their parent's
    /// world matrix: WorldMatrix = ParentWorldMatrix * GetLocalMatrix().
    class TransformSystem final : public ISystem
    {
    public:
        // -- ISystem interface -------------------------------------------------

        [[nodiscard]] std::string_view GetName() const noexcept override
        {
            return "TransformSystem";
        }

        void OnAttach(ecs::Registry& registry) override;

        void Update(f64 deltaTime) override;

    private:
        ecs::Registry* m_registry{nullptr};
    };

} // namespace engine::systems