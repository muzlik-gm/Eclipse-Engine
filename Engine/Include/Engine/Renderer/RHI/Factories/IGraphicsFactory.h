// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Factories/IGraphicsFactory.h
// Abstract factory interface for creating every type of GPU resource.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Interfaces/IShader.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "Engine/Renderer/RHI/Interfaces/ISync.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"

#include <memory>

namespace engine::rhi {

    // ========================================================================
    // IGraphicsFactory — creates all GPU resources.
    // ========================================================================

    /// @brief The central factory that creates every GPU resource.
    ///
    /// Backends implement this interface to translate engine resource
    /// descriptions into concrete backend objects.  All factory methods
    /// return std::unique_ptr so the caller owns the resource through RAII.
    class IGraphicsFactory
    {
    public:
        virtual ~IGraphicsFactory() = default;

        // -- Buffers ------------------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IVertexBuffer>  CreateVertexBuffer(const BufferDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IIndexBuffer>   CreateIndexBuffer(const BufferDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IUniformBuffer> CreateUniformBuffer(const BufferDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IStorageBuffer> CreateStorageBuffer(const BufferDescription& desc) = 0;

        // -- Textures -----------------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<ITexture2D>    CreateTexture2D(const TextureDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<ITextureCube>  CreateTextureCube(const TextureDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<ITextureArray> CreateTextureArray(const TextureDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<ISampler>      CreateSampler(const SamplerDescription& desc) = 0;

        // -- Shaders ------------------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IShaderModule> CreateShaderModule(const ShaderStageDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IShader>       CreateShader(const ShaderDescription& desc) = 0;

        // -- Pipelines ----------------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IPipelineLayout>     CreatePipelineLayout(const PipelineLayoutDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IGraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IComputePipeline>   CreateComputePipeline(const ComputePipelineDescription& desc) = 0;

        // -- Render passes / framebuffers --------------------------------

        [[nodiscard]] virtual std::unique_ptr<IRenderPass>   CreateRenderPass(const RenderPassDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IFramebuffer>  CreateFramebuffer(const FramebufferDescription& desc,
                                                                                IRenderPass* renderPass) = 0;

        // -- Vertex arrays -----------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IVertexArray>  CreateVertexArray() = 0;

        // -- Command buffers / queues ------------------------------------

        [[nodiscard]] virtual std::unique_ptr<ICommandBuffer> CreateCommandBuffer(QueueType queueType = QueueType::Graphics) = 0;

        // -- Swapchain ----------------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<ISwapChain>  CreateSwapChain(const SwapChainDescription& desc,
                                                                            ICommandQueue* presentQueue) = 0;

        // -- Synchronization ---------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IFence>     CreateFence(bool signaled = false) = 0;
        [[nodiscard]] virtual std::unique_ptr<ISemaphore> CreateSemaphore(u64 initialValue = 0) = 0;

        // -- Descriptor sets ---------------------------------------------

        [[nodiscard]] virtual std::unique_ptr<IDescriptorLayout> CreateDescriptorLayout(const DescriptorLayoutDescription& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<IDescriptorPool>   CreateDescriptorPool(u32 maxSets,
                                                                                       const std::vector<DescriptorType>& allowedTypes) = 0;
    };

} // namespace engine::rhi
