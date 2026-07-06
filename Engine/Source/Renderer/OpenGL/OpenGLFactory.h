// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLFactory.h
// ============================================================================
#pragma once

#include "Engine/Renderer/RHI/Factories/IGraphicsFactory.h"
#include "OpenGLDebugLayer.h"

namespace engine::opengl {

    class OpenGLFactory final : public engine::rhi::IGraphicsFactory
    {
    public:
        explicit OpenGLFactory(OpenGLDebugLayer* debugLayer) : m_DebugLayer(debugLayer) {}
        ~OpenGLFactory() override = default;

        // -- Buffers
        std::unique_ptr<engine::rhi::IVertexBuffer>  CreateVertexBuffer(const engine::rhi::BufferDescription& desc) override;
        std::unique_ptr<engine::rhi::IIndexBuffer>   CreateIndexBuffer(const engine::rhi::BufferDescription& desc) override;
        std::unique_ptr<engine::rhi::IUniformBuffer> CreateUniformBuffer(const engine::rhi::BufferDescription& desc) override;
        std::unique_ptr<engine::rhi::IStorageBuffer> CreateStorageBuffer(const engine::rhi::BufferDescription& desc) override;

        // -- Textures
        std::unique_ptr<engine::rhi::ITexture2D>    CreateTexture2D(const engine::rhi::TextureDescription& desc) override;
        std::unique_ptr<engine::rhi::ITextureCube>  CreateTextureCube(const engine::rhi::TextureDescription& desc) override;
        std::unique_ptr<engine::rhi::ITextureArray> CreateTextureArray(const engine::rhi::TextureDescription& desc) override;
        std::unique_ptr<engine::rhi::ISampler>      CreateSampler(const engine::rhi::SamplerDescription& desc) override;

        // -- Shaders
        std::unique_ptr<engine::rhi::IShaderModule> CreateShaderModule(const engine::rhi::ShaderStageDescription& desc) override;
        std::unique_ptr<engine::rhi::IShader>       CreateShader(const engine::rhi::ShaderDescription& desc) override;

        // -- Pipelines
        std::unique_ptr<engine::rhi::IPipelineLayout>    CreatePipelineLayout(const engine::rhi::PipelineLayoutDescription& desc) override;
        std::unique_ptr<engine::rhi::IGraphicsPipeline>  CreateGraphicsPipeline(const engine::rhi::GraphicsPipelineDescription& desc) override;
        std::unique_ptr<engine::rhi::IComputePipeline>   CreateComputePipeline(const engine::rhi::ComputePipelineDescription& desc) override;

        // -- Render passes / framebuffers
        std::unique_ptr<engine::rhi::IRenderPass>  CreateRenderPass(const engine::rhi::RenderPassDescription& desc) override;
        std::unique_ptr<engine::rhi::IFramebuffer> CreateFramebuffer(const engine::rhi::FramebufferDescription& desc,
                                                                     engine::rhi::IRenderPass* renderPass) override;

        // -- Vertex arrays
        std::unique_ptr<engine::rhi::IVertexArray>  CreateVertexArray() override;

        // -- Command buffers
        std::unique_ptr<engine::rhi::ICommandBuffer> CreateCommandBuffer(engine::rhi::QueueType queueType = engine::rhi::QueueType::Graphics) override;

        // -- Swapchain
        std::unique_ptr<engine::rhi::ISwapChain> CreateSwapChain(const engine::rhi::SwapChainDescription& desc,
                                                                  engine::rhi::ICommandQueue* presentQueue) override;

        // -- Sync
        std::unique_ptr<engine::rhi::IFence>     CreateFence(bool signaled = false) override;
        std::unique_ptr<engine::rhi::ISemaphore> CreateSemaphore(engine::core::u64 initialValue = 0) override;

        // -- Descriptor sets
        std::unique_ptr<engine::rhi::IDescriptorLayout> CreateDescriptorLayout(const engine::rhi::DescriptorLayoutDescription& desc) override;
        std::unique_ptr<engine::rhi::IDescriptorPool>   CreateDescriptorPool(engine::core::u32 maxSets,
                                                                              const std::vector<engine::rhi::DescriptorType>& allowedTypes) override;

    private:
        OpenGLDebugLayer* m_DebugLayer{nullptr};
    };

} // namespace engine::opengl
