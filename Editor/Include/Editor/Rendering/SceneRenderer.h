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

        /// @brief Returns the number of draw calls in the last render.
        [[nodiscard]] engine::core::u32 GetDrawCallCount() const noexcept
        { return m_DrawCalls; }

    private:
        void EnsureShaders();
        void EnsureGridGeometry();
        void EnsureCubeGeometry();
        void RenderGrid(const engine::math::Mat4& viewProjection);
        void RenderMeshes(EditorContext& context, const engine::math::Mat4& viewProjection);

        engine::core::u32 m_DrawCalls{0};

        // Shader program.
        GLuint m_ShaderProgram{0};
        GLuint m_GridShaderProgram{0};
        bool   m_ShadersCompiled{false};

        // Grid geometry.
        GLuint m_GridVAO{0};
        GLuint m_GridVBO{0};
        int    m_GridLineCount{0};

        // Cube geometry (for default mesh rendering).
        GLuint m_CubeVAO{0};
        GLuint m_CubeVBO{0};
        GLuint m_CubeIBO{0};

        // Uniform locations (cached).
        GLint  m_uViewProj{-1};
        GLint  m_uModel{-1};
        GLint  m_uColor{-1};
        GLint  m_uViewProjGrid{-1};
        GLint  m_uGridColor{-1};

        u32    m_LastMeshCount{0xFFFFFFFF}; // Track for change detection.
    };

} // namespace editor
