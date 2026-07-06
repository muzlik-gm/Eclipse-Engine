// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLSwapChain.cpp
// ============================================================================
#include "OpenGLSwapChain.h"
#include "OpenGLContext.h"
#include "OpenGLDebugLayer.h"
#include "OpenGLTexture.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine::opengl {

    using namespace engine::rhi;

    OpenGLSwapChain::OpenGLSwapChain(const SwapChainDescription& desc,
                                      OpenGLContext* context,
                                      OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Context(context)
        , m_DebugLayer(debugLayer)
    {
        // In OpenGL, the swapchain is implicit (managed by the windowing
        // system).  We create a back-buffer texture for compatibility
        // with the RHI's ITexture-based interface.
        TextureDescription texDesc{};
        texDesc.Name      = "SwapChain_BackBuffer";
        texDesc.Type      = TextureType::Texture2D;
        texDesc.Format    = desc.Format;
        texDesc.Usage     = TextureUsage::RenderTarget | TextureUsage::Sampled;
        texDesc.Width     = desc.Width;
        texDesc.Height    = desc.Height;
        texDesc.MipLevels = 1;
        texDesc.ArrayLayers = 1;
        texDesc.Samples   = 1;

        m_ColorBuffer = std::make_unique<OpenGLTexture2D>(texDesc, debugLayer);

        ENGINE_LOG_DEBUG("OpenGLSwapChain — created ({}x{}, format={})",
                         desc.Width, desc.Height, static_cast<u32>(desc.Format));
    }

    OpenGLSwapChain::~OpenGLSwapChain() = default;

    ITexture* OpenGLSwapChain::GetCurrentBuffer() const noexcept
    {
        return m_ColorBuffer.get();
    }

    ITexture* OpenGLSwapChain::GetBuffer(u32 /*index*/) const noexcept
    {
        return m_ColorBuffer.get();
    }

    void OpenGLSwapChain::AcquireNextImage()
    {
        // OpenGL uses a single back-buffer — no acquisition needed.
    }

    void OpenGLSwapChain::Present()
    {
        if (m_Context)
            m_Context->SwapBuffers();
    }

    void OpenGLSwapChain::Resize(u32 width, u32 height)
    {
        m_Description.Width = width;
        m_Description.Height = height;

        // Recreate the back-buffer texture.
        TextureDescription texDesc{};
        texDesc.Name      = "SwapChain_BackBuffer";
        texDesc.Type      = TextureType::Texture2D;
        texDesc.Format    = m_Description.Format;
        texDesc.Usage     = TextureUsage::RenderTarget | TextureUsage::Sampled;
        texDesc.Width     = width;
        texDesc.Height    = height;
        texDesc.MipLevels = 1;
        texDesc.ArrayLayers = 1;
        texDesc.Samples   = 1;

        m_ColorBuffer = std::make_unique<OpenGLTexture2D>(texDesc, m_DebugLayer);
        m_NeedsResize = false;

        ENGINE_LOG_DEBUG("OpenGLSwapChain — resized to {}x{}", width, height);
    }

} // namespace engine::opengl
