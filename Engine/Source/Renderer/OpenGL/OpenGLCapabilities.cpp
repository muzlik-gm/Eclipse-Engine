// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCapabilities.cpp
// ============================================================================
#include "OpenGLCapabilities.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::u32;

    OpenGLCapabilities::OpenGLCapabilities()
    {
        Initialize();
    }

    void OpenGLCapabilities::Initialize()
    {
        const GLubyte* vendorStr   = glGetString(GL_VENDOR);
        const GLubyte* rendererStr = glGetString(GL_RENDERER);
        const GLubyte* versionStr  = glGetString(GL_VERSION);

        m_DeviceName    = rendererStr ? reinterpret_cast<const char*>(rendererStr) : "Unknown";
        m_DriverVersion = versionStr  ? reinterpret_cast<const char*>(versionStr)  : "Unknown";

        // Format API version.
        GLint glMajor = 0, glMinor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
        glGetIntegerv(GL_MINOR_VERSION, &glMinor);
        m_APIVersion = std::to_string(glMajor) + "." + std::to_string(glMinor);

        QueryVendor();
        QueryLimits();
        QueryFeatures();

        ENGINE_LOG_INFO("OpenGLCapabilities — {} / {} / OpenGL {}",
                        m_DeviceName, m_DriverVersion, m_APIVersion);
    }

    void OpenGLCapabilities::QueryVendor()
    {
        const std::string vendorStr = m_DeviceName;
        const std::string vendorLower = [v = std::string(m_DriverVersion)]() {
            std::string s = v;
            for (auto& c : s) c = static_cast<char>(std::tolower(c));
            return s;
        }();

        const std::string nameLower = [n = m_DeviceName]() {
            std::string s = n;
            for (auto& c : s) c = static_cast<char>(std::tolower(c));
            return s;
        }();

        if (nameLower.find("nvidia") != std::string::npos
            || vendorLower.find("nvidia") != std::string::npos)
            m_Vendor = GraphicsVendor::NVIDIA;
        else if (nameLower.find("amd") != std::string::npos
                 || nameLower.find("radeon") != std::string::npos
                 || vendorLower.find("ati") != std::string::npos)
            m_Vendor = GraphicsVendor::AMD;
        else if (nameLower.find("intel") != std::string::npos)
            m_Vendor = GraphicsVendor::Intel;
        else if (nameLower.find("apple") != std::string::npos)
            m_Vendor = GraphicsVendor::Apple;
        else if (nameLower.find("arm") != std::string::npos
                 || nameLower.find("mali") != std::string::npos)
            m_Vendor = GraphicsVendor::ARM;
        else if (nameLower.find("qualcomm") != std::string::npos
                 || nameLower.find("adreno") != std::string::npos)
            m_Vendor = GraphicsVendor::Qualcomm;
        else if (nameLower.find("mesa") != std::string::npos)
            m_Vendor = GraphicsVendor::Mesa;
        else
            m_Vendor = GraphicsVendor::Unknown;
    }

    void OpenGLCapabilities::QueryLimits()
    {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, reinterpret_cast<GLint*>(&m_Limits.MaxTexture2DSize));
        m_Limits.MaxTexture1DSize = m_Limits.MaxTexture2DSize;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, reinterpret_cast<GLint*>(&m_Limits.MaxTexture3DSize));
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, reinterpret_cast<GLint*>(&m_Limits.MaxTextureCubeSize));
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, reinterpret_cast<GLint*>(&m_Limits.MaxTextureArrayLayers));

        GLint maxUBOSize = 0;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUBOSize);
        m_Limits.MaxUniformBufferSize = static_cast<u32>(maxUBOSize);
        m_Limits.MaxStorageBufferSize = static_cast<u32>(maxUBOSize) * 16;

        GLint maxVertexAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
        m_Limits.MaxVertexInputAttributes = static_cast<u32>(maxVertexAttribs);

        GLint maxVertexBindings = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &maxVertexBindings);
        m_Limits.MaxVertexInputBindings = static_cast<u32>(maxVertexBindings);

        GLint maxColorAttachments = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
        m_Limits.MaxColorAttachments = static_cast<u32>(maxColorAttachments);

        GLint maxFBWidth = 0, maxFBHeight = 0, maxFBLayers = 0;
        glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &maxFBWidth);
        glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &maxFBHeight);
        glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &maxFBLayers);
        m_Limits.MaxFramebufferWidth  = static_cast<u32>(maxFBWidth);
        m_Limits.MaxFramebufferHeight = static_cast<u32>(maxFBHeight);
        m_Limits.MaxFramebufferLayers = static_cast<u32>(maxFBLayers);

        GLint maxComputeWGCount[3] = {0};
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeWGCount[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxComputeWGCount[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxComputeWGCount[2]);
        m_Limits.MaxComputeWorkGroupCountX = static_cast<u32>(maxComputeWGCount[0]);
        m_Limits.MaxComputeWorkGroupCountY = static_cast<u32>(maxComputeWGCount[1]);
        m_Limits.MaxComputeWorkGroupCountZ = static_cast<u32>(maxComputeWGCount[2]);

        GLint maxComputeWGSize[3] = {0};
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxComputeWGSize[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxComputeWGSize[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxComputeWGSize[2]);
        m_Limits.MaxComputeWorkGroupSizeX = static_cast<u32>(maxComputeWGSize[0]);
        m_Limits.MaxComputeWorkGroupSizeY = static_cast<u32>(maxComputeWGSize[1]);
        m_Limits.MaxComputeWorkGroupSizeZ = static_cast<u32>(maxComputeWGSize[2]);

        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE,
                       reinterpret_cast<GLint*>(&m_Limits.MaxComputeSharedMemorySize));

        // Max anisotropy.
        GLfloat maxAniso = 1.0f;
        if (true)
        {
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        }
        m_MaxAnisotropy = static_cast<u32>(maxAniso);
        m_Limits.MaxSamplerAnisotropy = m_MaxAnisotropy;

        // Max push constants size — GL has UBO size for this.
        m_Limits.MaxPushConstantsSize = m_Limits.MaxUniformBufferSize;

        // Max viewports.
        GLint maxViewports = 1;
        glGetIntegerv(GL_MAX_VIEWPORTS, &maxViewports);
        m_Limits.MaxViewports = static_cast<u32>(maxViewports);

        GLint maxVPWidth = 0, maxVPHeight = 0;
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &maxVPWidth);
        maxVPHeight = maxVPWidth; // GL returns array[2]
        m_Limits.MaxViewportWidth  = static_cast<u32>(maxVPWidth);
        m_Limits.MaxViewportHeight = static_cast<u32>(maxVPHeight);

        // MSAA sample counts — query which are supported.
        GLint maxSamples = 1;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        m_MaxSamples = static_cast<u32>(maxSamples);
    }

    void OpenGLCapabilities::QueryFeatures()
    {
        m_Features.SupportsGeometryShader    = GLAD_GL_VERSION_4_0 != 0;
        m_Features.SupportsTessellationShader = GLAD_GL_VERSION_4_0 != 0;
        m_Features.SupportsComputeShader     = GLAD_GL_VERSION_4_3 != 0;
        m_Features.SupportsMeshShader        = false != 0;
        m_Features.SupportsRayTracing        = false;

        m_Features.SupportsAnisotropicFiltering = true != 0;
        m_Features.SupportsTextureCompressionBC = GLAD_GL_VERSION_3_0 != 0;
        m_Features.SupportsTextureCompressionETC2 = false;
        m_Features.SupportsTextureCompressionASTC = false;

        m_Features.SupportsMultiDrawIndirect       = GLAD_GL_VERSION_4_3 != 0;
        m_Features.SupportsDrawIndirectFirstInstance = GLAD_GL_VERSION_4_2 != 0;

        m_Features.SupportsPipelineStatisticsQuery = GLAD_GL_VERSION_3_3 != 0;
        m_Features.SupportsOcclusionQuery          = GLAD_GL_VERSION_1_5 != 0;
        m_Features.SupportsTimestampQuery          = GLAD_GL_VERSION_3_3 != 0;

        m_Features.SupportsWireframeFill           = true;
        m_Features.SupportsDepthClamp              = GLAD_GL_VERSION_3_2 != 0;
        m_Features.SupportsDepthBias               = true;

        m_Features.SupportsIndependentBlend        = GLAD_GL_VERSION_4_0 != 0;
        m_Features.SupportsDualSourceBlend         = GLAD_GL_VERSION_3_3 != 0;
        m_Features.SupportsLogicOp                 = false;

        m_Features.SupportsPersistentMapping       = GLAD_GL_VERSION_4_4 != 0;
        m_Features.SupportsMultiSampledFramebuffers = true;

        m_Features.SupportsDebugMarkers  = GLAD_GL_VERSION_4_3 != 0;
        m_Features.SupportsDebugOutput   = GLAD_GL_VERSION_4_3 != 0;
        m_Features.SupportsPushConstants = true;
        m_Features.SupportsDescriptorIndexing = GLAD_GL_VERSION_4_2 != 0;
    }

    GraphicsBackend OpenGLCapabilities::GetBackend() const noexcept
    {
        return GraphicsBackend::OpenGL;
    }

    GraphicsVendor OpenGLCapabilities::GetVendor() const noexcept
    {
        return m_Vendor;
    }

    const std::string& OpenGLCapabilities::GetDeviceName() const noexcept
    {
        return m_DeviceName;
    }

    const std::string& OpenGLCapabilities::GetDriverVersion() const noexcept
    {
        return m_DriverVersion;
    }

    const std::string& OpenGLCapabilities::GetAPIVersion() const noexcept
    {
        return m_APIVersion;
    }

    const AdapterLimits& OpenGLCapabilities::GetLimits() const noexcept
    {
        return m_Limits;
    }

    const AdapterFeatures& OpenGLCapabilities::GetFeatures() const noexcept
    {
        return m_Features;
    }

    FormatProperties OpenGLCapabilities::GetFormatProperties(GraphicsFormat format) const
    {
        FormatProperties props;
        props.Format = format;

        // Most color formats are universally supported in OpenGL 4.6.
        if (format != GraphicsFormat::Undefined)
        {
            props.SupportsSampled       = true;
            props.SupportsTransfer      = true;
            props.SupportsRenderTarget  = !engine::rhi::IsDepthFormat(format);
            props.SupportsDepthStencil  = engine::rhi::IsDepthFormat(format);
            props.SupportsBlend         = !engine::rhi::IsDepthFormat(format);
            props.SupportsStorage       = (format == GraphicsFormat::RGBA8_UNorm
                                        || format == GraphicsFormat::RGBA32_Float
                                        || format == GraphicsFormat::R32_Float);
        }

        return props;
    }

    bool OpenGLCapabilities::IsFormatSupported(GraphicsFormat format, TextureUsage) const
    {
        return format != GraphicsFormat::Undefined;
    }

    u32 OpenGLCapabilities::GetMaxSampleCount() const noexcept
    {
        return m_MaxSamples;
    }

    u32 OpenGLCapabilities::GetMaxAnisotropy() const noexcept
    {
        return m_MaxAnisotropy;
    }

} // namespace engine::opengl
