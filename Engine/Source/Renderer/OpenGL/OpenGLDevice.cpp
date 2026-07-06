// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLDevice.cpp
// ============================================================================
#include "OpenGLDevice.h"
#include "OpenGLSwapChain.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace engine::opengl {

    using namespace engine::rhi;

    OpenGLDevice::OpenGLDevice()
        : m_Factory(nullptr) // Will be set after debug layer creation
    {
    }

    OpenGLDevice::~OpenGLDevice()
    {
        Shutdown();
    }

    bool OpenGLDevice::Initialize(GLFWwindow* window, bool enableDebug)
    {
        // Create context.
        m_Context = std::make_unique<OpenGLContext>();
        if (!m_Context->Create(window, enableDebug))
        {
            ENGINE_LOG_ERROR("OpenGLDevice — failed to create context");
            return false;
        }

        // Create debug layer.
        m_DebugLayer = std::make_unique<OpenGLDebugLayer>();
        if (enableDebug)
        {
            m_DebugLayer->Initialize();
        }

        // Create adapter + capabilities.
        m_Adapter = std::make_unique<OpenGLAdapter>();

        // Reconstruct factory with debug layer.
        m_Factory = OpenGLFactory(m_DebugLayer.get());

        // Create graphics command queue.
        m_GraphicsQueue = std::make_unique<OpenGLCommandQueue>(
            QueueType::Graphics, m_Context.get(), m_DebugLayer.get());

        ENGINE_LOG_INFO("OpenGLDevice — initialized");
        return true;
    }

    void OpenGLDevice::Shutdown()
    {
        m_GraphicsQueue.reset();
        m_Adapter.reset();
        if (m_DebugLayer)
        {
            m_DebugLayer->Shutdown();
            m_DebugLayer.reset();
        }
        m_Context.reset();
        ENGINE_LOG_INFO("OpenGLDevice — shut down");
    }

    ICommandQueue* OpenGLDevice::GetQueue(QueueType type)
    {
        if (type == QueueType::Graphics)
            return m_GraphicsQueue.get();
        return m_GraphicsQueue.get(); // OpenGL has a single queue.
    }

    void OpenGLDevice::WaitIdle()
    {
        glFinish();
    }

    void OpenGLDevice::Flush()
    {
        glFlush();
    }

    std::unique_ptr<ISwapChain> OpenGLDevice::CreateSwapChain(const SwapChainDescription& desc)
    {
        return std::make_unique<OpenGLSwapChain>(desc, m_Context.get(), m_DebugLayer.get());
    }

} // namespace engine::opengl
