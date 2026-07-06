// ============================================================================
// File: Engine/Source/Renderer/Core/Renderer.cpp
// ============================================================================
#include "Engine/Renderer/Core/Renderer.h"
#include "Engine/Core/Log.h"

#include <chrono>

namespace engine::renderer {

    using engine::core::f64;
    using engine::core::u32;

    Renderer::Renderer() = default;

    Renderer::~Renderer()
    {
        Shutdown();
    }

    void Renderer::SetConfiguration(const RendererConfiguration& config)
    {
        m_Config = config;
    }

    bool Renderer::Initialize()
    {
        if (m_Initialized)
        {
            ENGINE_LOG_WARN("Renderer — already initialized");
            return true;
        }

        if (!m_Context.Initialize(m_Config))
        {
            ENGINE_LOG_ERROR("Renderer — failed to initialize context");
            return false;
        }

        m_Initialized = true;
        ENGINE_LOG_INFO("Renderer — initialized");
        return true;
    }

    void Renderer::Shutdown()
    {
        if (!m_Initialized)
            return;

        m_Context.Shutdown();
        m_Initialized = false;
        m_FrameInProgress = false;
        ENGINE_LOG_INFO("Renderer — shut down");
    }

    void Renderer::Update(f64 deltaTime)
    {
        if (!m_Initialized)
            return;

        auto& stats = m_Context.GetStatistics();
        stats.FrameTimeMs = deltaTime * 1000.0;
        stats.FrameCount++;
        stats.UpdateAverage();
    }

    void Renderer::FixedUpdate(f64 /*fixedDeltaTime*/)
    {
        // Rendering happens in Update, not FixedUpdate.
    }

    void Renderer::LateUpdate(f64 /*deltaTime*/)
    {
        // No late update work for the renderer.
    }

    void Renderer::BeginFrame()
    {
        if (!m_Initialized || m_FrameInProgress)
            return;

        m_Context.GetStatistics().Reset();

        // Acquire the swapchain image.
        if (m_Context.GetSwapChain())
        {
            m_Context.GetSwapChain()->AcquireNextImage();
        }

        // Begin recording the command buffer.
        if (m_Context.GetCommandBuffer())
        {
            m_Context.GetCommandBuffer()->Begin();
            m_Context.GetCommandBuffer()->BeginFrame();
        }

        m_FrameInProgress = true;
    }

    void Renderer::EndFrame()
    {
        if (!m_Initialized || !m_FrameInProgress)
            return;

        // End recording and submit.
        if (m_Context.GetCommandBuffer())
        {
            m_Context.GetCommandBuffer()->EndFrame();
            m_Context.GetCommandBuffer()->End();
        }

        // Submit to the queue.
        if (m_Context.GetCommandQueue() && m_Context.GetCommandBuffer())
        {
            auto* cmdBuf = m_Context.GetCommandBuffer();
            m_Context.GetCommandQueue()->Submit(cmdBuf);
        }

        // Present.
        if (m_Context.GetSwapChain())
        {
            m_Context.GetSwapChain()->Present();
        }

        m_FrameInProgress = false;
    }

    void Renderer::OnResize(u32 width, u32 height)
    {
        if (!m_Initialized)
            return;

        if (m_Context.GetSwapChain())
        {
            m_Context.GetSwapChain()->Resize(width, height);
        }

        ENGINE_LOG_DEBUG("Renderer — resized to {}x{}", width, height);
    }

} // namespace engine::renderer
