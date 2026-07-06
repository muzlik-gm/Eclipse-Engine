// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCommandBuffer.cpp
// ============================================================================
#include "OpenGLCommandBuffer.h"
#include "OpenGLTypes.h"
#include "OpenGLDebugLayer.h"
#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLPipeline.h"
#include "OpenGLShader.h"
#include "OpenGLRenderPass.h"
#include "OpenGLVertexArray.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::usize;

    // Helper: get GL handle from IGraphicsObject via GetNativeHandle().
    static inline GLuint GetGLHandle(IGraphicsObject* obj)
    {
        return obj ? static_cast<GLuint>(obj->GetNativeHandle()) : 0;
    }

    OpenGLCommandBuffer::OpenGLCommandBuffer(OpenGLDebugLayer* debugLayer)
        : m_DebugLayer(debugLayer)
    {
    }

    OpenGLCommandBuffer::~OpenGLCommandBuffer() = default;

    void OpenGLCommandBuffer::Begin()
    {
        m_StateCache.Invalidate();
        m_Recording = true;
        m_Executable = false;
    }

    void OpenGLCommandBuffer::End()
    {
        m_Recording = false;
        m_Executable = true;
    }

    void OpenGLCommandBuffer::Reset()
    {
        m_StateCache.Invalidate();
        m_Recording = false;
        m_Executable = false;
        m_BoundPipeline = nullptr;
        m_BoundVAO = nullptr;
        m_BoundIndexBuffer = nullptr;
        m_BoundFramebuffer = nullptr;
        m_ActiveRenderPass = nullptr;
        m_InRenderPass = false;
    }

    void OpenGLCommandBuffer::BeginFrame() {}
    void OpenGLCommandBuffer::EndFrame() {}
    void OpenGLCommandBuffer::Present() {}

    void OpenGLCommandBuffer::BeginRenderPass(IRenderPass* renderPass, IFramebuffer* framebuffer)
    {
        m_ActiveRenderPass = static_cast<OpenGLRenderPass*>(renderPass);
        m_BoundFramebuffer = static_cast<OpenGLFramebuffer*>(framebuffer);
        m_InRenderPass = true;

        GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        if (m_StateCache.SetFramebuffer(fbo))
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        if (!m_ActiveRenderPass) return;
        const auto& rpDesc = m_ActiveRenderPass->GetDescription();

        for (u32 i = 0; i < rpDesc.ColorAttachments.size(); ++i)
        {
            if (rpDesc.ColorAttachments[i].LoadOp == AttachmentLoadOperation::Clear)
            {
                const auto& cc = rpDesc.ColorAttachments[i].ClearColor;
                glClearNamedFramebufferfv(fbo, GL_COLOR, i, cc.data());
            }
        }

        if (rpDesc.HasDepthStencil
            && rpDesc.DepthStencilAttachment.LoadOp == AttachmentLoadOperation::Clear)
        {
            const auto& att = rpDesc.DepthStencilAttachment;
            const bool isDepth = engine::rhi::IsDepthFormat(att.Format);
            const bool isStencil = engine::rhi::IsStencilFormat(att.Format);

            if (isDepth && isStencil)
                glClearNamedFramebufferfi(fbo, GL_DEPTH_STENCIL, 0, att.ClearDepth, static_cast<GLint>(att.ClearStencil));
            else if (isDepth)
                glClearNamedFramebufferfv(fbo, GL_DEPTH, 0, &att.ClearDepth);
            else if (isStencil)
            {
                GLuint s = att.ClearStencil;
                glClearNamedFramebufferuiv(fbo, GL_STENCIL, 0, &s);
            }
        }
    }

    void OpenGLCommandBuffer::EndRenderPass()
    {
        m_InRenderPass = false;
        m_ActiveRenderPass = nullptr;
        m_BoundFramebuffer = nullptr;
    }

    void OpenGLCommandBuffer::BindPipeline(IGraphicsPipeline* pipeline)
    {
        m_BoundPipeline = static_cast<OpenGLGraphicsPipeline*>(pipeline);
        if (m_BoundPipeline)
            m_BoundPipeline->Apply(m_StateCache);
    }

    void OpenGLCommandBuffer::BindComputePipeline(IComputePipeline*) {}

    void OpenGLCommandBuffer::BindVertexArray(IVertexArray* vao)
    {
        m_BoundVAO = static_cast<OpenGLVertexArray*>(vao);
        if (m_BoundVAO)
        {
            const GLuint handle = m_BoundVAO->GetHandle();
            if (m_StateCache.SetVertexArray(handle))
                glBindVertexArray(handle);
        }
    }

    void OpenGLCommandBuffer::BindVertexBuffer(u32 binding, IVertexBuffer* buffer, usize offset)
    {
        if (!buffer) return;
        if (!m_BoundVAO)
        {
            GLuint defaultVAO = 0;
            if (m_StateCache.SetVertexArray(defaultVAO))
                glBindVertexArray(defaultVAO);
        }
        const GLuint handle = GetGLHandle(buffer);
        const GLsizei stride = static_cast<GLsizei>(buffer->GetStride());
        glBindVertexBuffer(binding, handle, static_cast<GLintptr>(offset), stride);
    }

    void OpenGLCommandBuffer::BindIndexBuffer(IIndexBuffer* buffer, usize offset)
    {
        if (!buffer) return;
        m_BoundIndexBuffer = nullptr;
        m_IndexType = buffer->GetIndexType();
        (void)offset;

        if (!m_BoundVAO)
        {
            GLuint defaultVAO = 0;
            if (m_StateCache.SetVertexArray(defaultVAO))
                glBindVertexArray(defaultVAO);
        }
        const GLuint handle = GetGLHandle(buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    }

    void OpenGLCommandBuffer::BindTexture(u32 slot, ITexture* texture)
    {
        if (!texture) return;
        if (m_StateCache.SetActiveTexture(slot))
            glActiveTexture(GL_TEXTURE0 + slot);
        const GLuint handle = GetGLHandle(texture);
        const TextureType tt = texture->GetType();
        const GLuint target = ToGLTextureTarget(tt, tt == TextureType::TextureCube || tt == TextureType::TextureCubeArray);
        glBindTexture(target, handle);
    }

    void OpenGLCommandBuffer::BindSampler(u32 slot, ISampler* sampler)
    {
        if (!sampler) return;
        const GLuint handle = GetGLHandle(sampler);
        glBindSampler(slot, handle);
    }

    void OpenGLCommandBuffer::BindUniformBuffer(u32 slot, IUniformBuffer* buffer)
    {
        if (!buffer) return;
        const GLuint handle = GetGLHandle(buffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, slot, handle);
    }

    void OpenGLCommandBuffer::BindStorageBuffer(u32 slot, IStorageBuffer* buffer)
    {
        if (!buffer) return;
        const GLuint handle = GetGLHandle(buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, handle);
    }

    void OpenGLCommandBuffer::BindFramebuffer(IFramebuffer* framebuffer)
    {
        m_BoundFramebuffer = static_cast<OpenGLFramebuffer*>(framebuffer);
        const GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        if (m_StateCache.SetFramebuffer(fbo))
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    void OpenGLCommandBuffer::PushConstants(ShaderStage, u32, u32, const void*) {}

    void OpenGLCommandBuffer::SetViewport(const ViewportDescription& viewport)
    {
        if (m_StateCache.SetViewport(static_cast<i32>(viewport.X), static_cast<i32>(viewport.Y),
                                      static_cast<u32>(viewport.Width), static_cast<u32>(viewport.Height)))
        {
            glViewport(static_cast<GLint>(viewport.X), static_cast<GLint>(viewport.Y),
                       static_cast<GLsizei>(viewport.Width), static_cast<GLsizei>(viewport.Height));
        }
        glDepthRangef(viewport.MinDepth, viewport.MaxDepth);
    }

    void OpenGLCommandBuffer::SetScissor(const ScissorDescription& scissor)
    {
        if (m_StateCache.SetScissorTestEnabled(true)) glEnable(GL_SCISSOR_TEST);
        if (m_StateCache.SetScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height))
            glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
    }

    void OpenGLCommandBuffer::SetViewportDynamic(f32 x, f32 y, f32 w, f32 h, f32 minDepth, f32 maxDepth)
    {
        ViewportDescription vp{x, y, w, h, minDepth, maxDepth};
        SetViewport(vp);
    }

    void OpenGLCommandBuffer::SetScissorDynamic(i32 x, i32 y, u32 w, u32 h)
    {
        if (m_StateCache.SetScissorTestEnabled(true)) glEnable(GL_SCISSOR_TEST);
        if (m_StateCache.SetScissor(x, y, w, h))
            glScissor(x, y, w, h);
    }

    void OpenGLCommandBuffer::ClearColor(u32, f32 r, f32 g, f32 b, f32 a)
    {
        GLfloat color[4] = {r, g, b, a};
        GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        glClearNamedFramebufferfv(fbo, GL_COLOR, 0, color);
    }

    void OpenGLCommandBuffer::ClearDepth(f32 depth)
    {
        GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        glClearNamedFramebufferfv(fbo, GL_DEPTH, 0, &depth);
    }

    void OpenGLCommandBuffer::ClearStencil(u32 stencil)
    {
        GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        GLint s = static_cast<GLint>(stencil);
        glClearNamedFramebufferiv(fbo, GL_STENCIL, 0, &s);
    }

    void OpenGLCommandBuffer::ClearDepthStencil(f32 depth, u32 stencil)
    {
        GLuint fbo = m_BoundFramebuffer ? m_BoundFramebuffer->GetHandle() : 0;
        glClearNamedFramebufferfi(fbo, GL_DEPTH_STENCIL, 0, depth, static_cast<GLint>(stencil));
    }

    void OpenGLCommandBuffer::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        glDrawArraysInstancedBaseInstance(
            ToGLPrimitiveTopology(m_BoundPipeline ? m_BoundPipeline->GetTopology() : PrimitiveTopology::TriangleList),
            static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount),
            static_cast<GLsizei>(instanceCount), static_cast<GLuint>(firstInstance));
    }

    void OpenGLCommandBuffer::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
    {
        const GLenum topology = ToGLPrimitiveTopology(m_BoundPipeline ? m_BoundPipeline->GetTopology() : PrimitiveTopology::TriangleList);
        const GLenum indexType = ToGLIndexType(m_IndexType);
        const void* indices = reinterpret_cast<const void*>(static_cast<usize>(firstIndex) * (m_IndexType == IndexType::UInt16 ? 2 : 4));
        glDrawElementsInstancedBaseVertexBaseInstance(topology, static_cast<GLsizei>(indexCount), indexType, indices,
            static_cast<GLsizei>(instanceCount), static_cast<GLint>(vertexOffset), static_cast<GLuint>(firstInstance));
    }

    void OpenGLCommandBuffer::DrawInstanced(u32 vertexCountPerInstance, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        Draw(vertexCountPerInstance, instanceCount, firstVertex, firstInstance);
    }

    void OpenGLCommandBuffer::DrawIndexedIndirect(IStorageBuffer* indirectBuffer, usize offset, u32 drawCount, usize stride)
    {
        if (!indirectBuffer) return;
        const GLuint handle = GetGLHandle(indirectBuffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, handle);
        glMultiDrawElementsIndirect(
            ToGLPrimitiveTopology(m_BoundPipeline ? m_BoundPipeline->GetTopology() : PrimitiveTopology::TriangleList),
            ToGLIndexType(m_IndexType), reinterpret_cast<const void*>(offset),
            static_cast<GLsizei>(drawCount), static_cast<GLsizei>(stride));
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    void OpenGLCommandBuffer::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
    {
        glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    }

    void OpenGLCommandBuffer::CopyBuffer(IBuffer* src, IBuffer* dst, usize srcOffset, usize dstOffset, usize size)
    {
        if (!src || !dst) return;
        const GLuint srcHandle = GetGLHandle(src);
        const GLuint dstHandle = GetGLHandle(dst);
        glCopyNamedBufferSubData(srcHandle, dstHandle, static_cast<GLintptr>(srcOffset),
                                  static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(size));
    }

    void OpenGLCommandBuffer::CopyTexture(ITexture* src, ITexture* dst, u32 srcMip, u32 srcLayer, u32 dstMip, u32 dstLayer)
    {
        if (!src || !dst) return;
        const GLuint srcHandle = GetGLHandle(src);
        const GLuint dstHandle = GetGLHandle(dst);
        const GLuint srcTarget = ToGLTextureTarget(src->GetType());
        const GLuint dstTarget = ToGLTextureTarget(dst->GetType());
        glCopyImageSubData(srcHandle, srcTarget, static_cast<GLint>(srcMip), 0, 0, static_cast<GLint>(srcLayer),
                           dstHandle, dstTarget, static_cast<GLint>(dstMip), 0, 0, static_cast<GLint>(dstLayer),
                           static_cast<GLsizei>(src->GetWidth()), static_cast<GLsizei>(src->GetHeight()), 1);
    }

    void OpenGLCommandBuffer::CopyBufferToTexture(IBuffer* src, ITexture* dst, usize srcOffset, u32 dstMip, u32 dstLayer)
    {
        if (!src || !dst) return;
        const GLuint bufHandle = GetGLHandle(src);
        const GLuint texHandle = GetGLHandle(dst);
        const GLuint texTarget = ToGLTextureTarget(dst->GetType());

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufHandle);
        glBindTexture(texTarget, texHandle);

        const GLuint format = ToGLFormat(dst->GetFormat());
        const GLuint type = ToGLType(dst->GetFormat());

        if (texTarget == GL_TEXTURE_2D)
            glTexSubImage2D(GL_TEXTURE_2D, dstMip, 0, 0, dst->GetWidth(), dst->GetHeight(), format, type, reinterpret_cast<const void*>(srcOffset));
        else if (texTarget == GL_TEXTURE_2D_ARRAY || texTarget == GL_TEXTURE_3D)
            glTexSubImage3D(texTarget, dstMip, 0, 0, dstLayer, dst->GetWidth(), dst->GetHeight(), 1, format, type, reinterpret_cast<const void*>(srcOffset));

        glBindTexture(texTarget, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    void OpenGLCommandBuffer::CopyTextureToBuffer(ITexture* src, IBuffer* dst, u32 srcMip, u32 srcLayer, usize dstOffset)
    {
        if (!src || !dst) return;
        (void)srcMip; (void)srcLayer; (void)dstOffset;
        // Note: glGetTexImage is complex to use with PBOs — simplified.
    }

    void OpenGLCommandBuffer::GenerateMipmaps(ITexture* texture)
    {
        if (!texture) return;
        const GLuint handle = GetGLHandle(texture);
        const GLuint target = ToGLTextureTarget(texture->GetType());
        glBindTexture(target, handle);
        glGenerateMipmap(target);
        glBindTexture(target, 0);
    }

    void OpenGLCommandBuffer::TransitionResource(IGraphicsObject*, ResourceState, ResourceState) {}

    void OpenGLCommandBuffer::Flush() { glFlush(); }
    void OpenGLCommandBuffer::Finish() { glFinish(); }

    void OpenGLCommandBuffer::BeginDebugGroup(std::string_view name)
    {
        if (m_DebugLayer) m_DebugLayer->PushDebugGroup(name);
    }

    void OpenGLCommandBuffer::EndDebugGroup()
    {
        if (m_DebugLayer) m_DebugLayer->PopDebugGroup();
    }

    void OpenGLCommandBuffer::InsertDebugMarker(std::string_view name)
    {
        if (m_DebugLayer) m_DebugLayer->InsertMarker(name);
    }

} // namespace engine::opengl
