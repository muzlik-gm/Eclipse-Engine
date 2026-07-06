// ============================================================================
// File: Editor/Source/Viewport/ViewportFramebuffer.cpp
// ============================================================================
#include "Editor/Viewport/ViewportFramebuffer.h"
#include "Engine/Core/Log.h"

namespace editor {

    ViewportFramebuffer::ViewportFramebuffer() = default;

    ViewportFramebuffer::~ViewportFramebuffer()
    {
        Destroy();
    }

    void ViewportFramebuffer::Resize(engine::core::u32 width, engine::core::u32 height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_Width == width && m_Height == height && m_Valid)
            return;

        m_Width = width;
        m_Height = height;

        Destroy();
        Create();
    }

    void ViewportFramebuffer::Create()
    {
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // Color attachment.
        glGenTextures(1, &m_ColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, m_ColorTexture, 0);

        // Depth + stencil attachment.
        glGenTextures(1, &m_DepthStencil);
        glBindTexture(GL_TEXTURE_2D, m_DepthStencil);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0,
                     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_TEXTURE_2D, m_DepthStencil, 0);

        // Check completeness.
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        m_Valid = (status == GL_FRAMEBUFFER_COMPLETE);
        if (!m_Valid)
        {
            ENGINE_LOG_ERROR("ViewportFramebuffer — incomplete (status=0x{:X})", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        ENGINE_LOG_DEBUG("ViewportFramebuffer — created {}x{} (valid={})", m_Width, m_Height, m_Valid);
    }

    void ViewportFramebuffer::Destroy()
    {
        m_Valid = false;
        if (m_DepthStencil)
        {
            glDeleteTextures(1, &m_DepthStencil);
            m_DepthStencil = 0;
        }
        if (m_ColorTexture)
        {
            glDeleteTextures(1, &m_ColorTexture);
            m_ColorTexture = 0;
        }
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
    }

    void ViewportFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, static_cast<GLsizei>(m_Width), static_cast<GLsizei>(m_Height));
    }

    void ViewportFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ViewportFramebuffer::Clear(engine::core::f32 r, engine::core::f32 g,
                                     engine::core::f32 b, engine::core::f32 a)
    {
        Bind();
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

} // namespace editor
