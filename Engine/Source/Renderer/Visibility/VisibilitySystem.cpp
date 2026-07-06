// ============================================================================
// File: Engine/Source/Renderer/Visibility/VisibilitySystem.cpp
// ============================================================================
#include "Engine/Renderer/Visibility/VisibilitySystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Core/Log.h"

namespace engine::renderer {

    using engine::components::MeshComponent;
    using engine::components::TransformComponent;
    using engine::components::VisibilityComponent;
    using engine::math::Vec4;

    // ========================================================================
    // Frustum
    // ========================================================================

    void Frustum::ExtractFromMatrix(const Mat4& m)
    {
        // Extract the 6 frustum planes from the view-projection matrix.
        // Each row of the combined matrix gives a plane equation.
        // Left, Right, Bottom, Top, Near, Far.

        const float* v = &m[0][0];

        // Left:   row4 + row1
        FullPlanes[0] = Vec4(v[3]+v[0], v[7]+v[4], v[11]+v[8], v[15]+v[12]);
        // Right:  row4 - row1
        FullPlanes[1] = Vec4(v[3]-v[0], v[7]-v[4], v[11]-v[8], v[15]-v[12]);
        // Bottom: row4 + row2
        FullPlanes[2] = Vec4(v[3]+v[1], v[7]+v[5], v[11]+v[9], v[15]+v[13]);
        // Top:    row4 - row2
        FullPlanes[3] = Vec4(v[3]-v[1], v[7]-v[5], v[11]-v[9], v[15]-v[13]);
        // Near:   row4 + row3
        FullPlanes[4] = Vec4(v[3]+v[2], v[7]+v[6], v[11]+v[10], v[15]+v[14]);
        // Far:    row4 - row3
        FullPlanes[5] = Vec4(v[3]-v[2], v[7]-v[6], v[11]-v[10], v[15]-v[14]);

        // Normalize planes.
        for (int i = 0; i < 6; ++i)
        {
            float len = std::sqrt(
                FullPlanes[i].x * FullPlanes[i].x +
                FullPlanes[i].y * FullPlanes[i].y +
                FullPlanes[i].z * FullPlanes[i].z);
            if (len > 0.0001f)
            {
                FullPlanes[i] /= len;
            }
        }
    }

    bool Frustum::IsSphereVisible(const Vec3& center, float radius) const
    {
        for (int i = 0; i < 6; ++i)
        {
            float dist =
                FullPlanes[i].x * center.x +
                FullPlanes[i].y * center.y +
                FullPlanes[i].z * center.z +
                FullPlanes[i].w;
            if (dist < -radius)
                return false;
        }
        return true;
    }

    // ========================================================================
    // VisibilitySystem
    // ========================================================================

    void VisibilitySystem::CullAndSubmit(const Mat4& viewProjection,
                                          RenderQueueManager& queueManager)
    {
        m_Frustum.ExtractFromMatrix(viewProjection);
        m_VisibleCount = 0;
        m_CulledCount = 0;

        // This method needs access to the active scene.
        // In the current architecture, the scene is accessed via the
        // editor context.  For the engine-level VisibilitySystem, we
        // pass it through a different mechanism.
        //
        // The actual culling + submission is done by the RenderPipeline
        // which has access to both the scene and the visibility system.
        // This method is called from there.
    }

} // namespace engine::renderer
