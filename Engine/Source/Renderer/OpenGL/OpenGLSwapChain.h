// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLSwapChain.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"

#include <glad/glad.h>
#include <memory>
#include <string>

struct GLFWwindow;

namespace engine::opengl {

    class OpenGLDebugLayer;
    class OpenGLContext;
    class OpenGLTexture2D;

    class OpenGLSwapChain final : public engine::rhi::ISwapChain
    {
    public:
        OpenGLSwapChain(const engine::rhi::SwapChainDescription& desc,
                        OpenGLContext* context,
                        OpenGLDebugLayer* debugLayer);
        ~OpenGLSwapChain() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return m_Context != nullptr; }

        const engine::rhi::SwapChainDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::rhi::ITexture* GetCurrentBuffer() const noexcept override;
        engine::rhi::ITexture* GetBuffer(engine::core::u32 index) const noexcept override;
        engine::core::u32 GetBufferCount() const noexcept override { return 1; }
        void AcquireNextImage() override;
        void Present() override;
        void Resize(engine::core::u32 width, engine::core::u32 height) override;
        bool NeedsResize() const noexcept override { return m_NeedsResize; }
        engine::core::u32 GetCurrentBufferIndex() const noexcept override { return 0; }

    private:
        engine::rhi::SwapChainDescription m_Description;
        OpenGLContext*                     m_Context{nullptr};
        OpenGLDebugLayer*                  m_DebugLayer{nullptr};
        std::string                        m_Name;
        bool                               m_NeedsResize{false};
        std::unique_ptr<OpenGLTexture2D>   m_ColorBuffer;
    };

} // namespace engine::opengl
