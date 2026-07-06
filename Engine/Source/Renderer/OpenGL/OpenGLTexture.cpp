// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLTexture.cpp
// ============================================================================
#include "OpenGLTexture.h"
#include "OpenGLTypes.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    // OpenGLTexture — base
    // ========================================================================

    OpenGLTexture::OpenGLTexture(const TextureDescription& desc, OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        m_Target = ToGLTextureTarget(desc.Type, desc.IsCubemap());

        glGenTextures(1, &m_Handle);
        if (m_Handle == 0)
        {
            ENGINE_LOG_ERROR("OpenGLTexture — glGenTextures failed");
            return;
        }

        glBindTexture(m_Target, m_Handle);

        const GLuint internalFormat = ToGLInternalFormat(desc.Format);

        // Allocate immutable storage (glTexStorage) for all mips/layers.
        switch (m_Target)
        {
            case GL_TEXTURE_2D:
                glTexStorage2D(GL_TEXTURE_2D, desc.MipLevels, internalFormat,
                               desc.Width, desc.Height);
                break;

            case GL_TEXTURE_2D_ARRAY:
                glTexStorage3D(GL_TEXTURE_2D_ARRAY, desc.MipLevels, internalFormat,
                               desc.Width, desc.Height, desc.ArrayLayers);
                break;

            case GL_TEXTURE_3D:
                glTexStorage3D(GL_TEXTURE_3D, desc.MipLevels, internalFormat,
                               desc.Width, desc.Height, desc.Depth);
                break;

            case GL_TEXTURE_CUBE_MAP:
                glTexStorage2D(GL_TEXTURE_CUBE_MAP, desc.MipLevels, internalFormat,
                               desc.Width, desc.Height);
                break;

            case GL_TEXTURE_CUBE_MAP_ARRAY:
                glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, desc.MipLevels, internalFormat,
                               desc.Width, desc.Height, desc.ArrayLayers);
                break;

            case GL_TEXTURE_1D:
                glTexStorage1D(GL_TEXTURE_1D, desc.MipLevels, internalFormat, desc.Width);
                break;

            case GL_TEXTURE_1D_ARRAY:
                glTexStorage2D(GL_TEXTURE_1D_ARRAY, desc.MipLevels, internalFormat,
                               desc.Width, desc.ArrayLayers);
                break;
        }

        // Set default parameters.
        glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(m_Target, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glBindTexture(m_Target, 0);

        if (m_DebugLayer && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_TEXTURE, m_Handle, m_Name);
        }

        ENGINE_LOG_DEBUG("OpenGLTexture — created '{}' (handle={}, {}x{}, mips={}, layers={})",
                         m_Name, m_Handle, desc.Width, desc.Height,
                         desc.MipLevels, desc.ArrayLayers);
    }

    OpenGLTexture::~OpenGLTexture()
    {
        if (m_Handle != 0)
        {
            glDeleteTextures(1, &m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLTexture::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_TEXTURE, m_Handle, m_Name);
    }

    void OpenGLTexture::UploadData(u32 mipLevel, u32 arrayLayer,
                                    const TextureData& data,
                                    u32 xOffset, u32 yOffset,
                                    u32 width, u32 height)
    {
        if (m_Handle == 0 || !data.Pixels)
            return;

        const u32 w = (width == 0) ? data.Width : width;
        const u32 h = (height == 0) ? data.Height : height;
        const GLuint format = ToGLFormat(m_Description.Format);
        const GLuint type    = ToGLType(m_Description.Format);

        glBindTexture(m_Target, m_Handle);

        switch (m_Target)
        {
            case GL_TEXTURE_2D:
                glTexSubImage2D(GL_TEXTURE_2D, mipLevel, xOffset, yOffset,
                                w, h, format, type, data.Pixels);
                break;

            case GL_TEXTURE_2D_ARRAY:
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset,
                                arrayLayer, w, h, 1, format, type, data.Pixels);
                break;

            case GL_TEXTURE_CUBE_MAP:
                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + arrayLayer,
                                mipLevel, xOffset, yOffset, w, h, format, type, data.Pixels);
                break;

            case GL_TEXTURE_CUBE_MAP_ARRAY:
                glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevel, xOffset, yOffset,
                                arrayLayer, w, h, 1, format, type, data.Pixels);
                break;

            case GL_TEXTURE_3D:
                glTexSubImage3D(GL_TEXTURE_3D, mipLevel, xOffset, yOffset,
                                arrayLayer, w, h, 1, format, type, data.Pixels);
                break;

            case GL_TEXTURE_1D:
                glTexSubImage1D(GL_TEXTURE_1D, mipLevel, xOffset, w, format, type, data.Pixels);
                break;
        }

        glBindTexture(m_Target, 0);
    }

    void OpenGLTexture::GenerateMipmaps()
    {
        if (m_Handle == 0)
            return;
        glBindTexture(m_Target, m_Handle);
        glGenerateMipmap(m_Target);
        glBindTexture(m_Target, 0);
    }

    void OpenGLTexture::TransitionTo(ResourceState newState)
    {
        // OpenGL uses implicit transitions — no explicit barrier needed.
        m_CurrentState = newState;
    }

    ResourceState OpenGLTexture::GetCurrentState() const noexcept
    {
        return m_CurrentState;
    }

    void OpenGLTexture::Bind(u32 slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(m_Target, m_Handle);
    }

    // ========================================================================
    // OpenGLTexture2D
    // ========================================================================

    OpenGLTexture2D::OpenGLTexture2D(const TextureDescription& desc,
                                      OpenGLDebugLayer* debugLayer)
        : OpenGLTexture(desc, debugLayer)
    {
    }

    void OpenGLTexture2D::Upload(const void* pixels, usize size,
                                  u32 width, u32 height)
    {
        TextureData data;
        data.Pixels = pixels;
        data.Size   = size;
        data.Width  = width;
        data.Height = height;
        OpenGLTexture::UploadData(0, 0, data);
    }

    // ========================================================================
    // OpenGLTextureCube
    // ========================================================================

    OpenGLTextureCube::OpenGLTextureCube(const TextureDescription& desc,
                                          OpenGLDebugLayer* debugLayer)
        : OpenGLTexture(desc, debugLayer)
    {
    }

    void OpenGLTextureCube::UploadFace(u32 face, u32 mipLevel, const TextureData& data)
    {
        OpenGLTexture::UploadData(mipLevel, face, data);
    }

    // ========================================================================
    // OpenGLTextureArray
    // ========================================================================

    OpenGLTextureArray::OpenGLTextureArray(const TextureDescription& desc,
                                            OpenGLDebugLayer* debugLayer)
        : OpenGLTexture(desc, debugLayer)
    {
    }

    void OpenGLTextureArray::UploadLayer(u32 arrayLayer, u32 mipLevel,
                                          const TextureData& data)
    {
        OpenGLTexture::UploadData(mipLevel, arrayLayer, data);
    }

    // ========================================================================
    // OpenGLSampler
    // ========================================================================

    OpenGLSampler::OpenGLSampler(const SamplerDescription& desc,
                                  OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        glGenSamplers(1, &m_Handle);
        if (m_Handle == 0)
        {
            ENGINE_LOG_ERROR("OpenGLSampler — glGenSamplers failed");
            return;
        }

        glSamplerParameteri(m_Handle, GL_TEXTURE_MIN_FILTER,
                            ToGLMinFilter(desc.MinFilter, desc.MipFilter));
        glSamplerParameteri(m_Handle, GL_TEXTURE_MAG_FILTER,
                            ToGLMagFilter(desc.MagFilter));
        glSamplerParameteri(m_Handle, GL_TEXTURE_WRAP_S, ToGLAddressMode(desc.AddressU));
        glSamplerParameteri(m_Handle, GL_TEXTURE_WRAP_T, ToGLAddressMode(desc.AddressV));
        glSamplerParameteri(m_Handle, GL_TEXTURE_WRAP_R, ToGLAddressMode(desc.AddressW));

        glSamplerParameterfv(m_Handle, GL_TEXTURE_BORDER_COLOR, desc.BorderColor.data());

        if (desc.AnisotropyEnable && true)
        {
            glSamplerParameterf(m_Handle, GL_TEXTURE_MAX_ANISOTROPY, desc.MaxAnisotropy);
        }

        if (desc.CompareEnable)
        {
            glSamplerParameteri(m_Handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(m_Handle, GL_TEXTURE_COMPARE_FUNC,
                                ToGLCompareOperation(desc.CompareOp));
        }

        glSamplerParameterf(m_Handle, GL_TEXTURE_MIN_LOD, desc.MinLod);
        glSamplerParameterf(m_Handle, GL_TEXTURE_MAX_LOD, desc.MaxLod);
        glSamplerParameterf(m_Handle, GL_TEXTURE_LOD_BIAS, desc.MipLodBias);

        if (m_DebugLayer && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_SAMPLER, m_Handle, m_Name);
        }

        ENGINE_LOG_DEBUG("OpenGLSampler — created '{}' (handle={})", m_Name, m_Handle);
    }

    OpenGLSampler::~OpenGLSampler()
    {
        if (m_Handle != 0)
        {
            glDeleteSamplers(1, &m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLSampler::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_SAMPLER, m_Handle, m_Name);
    }

    void OpenGLSampler::Bind(u32 slot)
    {
        glBindSampler(slot, m_Handle);
    }

} // namespace engine::opengl
