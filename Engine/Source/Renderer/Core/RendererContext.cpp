// ============================================================================
// File: Engine/Source/Renderer/Core/RendererContext.cpp
// ============================================================================
#include "Engine/Renderer/Core/RendererContext.h"
#include "Engine/Core/Log.h"

namespace engine::renderer {

    using namespace engine::rhi;

    bool RendererContext::Initialize(const RendererConfiguration& config)
    {
        m_Config = config;

        // Find the requested backend.
        IGraphicsBackend* backend = GraphicsBackendRegistry::Get().GetBackend(config.Backend);
        if (!backend)
        {
            ENGINE_LOG_ERROR("RendererContext — backend {} not available",
                             GraphicsBackendToString(config.Backend));
            return false;
        }

        // Create the instance.
        BackendCreateInfo createInfo{};
        createInfo.Backend          = config.Backend;
        createInfo.ApplicationName  = "Eclipse Engine";
        createInfo.EnableValidation = config.EnableValidation;
        createInfo.EnableDebugOutput = config.EnableDebugOutput;
        createInfo.WindowHandle     = config.WindowHandle;
        createInfo.WindowWidth      = config.Width;
        createInfo.WindowHeight     = config.Height;

        m_Instance = backend->CreateInstance(createInfo);
        if (!m_Instance)
        {
            ENGINE_LOG_ERROR("RendererContext — failed to create graphics instance");
            return false;
        }

        // Get the logical device.
        m_Device = m_Instance->GetDevice();
        if (!m_Device)
        {
            ENGINE_LOG_ERROR("RendererContext — no device available");
            return false;
        }

        // Get the graphics command queue.
        m_CommandQueue = m_Device->GetQueue(QueueType::Graphics);

        // Create the swapchain.
        SwapChainDescription scDesc{};
        scDesc.Width       = config.Width;
        scDesc.Height      = config.Height;
        scDesc.BufferCount = config.BufferCount;
        scDesc.Format      = config.SwapchainFormat;
        scDesc.PresentModeValue = config.VSync ? PresentMode::FIFO : PresentMode::Immediate;
        scDesc.VSync       = config.VSync;
        scDesc.WindowHandle = config.WindowHandle;

        // Create the swapchain via the device.
        m_SwapChain = m_Device->CreateSwapChain(scDesc);

        // Create a command buffer for recording.
        m_CommandBuffer = m_Device->GetFactory().CreateCommandBuffer(QueueType::Graphics);

        ENGINE_LOG_INFO("RendererContext — initialized with backend '{}'",
                        GraphicsBackendToString(config.Backend));
        return true;
    }

    void RendererContext::Shutdown()
    {
        m_CommandBuffer.reset();
        m_SwapChain.reset();
        m_CommandQueue = nullptr;
        m_Device = nullptr;
        m_Instance.reset();

        ENGINE_LOG_INFO("RendererContext — shut down");
    }

} // namespace engine::renderer
