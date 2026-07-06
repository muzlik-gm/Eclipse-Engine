// ============================================================================
// File: Editor/Include/Editor/Viewport/ViewportFramebuffer.h
// Manages a framebuffer for editor viewports (Scene View, Game View).
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

#include <glad/glad.h>
#include <memory>

namespace editor {

    /// @brief A framebuffer + color attachment + depth-stencil attachment
    ///        sized for an editor viewport.  Resizes automatically when
    ///        the viewport dimensions change.
    class ViewportFramebuffer
    {
    public:
        ViewportFramebuffer();
        ~ViewportFramebuffer();

        ViewportFramebuffer(const ViewportFramebuffer&)            = delete;
        ViewportFramebuffer& operator=(const ViewportFramebuffer&) = delete;
        ViewportFramebuffer(ViewportFramebuffer&&)                 = delete;
        ViewportFramebuffer& operator=(ViewportFramebuffer&&)      = delete;

        /// @brief Creates (or recreates) the framebuffer at the given size.
        /// @param width  Viewport width in pixels.
        /// @param height Viewport height in pixels.
        void Resize(engine::core::u32 width, engine::core::u32 height);

        /// @brief Binds the framebuffer for rendering.
        void Bind();

        /// @brief Unbinds the framebuffer (binds the default framebuffer).
        void Unbind();

        /// @brief Clears the framebuffer to the given color.
        void Clear(engine::core::f32 r, engine::core::f32 g,
                   engine::core::f32 b, engine::core::f32 a);

        /// @brief Returns the OpenGL texture ID of the color attachment.
        ///        Used by ImGui to render the viewport image.
        [[nodiscard]] GLuint GetColorTextureID() const noexcept { return m_ColorTexture; }

        /// @brief Returns the framebuffer width.
        [[nodiscard]] engine::core::u32 GetWidth() const noexcept { return m_Width; }

        /// @brief Returns the framebuffer height.
        [[nodiscard]] engine::core::u32 GetHeight() const noexcept { return m_Height; }

        /// @brief Returns true if the framebuffer is valid, complete, and ready.
        [[nodiscard]] bool IsValid() const noexcept { return m_Valid; }

        /// @brief Returns true if the framebuffer needs to be resized
        ///        to match @p width x @p height.
        [[nodiscard]] bool NeedsResize(engine::core::u32 width,
                                        engine::core::u32 height) const noexcept
        { return m_Width != width || m_Height != height; }

    private:
        void Create();
        void Destroy();

        GLuint                   m_FBO{0};
        GLuint                   m_ColorTexture{0};
        GLuint                   m_DepthStencil{0};
        engine::core::u32        m_Width{0};
        engine::core::u32        m_Height{0};
        bool                     m_Valid{false};
    };

} // namespace editor
