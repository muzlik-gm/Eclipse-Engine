// ============================================================================
// File: Editor/Include/Editor/Rendering/SceneRenderer.h
// Renders the active scene's entities into a viewport framebuffer.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Editor/Viewport/ViewportFramebuffer.h"

namespace editor {

    class EditorContext;

    /// @brief Renders the active scene into a viewport framebuffer.
    ///        Iterates entities with MeshComponent + TransformComponent,
    ///        applies the camera's view-projection, and draws them.
    class SceneRenderer
    {
    public:
        SceneRenderer();
        ~SceneRenderer();

        /// @brief Renders the scene into @p framebuffer using @p viewProjection.
        /// @param context         The editor context.
        /// @param framebuffer     Target framebuffer.
        /// @param viewProjection  Camera view-projection matrix.
        /// @param renderGrid      If true, renders the editor grid.
        void RenderScene(EditorContext& context, ViewportFramebuffer& framebuffer,
                         const engine::math::Mat4& viewProjection, bool renderGrid = true);

        /// @brief Renders the runtime game view into @p framebuffer.
        /// @param context         The editor context.
        /// @param framebuffer     Target framebuffer.
        void RenderGameView(EditorContext& context, ViewportFramebuffer& framebuffer);

        /// @brief Returns the number of draw calls in the last render.
        [[nodiscard]] engine::core::u32 GetDrawCallCount() const noexcept
        { return m_DrawCalls; }

    private:
        engine::core::u32 m_DrawCalls{0};

        // Procedural validation shader sources.
        const char* m_VertexShaderSrc;
        const char* m_FragmentShaderSrc;
    };

} // namespace editor
