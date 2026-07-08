// ============================================================================
// File: Editor/Include/Editor/Rendering/SceneRenderer.h
// Renders the active scene's entities into a viewport framebuffer.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Editor/Viewport/ViewportFramebuffer.h"

#include <glad/glad.h>

namespace editor {

    using engine::core::u32;

    class EditorContext;

/// @brief Renders the active scene into a viewport framebuffer.
///        Uses direct OpenGL calls for the editor viewport.
class SceneRenderer
{
public:
    SceneRenderer();
    ~SceneRenderer();

    /// @brief Renders the scene into @p framebuffer using @p viewProjection.
    void RenderScene(EditorContext& context, ViewportFramebuffer& framebuffer,
                     const engine::math::Mat4& viewProjection, bool renderGrid = true);

    /// @brief Renders the runtime game view into @p framebuffer.
    void RenderGameView(EditorContext& context, ViewportFramebuffer& framebuffer);

    /// @brief Renders a selection wireframe outline around the selected entity.
    void RenderSelectionOutline(EditorContext& context,
                                const engine::math::Mat4& viewProjection);

    /// @brief Renders the transform gizmo (translation/rotation/scale)
    ///        for the selected entity.
    void RenderGizmos(EditorContext& context,
                      const engine::math::Mat4& viewProjection);

    /// @brief Renders all entities with pick IDs into the currently bound FBO.
    void RenderPicking(EditorContext& context,
                       const engine::math::Mat4& viewProjection);

    /// @brief Returns the number of draw calls in the last render.
    [[nodiscard]] engine::core::u32 GetDrawCallCount() const noexcept
    { return m_DrawCalls; }

private:
    void EnsureShaders();
    void EnsureGridGeometry();
    void EnsureCubeGeometry();
    void EnsureWireframeGeometry();
    void EnsureGizmoGeometry();
    void RenderGrid(const engine::math::Mat4& viewProjection,
                    const engine::math::Vec3& cameraPos,
                    engine::core::f32 viewportW, engine::core::f32 viewportH);
    void RenderMeshes(EditorContext& context, const engine::math::Mat4& viewProjection);

    engine::core::u32 m_DrawCalls{0};

    // Shader programs.
    GLuint m_ShaderProgram{0};
    GLuint m_GridShaderProgram{0};
    GLuint m_LineProgram{0};
    GLuint m_PickProgram{0};
    bool   m_ShadersCompiled{false};

    // Grid geometry.
    GLuint m_GridVAO{0};
    GLuint m_GridVBO{0};
    int    m_GridLineCount{0};

    // Cube geometry (for default mesh rendering).
    GLuint m_CubeVAO{0};
    GLuint m_CubeVBO{0};
    GLuint m_CubeIBO{0};

    // Wireframe geometry (for selection outline).
    GLuint m_WireframeVAO{0};
    GLuint m_WireframeVBO{0};

    // Gizmo geometry.
    GLuint m_GizmoVAO{0};
    GLuint m_GizmoVBO{0};
    GLuint m_CircleVAO{0};
    GLuint m_CircleVBO{0};
    int    m_CircleVertexCount{0};

    // Mesh shader uniform locations.
    GLint  m_uViewProj{-1};
    GLint  m_uModel{-1};
    GLint  m_uColor{-1};

    // Grid shader uniform locations.
    GLint  m_uViewProjGrid{-1};
    GLint  m_uGridColor{-1};
    GLint  m_uGridCamPos{-1};
    GLint  m_uGridInvVP{-1};
    GLint  m_uGridViewportSize{-1};

    // Line shader uniform locations.
    GLint  m_uLineViewProj{-1};
    GLint  m_uLineModel{-1};

    // Pick shader uniform locations.
    GLint  m_uPickViewProj{-1};
    GLint  m_uPickModel{-1};
    GLint  m_uPickEntityColor{-1};

    u32    m_LastMeshCount{0xFFFFFFFF}; // Track for change detection.
};

} // namespace editor
