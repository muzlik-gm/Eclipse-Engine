// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLDevice.h
// ============================================================================
#pragma once

#include "Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h"
#include "OpenGLContext.h"
#include "OpenGLAdapter.h"
#include "OpenGLCapabilities.h"
#include "OpenGLDebugLayer.h"
#include "OpenGLFactory.h"
#include "OpenGLCommandQueue.h"

#include <memory>

namespace engine::opengl {

    class OpenGLDevice final : public engine::rhi::IGraphicsDevice
    {
    public:
        OpenGLDevice();
        ~OpenGLDevice() override;

        bool Initialize(GLFWwindow* window, bool enableDebug);
        void Shutdown();

        // -- IGraphicsObject
        std::string_view GetDebugName() const noexcept override { return "OpenGLDevice"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return m_Context != nullptr; }

        // -- IGraphicsDevice
        engine::rhi::IGraphicsInstance* GetInstance() const noexcept override { return nullptr; }
        engine::rhi::IGraphicsAdapter* GetAdapter() const noexcept override { return m_Adapter.get(); }
        engine::rhi::IGraphicsContext* GetContext() const noexcept override { return m_Context.get(); }
        engine::rhi::ICommandQueue* GetQueue(engine::rhi::QueueType type) override;
        const engine::rhi::ICapabilities& GetCapabilities() const noexcept override
        { return m_Capabilities; }
        engine::rhi::IGraphicsFactory& GetFactory() override { return m_Factory; }

        std::unique_ptr<engine::rhi::ISwapChain>
            CreateSwapChain(const engine::rhi::SwapChainDescription& desc) override;

        void WaitIdle() override;
        void Flush() override;

        // -- OpenGL-specific
        [[nodiscard]] OpenGLDebugLayer* GetDebugLayer() const noexcept { return m_DebugLayer.get(); }
        [[nodiscard]] OpenGLContext* GetGLContext() const noexcept { return m_Context.get(); }

    private:
        std::unique_ptr<OpenGLContext>        m_Context;
        std::unique_ptr<OpenGLAdapter>        m_Adapter;
        OpenGLCapabilities                    m_Capabilities;
        std::unique_ptr<OpenGLDebugLayer>     m_DebugLayer;
        OpenGLFactory                         m_Factory;
        std::unique_ptr<OpenGLCommandQueue>   m_GraphicsQueue;
    };

} // namespace engine::opengl
