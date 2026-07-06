// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLRenderPass.cpp
// ============================================================================
#include "OpenGLRenderPass.h"
#include "OpenGLTexture.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;

    // ========================================================================
    // OpenGLRenderPass
    // ========================================================================

    OpenGLRenderPass::OpenGLRenderPass(const RenderPassDescription& desc,
                                         OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
    {
        (void)debugLayer;
        ENGINE_LOG_DEBUG("OpenGLRenderPass — created '{}' ({} color attachments, depth={})",
                         m_Name, desc.ColorAttachments.size(),
                         desc.HasDepthStencil ? "yes" : "no");
    }

    // ========================================================================
    // OpenGLFramebuffer
    // ========================================================================

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferDescription& desc,
                                           OpenGLRenderPass* renderPass,
                                           OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_RenderPass(renderPass)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        glGenFramebuffers(1, &m_Handle);
        if (m_Handle == 0)
        {
            ENGINE_LOG_ERROR("OpenGLFramebuffer — glGenFramebuffers failed");
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);

        // Attach color textures.
        for (u32 i = 0; i < desc.ColorAttachments.size(); ++i)
        {
            auto* texture = static_cast<OpenGLTexture*>(const_cast<void*>(desc.ColorAttachments[i]));
            if (texture)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                        GL_COLOR_ATTACHMENT0 + i,
                                        GL_TEXTURE_2D,
                                        texture->GetHandle(),
                                        0);
            }
        }

        // Attach depth/stencil texture.
        if (desc.DepthStencilAttachment)
        {
            auto* dsTexture = static_cast<OpenGLTexture*>(
                const_cast<void*>(desc.DepthStencilAttachment));
            if (dsTexture)
            {
                const GraphicsFormat dsFormat = dsTexture->GetFormat();
                const GLenum attach = (dsFormat == GraphicsFormat::D32_Float
                                     || dsFormat == GraphicsFormat::D16_UNorm)
                    ? GL_DEPTH_ATTACHMENT
                    : (dsFormat == GraphicsFormat::S8_UInt)
                        ? GL_STENCIL_ATTACHMENT
                        : GL_DEPTH_STENCIL_ATTACHMENT;

                glFramebufferTexture2D(GL_FRAMEBUFFER, attach,
                                        GL_TEXTURE_2D, dsTexture->GetHandle(), 0);
            }
        }

        // Check completeness.
        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            ENGINE_LOG_ERROR("OpenGLFramebuffer — incomplete (status=0x{:X})", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (m_DebugLayer && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_FRAMEBUFFER, m_Handle, m_Name);
        }

        ENGINE_LOG_DEBUG("OpenGLFramebuffer — created '{}' (handle={}, {}x{})",
                         m_Name, m_Handle, desc.Width, desc.Height);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        if (m_Handle != 0)
        {
            glDeleteFramebuffers(1, &m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLFramebuffer::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_FRAMEBUFFER, m_Handle, m_Name);
    }

    ITexture* OpenGLFramebuffer::GetColorAttachment(engine::core::u32 index) const noexcept
    {
        if (index >= m_Description.ColorAttachments.size())
            return nullptr;
        return static_cast<ITexture*>(const_cast<void*>(m_Description.ColorAttachments[index]));
    }

    ITexture* OpenGLFramebuffer::GetDepthStencilAttachment() const noexcept
    {
        return static_cast<ITexture*>(const_cast<void*>(m_Description.DepthStencilAttachment));
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
    }

} // namespace engine::opengl
