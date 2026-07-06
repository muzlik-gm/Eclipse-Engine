// ============================================================================
// File: Engine/Include/Engine/Renderer/Visibility/VisibilitySystem.h
// Frustum culling and render layer management.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Renderer/Queue/RenderQueue.h"

namespace engine::renderer {

    using engine::core::u32;
    using engine::math::Mat4;
    using engine::math::Vec3;
    using engine::ecs::Entity;

    // ========================================================================
    // Frustum — camera frustum planes for culling.
    // ========================================================================

    /// @brief 6-plane frustum extracted from a view-projection matrix.
    /// Used for frustum culling of entities before rendering.
    struct Frustum
    {
        Vec3 Planes[6]{}; // [a, b, c, d] stored as Vec3.xyz + implicit d = 0
        // Each plane: ax + by + cz + d = 0
        // Stored as Vec4(a, b, c, d) for simplicity.
        math::Vec4 FullPlanes[6]{};

        /// @brief Extracts the 6 frustum planes from a view-projection matrix.
        void ExtractFromMatrix(const Mat4& viewProjection);

        /// @brief Returns true if a sphere at @p center with @p radius
        ///        is at least partially inside the frustum.
        [[nodiscard]] bool IsSphereVisible(const Vec3& center, float radius) const;
    };

    // ========================================================================
    // VisibilitySystem — collects visible entities and submits to queues.
    // ========================================================================

    /// @brief The visibility system iterates the scene's entities,
    ///        performs frustum culling, and submits visible entities
    ///        to the appropriate render queues.
    class VisibilitySystem
    {
    public:
        VisibilitySystem() = default;
        ~VisibilitySystem() = default;

        /// @brief Culls and submits visible entities to @p queueManager.
        /// @param viewProjection  Camera view-projection matrix.
        /// @param queueManager    Target render queues.
        void CullAndSubmit(const Mat4& viewProjection,
                           RenderQueueManager& queueManager);

        /// @brief Returns the number of visible entities in the last cull.
        [[nodiscard]] u32 GetVisibleCount() const noexcept { return m_VisibleCount; }

        /// @brief Returns the number of culled entities in the last cull.
        [[nodiscard]] u32 GetCulledCount() const noexcept { return m_CulledCount; }

    private:
        friend class RenderPipeline;
        Frustum m_Frustum;
        u32     m_VisibleCount{0};
        u32     m_CulledCount{0};
    };

} // namespace engine::renderer
