// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLStateCache.cpp
// ============================================================================
#include "OpenGLStateCache.h"

namespace engine::opengl {

    OpenGLStateCache::OpenGLStateCache()
    {
        Invalidate();
    }

    void OpenGLStateCache::Invalidate()
    {
        m_Program = 0;
        m_VAO = 0;
        m_DrawFramebuffer = 0;
        m_ReadFramebuffer = 0;
        m_ActiveTexture = 0xFFFFFFFF;

        m_Buffers.fill(0xFFFFFFFF);
        for (auto& unit : m_Textures)
        {
            unit.Target = 0;
            unit.Texture = 0xFFFFFFFF;
        }
        m_Samplers.fill(0xFFFFFFFF);

        m_ViewportSet = false;
        m_ScissorSet = false;
        m_ScissorTestEnabled = false;
        m_DepthTestEnabled = false;
        m_DepthWriteEnabled = true;
        m_StencilTestEnabled = false;
        m_BlendEnabled = false;
        m_CullFaceEnabled = false;
        m_DepthClampEnabled = false;
    }

    // ========================================================================
    // Bound objects
    // ========================================================================

    bool OpenGLStateCache::SetProgram(u32 program)
    {
        if (m_Program == program)
            return false;
        m_Program = program;
        return true;
    }

    bool OpenGLStateCache::SetVertexArray(u32 vao)
    {
        if (m_VAO == vao)
            return false;
        m_VAO = vao;
        return true;
    }

    bool OpenGLStateCache::SetFramebuffer(u32 framebuffer, bool isRead)
    {
        if (isRead)
        {
            if (m_ReadFramebuffer == framebuffer)
                return false;
            m_ReadFramebuffer = framebuffer;
        }
        else
        {
            if (m_DrawFramebuffer == framebuffer)
                return false;
            m_DrawFramebuffer = framebuffer;
        }
        return true;
    }

    // ========================================================================
    // Buffers
    // ========================================================================

    bool OpenGLStateCache::SetBuffer(u32 target, u32 buffer)
    {
        // Map target to index.  We support the common targets.
        u32 index = 0;
        switch (target)
        {
            case GL_ARRAY_BUFFER:              index = 0; break;
            case GL_ELEMENT_ARRAY_BUFFER:      index = 1; break;
            case GL_UNIFORM_BUFFER:            index = 2; break;
            case GL_SHADER_STORAGE_BUFFER:     index = 3; break;
            case GL_TEXTURE_BUFFER:            index = 4; break;
            case GL_TRANSFORM_FEEDBACK_BUFFER: index = 5; break;
            case GL_COPY_READ_BUFFER:          index = 6; break;
            case GL_COPY_WRITE_BUFFER:         index = 7; break;
            case GL_PIXEL_PACK_BUFFER:         index = 8; break;
            case GL_PIXEL_UNPACK_BUFFER:       index = 9; break;
            case GL_DRAW_INDIRECT_BUFFER:      index = 10; break;
            case GL_DISPATCH_INDIRECT_BUFFER:  index = 11; break;
            case GL_QUERY_BUFFER:              index = 12; break;
            case GL_ATOMIC_COUNTER_BUFFER:     index = 13; break;
            default:                           index = 14; break;
        }

        if (index >= kMaxBufferTargets)
            return true; // Unknown target — always issue the call.

        if (m_Buffers[index] == buffer)
            return false;
        m_Buffers[index] = buffer;
        return true;
    }

    // ========================================================================
    // Textures
    // ========================================================================

    bool OpenGLStateCache::SetTexture(u32 slot, u32 target, u32 texture)
    {
        if (slot >= kMaxTextureUnits)
            return true;

        if (m_Textures[slot].Target == target && m_Textures[slot].Texture == texture)
            return false;

        m_Textures[slot].Target = target;
        m_Textures[slot].Texture = texture;
        return true;
    }

    bool OpenGLStateCache::SetSampler(u32 slot, u32 sampler)
    {
        if (slot >= kMaxTextureUnits)
            return true;

        if (m_Samplers[slot] == sampler)
            return false;

        m_Samplers[slot] = sampler;
        return true;
    }

    // ========================================================================
    // Viewport / scissor
    // ========================================================================

    bool OpenGLStateCache::SetViewport(i32 x, i32 y, u32 w, u32 h)
    {
        if (m_ViewportSet
            && m_Viewport.X == x && m_Viewport.Y == y
            && m_Viewport.W == w && m_Viewport.H == h)
            return false;

        m_Viewport = {x, y, w, h};
        m_ViewportSet = true;
        return true;
    }

    bool OpenGLStateCache::SetScissor(i32 x, i32 y, u32 w, u32 h)
    {
        if (m_ScissorSet
            && m_Scissor.X == x && m_Scissor.Y == y
            && m_Scissor.W == w && m_Scissor.H == h)
            return false;

        m_Scissor = {x, y, w, h};
        m_ScissorSet = true;
        return true;
    }

    // ========================================================================
    // Enable / disable
    // ========================================================================

    bool OpenGLStateCache::SetScissorTestEnabled(bool enabled)
    {
        if (m_ScissorTestEnabled == enabled) return false;
        m_ScissorTestEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetDepthTestEnabled(bool enabled)
    {
        if (m_DepthTestEnabled == enabled) return false;
        m_DepthTestEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetDepthWriteEnabled(bool enabled)
    {
        if (m_DepthWriteEnabled == enabled) return false;
        m_DepthWriteEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetStencilTestEnabled(bool enabled)
    {
        if (m_StencilTestEnabled == enabled) return false;
        m_StencilTestEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetBlendEnabled(bool enabled)
    {
        if (m_BlendEnabled == enabled) return false;
        m_BlendEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetCullFaceEnabled(bool enabled)
    {
        if (m_CullFaceEnabled == enabled) return false;
        m_CullFaceEnabled = enabled;
        return true;
    }

    bool OpenGLStateCache::SetDepthClampEnabled(bool enabled)
    {
        if (m_DepthClampEnabled == enabled) return false;
        m_DepthClampEnabled = enabled;
        return true;
    }

    // ========================================================================
    // Rasterizer
    // ========================================================================

    bool OpenGLStateCache::SetCullMode(u32 mode)
    {
        if (m_CullMode == mode) return false;
        m_CullMode = mode;
        return true;
    }

    bool OpenGLStateCache::SetFrontFace(u32 face)
    {
        if (m_FrontFace == face) return false;
        m_FrontFace = face;
        return true;
    }

    bool OpenGLStateCache::SetPolygonMode(u32 mode)
    {
        if (m_PolygonMode == mode) return false;
        m_PolygonMode = mode;
        return true;
    }

    // ========================================================================
    // Depth
    // ========================================================================

    bool OpenGLStateCache::SetDepthFunc(u32 func)
    {
        if (m_DepthFunc == func) return false;
        m_DepthFunc = func;
        return true;
    }

    bool OpenGLStateCache::SetDepthMask(bool enabled)
    {
        return SetDepthWriteEnabled(enabled);
    }

    // ========================================================================
    // Stencil
    // ========================================================================

    bool OpenGLStateCache::SetStencilFunc(u32 face, u32 func, i32 ref, u32 mask)
    {
        if (face == GL_FRONT)
        {
            if (m_StencilFrontFunc.Func == func
                && m_StencilFrontFunc.Ref == ref
                && m_StencilFrontFunc.Mask == mask)
                return false;
            m_StencilFrontFunc = {func, ref, mask};
            return true;
        }
        else if (face == GL_BACK)
        {
            if (m_StencilBackFunc.Func == func
                && m_StencilBackFunc.Ref == ref
                && m_StencilBackFunc.Mask == mask)
                return false;
            m_StencilBackFunc = {func, ref, mask};
            return true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            bool changed = false;
            if (!(m_StencilFrontFunc.Func == func
                  && m_StencilFrontFunc.Ref == ref
                  && m_StencilFrontFunc.Mask == mask))
            {
                m_StencilFrontFunc = {func, ref, mask};
                changed = true;
            }
            if (!(m_StencilBackFunc.Func == func
                  && m_StencilBackFunc.Ref == ref
                  && m_StencilBackFunc.Mask == mask))
            {
                m_StencilBackFunc = {func, ref, mask};
                changed = true;
            }
            return changed;
        }
        return false;
    }

    bool OpenGLStateCache::SetStencilOp(u32 face, u32 sfail, u32 dpfail, u32 dppass)
    {
        if (face == GL_FRONT)
        {
            if (m_StencilFrontOp.SFail == sfail
                && m_StencilFrontOp.DpFail == dpfail
                && m_StencilFrontOp.DpPass == dppass)
                return false;
            m_StencilFrontOp = {sfail, dpfail, dppass};
            return true;
        }
        else if (face == GL_BACK)
        {
            if (m_StencilBackOp.SFail == sfail
                && m_StencilBackOp.DpFail == dpfail
                && m_StencilBackOp.DpPass == dppass)
                return false;
            m_StencilBackOp = {sfail, dpfail, dppass};
            return true;
        }
        return true;
    }

    bool OpenGLStateCache::SetStencilMask(u32 face, u32 mask)
    {
        if (face == GL_FRONT)
        {
            if (m_StencilFrontMask == mask) return false;
            m_StencilFrontMask = mask;
            return true;
        }
        else if (face == GL_BACK)
        {
            if (m_StencilBackMask == mask) return false;
            m_StencilBackMask = mask;
            return true;
        }
        return true;
    }

    // ========================================================================
    // Blend
    // ========================================================================

    bool OpenGLStateCache::SetBlendFunc(u32 srcRGB, u32 dstRGB, u32 srcA, u32 dstA)
    {
        if (m_BlendSrcRGB == srcRGB && m_BlendDstRGB == dstRGB
            && m_BlendSrcA == srcA && m_BlendDstA == dstA)
            return false;
        m_BlendSrcRGB = srcRGB;
        m_BlendDstRGB = dstRGB;
        m_BlendSrcA = srcA;
        m_BlendDstA = dstA;
        return true;
    }

    bool OpenGLStateCache::SetBlendEquation(u32 modeRGB, u32 modeA)
    {
        if (m_BlendEqRGB == modeRGB && m_BlendEqA == modeA)
            return false;
        m_BlendEqRGB = modeRGB;
        m_BlendEqA = modeA;
        return true;
    }

    bool OpenGLStateCache::SetBlendColor(f32 r, f32 g, f32 b, f32 a)
    {
        if (m_BlendColor[0] == r && m_BlendColor[1] == g
            && m_BlendColor[2] == b && m_BlendColor[3] == a)
            return false;
        m_BlendColor[0] = r;
        m_BlendColor[1] = g;
        m_BlendColor[2] = b;
        m_BlendColor[3] = a;
        return true;
    }

    // ========================================================================
    // Primitive restart
    // ========================================================================

    bool OpenGLStateCache::SetPrimitiveRestartIndex(u32 index)
    {
        if (m_PrimitiveRestartIndex == index) return false;
        m_PrimitiveRestartIndex = index;
        return true;
    }

    // ========================================================================
    // Active texture
    // ========================================================================

    bool OpenGLStateCache::SetActiveTexture(u32 slot)
    {
        if (m_ActiveTexture == slot) return false;
        m_ActiveTexture = slot;
        return true;
    }

} // namespace engine::opengl
