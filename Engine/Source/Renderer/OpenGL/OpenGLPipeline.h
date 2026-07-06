// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLPipeline.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"

#include <glad/glad.h>
#include <memory>
#include <string>

namespace engine::opengl {

    class OpenGLShader;
    class OpenGLDebugLayer;
    class OpenGLRenderPass;
    class OpenGLStateCache;

    // ========================================================================
    // OpenGLPipelineLayout
    // ========================================================================

    class OpenGLPipelineLayout final : public engine::rhi::IPipelineLayout
    {
    public:
        OpenGLPipelineLayout(const engine::rhi::PipelineLayoutDescription& desc,
                             OpenGLDebugLayer* debugLayer);
        ~OpenGLPipelineLayout() override = default;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return true; }

        const engine::rhi::PipelineLayoutDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::core::u32 GetSetLayoutCount() const noexcept override
        { return static_cast<engine::core::u32>(m_Description.SetLayouts.size()); }

        engine::core::u32 GetPushConstantsSize() const noexcept override
        {
            engine::core::u32 total = 0;
            for (const auto& pc : m_Description.PushConstants)
                total += pc.Size;
            return total;
        }

    private:
        engine::rhi::PipelineLayoutDescription m_Description;
        std::string m_Name;
    };

    // ========================================================================
    // OpenGLGraphicsPipeline
    // ========================================================================

    class OpenGLGraphicsPipeline final : public engine::rhi::IGraphicsPipeline
    {
    public:
        OpenGLGraphicsPipeline(const engine::rhi::GraphicsPipelineDescription& desc,
                               OpenGLDebugLayer* debugLayer);
        ~OpenGLGraphicsPipeline() override = default;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override { m_Name = std::string(name); }
        engine::core::u64 GetNativeHandle() const noexcept override { return 0; }
        bool IsValid() const noexcept override { return m_Valid; }

        // -- IPipeline
        engine::rhi::IPipelineLayout* GetLayout() const noexcept override { return m_Layout.get(); }
        engine::rhi::IShader* GetShader() const noexcept override;
        const std::string& GetValidationLog() const noexcept override { return m_ValidationLog; }

        // -- IGraphicsPipeline
        const engine::rhi::GraphicsPipelineDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::rhi::PrimitiveTopology GetTopology() const noexcept override
        { return m_Description.TopologyValue; }
        const engine::rhi::RasterizerStateDescription& GetRasterizerState() const noexcept override
        { return m_Description.RasterizerState; }
        const engine::rhi::BlendStateDescription& GetBlendState() const noexcept override
        { return m_Description.BlendState; }
        const engine::rhi::DepthStencilDescription& GetDepthStencilState() const noexcept override
        { return m_Description.DepthStencilState; }
        const engine::rhi::VertexLayoutDescription& GetVertexLayout() const noexcept override
        { return m_Description.VertexLayout; }

        engine::rhi::IRenderPass* GetRenderPass() const noexcept override { return nullptr; }

        // -- OpenGL-specific
        void Apply(OpenGLStateCache& stateCache);

    private:
        void Validate();

        engine::rhi::GraphicsPipelineDescription                m_Description;
        std::unique_ptr<OpenGLPipelineLayout>                   m_Layout;
        std::unique_ptr<OpenGLShader>                           m_Shader;
        std::string                                             m_Name;
        std::string                                             m_ValidationLog;
        bool                                                    m_Valid{false};
        OpenGLDebugLayer*                                       m_DebugLayer{nullptr};
    };

} // namespace engine::opengl
