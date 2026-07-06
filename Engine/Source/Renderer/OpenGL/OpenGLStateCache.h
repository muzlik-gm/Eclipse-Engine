// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLStateCache.h
// Caches OpenGL state to minimize redundant driver calls.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <glad/glad.h>

#include <array>
#include <cstdint>

namespace engine::opengl {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;

    // ========================================================================
    // OpenGLStateCache
    // ========================================================================

    /// @brief Tracks the current OpenGL state so the backend can skip
    ///        redundant gl* calls.  Every Set* method returns true if the
    ///        state actually changed (and the GL call should be issued)
    ///        or false if the state was already correct.
    class OpenGLStateCache
    {
    public:
        OpenGLStateCache();
        ~OpenGLStateCache() = default;

        /// @brief Resets all cached state to "unknown" — forces the next
        ///        Set* call to issue the GL call.
        void Invalidate();

        // -- Bound objects --------------------------------------------------

        /// @brief Returns true if the program needs to be bound.
        bool SetProgram(u32 program);

        /// @brief Returns true if the VAO needs to be bound.
        bool SetVertexArray(u32 vao);

        /// @brief Returns true if the framebuffer needs to be bound.
        /// @param framebuffer  Framebuffer handle, 0 = default framebuffer.
        /// @param isRead       True for read framebuffer, false for draw.
        bool SetFramebuffer(u32 framebuffer, bool isRead = false);

        // -- Buffers --------------------------------------------------------

        /// @brief Returns true if the buffer needs to be bound.
        bool SetBuffer(u32 target, u32 buffer);

        // -- Textures -------------------------------------------------------

        /// @brief Returns true if the texture needs to be bound.
        bool SetTexture(u32 slot, u32 target, u32 texture);

        /// @brief Returns true if the sampler needs to be bound.
        bool SetSampler(u32 slot, u32 sampler);

        // -- Viewport / scissor --------------------------------------------

        bool SetViewport(i32 x, i32 y, u32 w, u32 h);
        bool SetScissor(i32 x, i32 y, u32 w, u32 h);

        bool SetScissorTestEnabled(bool enabled);
        bool SetDepthTestEnabled(bool enabled);
        bool SetDepthWriteEnabled(bool enabled);
        bool SetStencilTestEnabled(bool enabled);
        bool SetBlendEnabled(bool enabled);
        bool SetCullFaceEnabled(bool enabled);
        bool SetDepthClampEnabled(bool enabled);

        bool SetCullMode(u32 mode);
        bool SetFrontFace(u32 face);
        bool SetPolygonMode(u32 mode);

        bool SetDepthFunc(u32 func);
        bool SetDepthMask(bool enabled);

        bool SetStencilFunc(u32 face, u32 func, i32 ref, u32 mask);
        bool SetStencilOp(u32 face, u32 sfail, u32 dpfail, u32 dppass);
        bool SetStencilMask(u32 face, u32 mask);

        bool SetBlendFunc(u32 srcRGB, u32 dstRGB, u32 srcA, u32 dstA);
        bool SetBlendEquation(u32 modeRGB, u32 modeA);
        bool SetBlendColor(f32 r, f32 g, f32 b, f32 a);

        bool SetPrimitiveRestartIndex(u32 index);

        // -- Active texture -------------------------------------------------

        bool SetActiveTexture(u32 slot);

        // -- Query ----------------------------------------------------------

        [[nodiscard]] u32 GetBoundProgram() const noexcept { return m_Program; }
        [[nodiscard]] u32 GetBoundVertexArray() const noexcept { return m_VAO; }
        [[nodiscard]] u32 GetBoundDrawFramebuffer() const noexcept { return m_DrawFramebuffer; }

    private:
        // Bound objects
        u32 m_Program{0};
        u32 m_VAO{0};
        u32 m_DrawFramebuffer{0};
        u32 m_ReadFramebuffer{0};
        u32 m_ActiveTexture{0};

        // Buffer bindings (indexed by target)
        static constexpr u32 kMaxBufferTargets = 16;
        std::array<u32, kMaxBufferTargets> m_Buffers{};

        // Texture units (up to 80 typical max)
        static constexpr u32 kMaxTextureUnits = 32;
        struct TextureUnit
        {
            u32 Target{0};
            u32 Texture{0};
        };
        std::array<TextureUnit, kMaxTextureUnits> m_Textures{};
        std::array<u32, kMaxTextureUnits> m_Samplers{};

        // Viewport / scissor
        struct ViewportState
        {
            i32 X, Y;
            u32 W, H;
        } m_Viewport{0, 0, 0, 0};

        struct ScissorState
        {
            i32 X, Y;
            u32 W, H;
        } m_Scissor{0, 0, 0, 0};

        // Enabled / disabled state
        bool m_ScissorTestEnabled{false};
        bool m_DepthTestEnabled{false};
        bool m_DepthWriteEnabled{true};
        bool m_StencilTestEnabled{false};
        bool m_BlendEnabled{false};
        bool m_CullFaceEnabled{false};
        bool m_DepthClampEnabled{false};

        // Rasterizer
        u32 m_CullMode{GL_BACK};
        u32 m_FrontFace{GL_CCW};
        u32 m_PolygonMode{GL_FILL};

        // Depth
        u32 m_DepthFunc{GL_LESS};

        // Stencil
        struct StencilFuncState
        {
            u32 Func{GL_ALWAYS};
            i32 Ref{0};
            u32 Mask{0xFFFFFFFF};
        } m_StencilFrontFunc{}, m_StencilBackFunc{};

        struct StencilOpState
        {
            u32 SFail{GL_KEEP};
            u32 DpFail{GL_KEEP};
            u32 DpPass{GL_KEEP};
        } m_StencilFrontOp{}, m_StencilBackOp{};

        u32 m_StencilFrontMask{0xFFFFFFFF};
        u32 m_StencilBackMask{0xFFFFFFFF};

        // Blend
        u32 m_BlendSrcRGB{GL_ONE};
        u32 m_BlendDstRGB{GL_ZERO};
        u32 m_BlendSrcA{GL_ONE};
        u32 m_BlendDstA{GL_ZERO};
        u32 m_BlendEqRGB{GL_FUNC_ADD};
        u32 m_BlendEqA{GL_FUNC_ADD};
        f32 m_BlendColor[4]{0.0f, 0.0f, 0.0f, 0.0f};

        // Primitive restart
        u32 m_PrimitiveRestartIndex{0};

        // Track which states have been set at least once.
        bool m_ViewportSet{false};
        bool m_ScissorSet{false};
    };

} // namespace engine::opengl
