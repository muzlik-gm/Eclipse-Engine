// ============================================================================
// File: Editor/Include/Editor/Rendering/DebugRenderer.h
// Renders debug primitives: grid, axes, bounding boxes, selection outlines.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/ECS/Entity.h"

namespace editor {

    class EditorContext;

    /// @brief Renders debug primitives in the scene view.  All primitives
    ///        are drawn immediately through OpenGL and are not persistent.
    class DebugRenderer
    {
    public:
        DebugRenderer() = default;
        ~DebugRenderer() = default;

        /// @brief Renders a grid on the XZ plane.
        /// @param center     Center of the grid.
        /// @param size       Total size of the grid (world units).
        /// @param divisions  Number of grid cells per side.
        /// @param color      Grid line color.
        void RenderGrid(const engine::math::Vec3& center, float size,
                        int divisions, const engine::math::Vec4& color);

        /// @brief Renders the world origin axes (X=red, Y=green, Z=blue).
        /// @param axisLength Length of each axis.
        void RenderOriginAxes(float axisLength = 1.0f);

        /// @brief Renders a wireframe bounding box.
        void RenderBoundingBox(const engine::math::Vec3& min,
                               const engine::math::Vec3& max,
                               const engine::math::Vec4& color);

        /// @brief Renders a selection outline for @p entity.
        void RenderSelectionOutline(EditorContext& context, engine::ecs::Entity entity);

        /// @brief Renders a camera frustum.
        void RenderCameraFrustum(const engine::math::Mat4& viewProjection,
                                  const engine::math::Vec4& color);

        /// @brief Renders a line between two points.
        void RenderLine(const engine::math::Vec3& p0, const engine::math::Vec3& p1,
                        const engine::math::Vec4& color);

        /// @brief Sets the view-projection matrix for rendering.
        void SetViewProjection(const engine::math::Mat4& vp) { m_ViewProjection = vp; }

    private:
        engine::math::Mat4 m_ViewProjection{1.0f};
    };

} // namespace editor
