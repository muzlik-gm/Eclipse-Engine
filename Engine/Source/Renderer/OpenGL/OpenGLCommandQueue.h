// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLCommandQueue.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"

namespace engine::opengl {

    class OpenGLDebugLayer;
    class OpenGLContext;

    class OpenGLCommandQueue final : public engine::rhi::ICommandQueue
    {
    public:
        OpenGLCommandQueue(engine::rhi::QueueType type, OpenGLContext* context,
                           OpenGLDebugLayer* debugLayer);
        ~OpenGLCommandQueue() override = default;

        std::string_view GetDebugName() const noexcept override { return "OpenGLCommandQueue"; }
        void SetDebugName(std::string_view) override {}
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        engine::rhi::QueueType GetType() const noexcept override { return m_Type; }
        void Submit(std::span<engine::rhi::ICommandBuffer* const> commandBuffers) override;
        void WaitIdle() override;
        void Present(class engine::rhi::ISwapChain* swapchain) override;

    private:
        engine::rhi::QueueType m_Type;
        OpenGLContext*         m_Context{nullptr};
        OpenGLDebugLayer*      m_DebugLayer{nullptr};
    };

} // namespace engine::opengl
