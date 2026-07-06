// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLRenderPass.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"

#include <glad/glad.h>
#include <string>

namespace engine::opengl {

    using engine::core::u32;
    using engine::core::u64;
    using engine::rhi::IGraphicsObject;
    using engine::rhi::IRenderPass;
    using engine::rhi::IFramebuffer;
    using engine::rhi::ITexture;

    class OpenGLDebugLayer;

    class OpenGLRenderPass final : public engine::rhi::IRenderPass
    {
    public:
        OpenGLRenderPass(const engine::rhi::RenderPassDescription& desc,
                         OpenGLDebugLayer* debugLayer);
        ~OpenGLRenderPass() override = default;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        const engine::rhi::RenderPassDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::core::u32 GetColorAttachmentCount() const noexcept override
        { return static_cast<engine::core::u32>(m_Description.ColorAttachments.size()); }

        bool HasDepthStencilAttachment() const noexcept override
        { return m_Description.HasDepthStencil; }

        engine::core::u32 GetWidth() const noexcept override { return m_Description.Width; }
        engine::core::u32 GetHeight() const noexcept override { return m_Description.Height; }

    private:
        engine::rhi::RenderPassDescription m_Description;
        std::string m_Name;
    };

    // ========================================================================
    // OpenGLFramebuffer
    // ========================================================================

    class OpenGLFramebuffer final : public engine::rhi::IFramebuffer
    {
    public:
        OpenGLFramebuffer(const engine::rhi::FramebufferDescription& desc,
                          OpenGLRenderPass* renderPass,
                          OpenGLDebugLayer* debugLayer);
        ~OpenGLFramebuffer() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        const engine::rhi::FramebufferDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::rhi::IRenderPass* GetRenderPass() const noexcept override { return m_RenderPass; }

        engine::core::u32 GetWidth() const noexcept override { return m_Description.Width; }
        engine::core::u32 GetHeight() const noexcept override { return m_Description.Height; }
        engine::core::u32 GetLayers() const noexcept override { return m_Description.Layers; }

        engine::rhi::ITexture* GetColorAttachment(engine::core::u32 index) const noexcept override;
        engine::rhi::ITexture* GetDepthStencilAttachment() const noexcept override;

        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }
        void Bind();

    private:
        engine::rhi::FramebufferDescription m_Description;
        OpenGLRenderPass*                    m_RenderPass{nullptr};
        GLuint                               m_Handle{0};
        std::string                          m_Name;
        OpenGLDebugLayer*                    m_DebugLayer{nullptr};
    };

} // namespace engine::opengl
