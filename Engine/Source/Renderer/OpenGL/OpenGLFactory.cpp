// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLFactory.cpp
// ============================================================================
#include "OpenGLFactory.h"
#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLShader.h"
#include "OpenGLPipeline.h"
#include "OpenGLRenderPass.h"
#include "OpenGLVertexArray.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLSwapChain.h"
#include "OpenGLSync.h"
#include "OpenGLContext.h"
#include "OpenGLCommandQueue.h"

namespace engine::opengl {

    using namespace engine::rhi;

    std::unique_ptr<IVertexBuffer> OpenGLFactory::CreateVertexBuffer(const BufferDescription& desc)
    { return std::make_unique<OpenGLVertexBuffer>(desc, m_DebugLayer); }

    std::unique_ptr<IIndexBuffer> OpenGLFactory::CreateIndexBuffer(const BufferDescription& desc)
    { return std::make_unique<OpenGLIndexBuffer>(desc, m_DebugLayer); }

    std::unique_ptr<IUniformBuffer> OpenGLFactory::CreateUniformBuffer(const BufferDescription& desc)
    { return std::make_unique<OpenGLUniformBuffer>(desc, m_DebugLayer); }

    std::unique_ptr<IStorageBuffer> OpenGLFactory::CreateStorageBuffer(const BufferDescription& desc)
    { return std::make_unique<OpenGLStorageBuffer>(desc, m_DebugLayer); }

    std::unique_ptr<ITexture2D> OpenGLFactory::CreateTexture2D(const TextureDescription& desc)
    { return std::make_unique<OpenGLTexture2D>(desc, m_DebugLayer); }

    std::unique_ptr<ITextureCube> OpenGLFactory::CreateTextureCube(const TextureDescription& desc)
    { return std::make_unique<OpenGLTextureCube>(desc, m_DebugLayer); }

    std::unique_ptr<ITextureArray> OpenGLFactory::CreateTextureArray(const TextureDescription& desc)
    { return std::make_unique<OpenGLTextureArray>(desc, m_DebugLayer); }

    std::unique_ptr<ISampler> OpenGLFactory::CreateSampler(const SamplerDescription& desc)
    { return std::make_unique<OpenGLSampler>(desc, m_DebugLayer); }

    std::unique_ptr<IShaderModule> OpenGLFactory::CreateShaderModule(const ShaderStageDescription& desc)
    { return std::make_unique<OpenGLShaderModule>(desc, m_DebugLayer); }

    std::unique_ptr<IShader> OpenGLFactory::CreateShader(const ShaderDescription& desc)
    { return std::make_unique<OpenGLShader>(desc, m_DebugLayer); }

    std::unique_ptr<IPipelineLayout> OpenGLFactory::CreatePipelineLayout(const PipelineLayoutDescription& desc)
    { return std::make_unique<OpenGLPipelineLayout>(desc, m_DebugLayer); }

    std::unique_ptr<IGraphicsPipeline> OpenGLFactory::CreateGraphicsPipeline(const GraphicsPipelineDescription& desc)
    { return std::make_unique<OpenGLGraphicsPipeline>(desc, m_DebugLayer); }

    std::unique_ptr<IComputePipeline> OpenGLFactory::CreateComputePipeline(const ComputePipelineDescription& /*desc*/)
    {
        // Compute pipelines are prepared for future phases.
        return nullptr;
    }

    std::unique_ptr<IRenderPass> OpenGLFactory::CreateRenderPass(const RenderPassDescription& desc)
    { return std::make_unique<OpenGLRenderPass>(desc, m_DebugLayer); }

    std::unique_ptr<IFramebuffer> OpenGLFactory::CreateFramebuffer(const FramebufferDescription& desc,
                                                                    IRenderPass* renderPass)
    {
        return std::make_unique<OpenGLFramebuffer>(desc,
            static_cast<OpenGLRenderPass*>(renderPass), m_DebugLayer);
    }

    std::unique_ptr<IVertexArray> OpenGLFactory::CreateVertexArray()
    { return std::make_unique<OpenGLVertexArray>(m_DebugLayer); }

    std::unique_ptr<ICommandBuffer> OpenGLFactory::CreateCommandBuffer(QueueType /*queueType*/)
    { return std::make_unique<OpenGLCommandBuffer>(m_DebugLayer); }

    std::unique_ptr<ISwapChain> OpenGLFactory::CreateSwapChain(const SwapChainDescription& desc,
                                                                ICommandQueue* /*presentQueue*/)
    {
        // The swapchain needs the context — retrieved from the global state.
        // In a production engine this would be passed in explicitly.
        return nullptr; // Created by the device directly.
    }

    std::unique_ptr<IFence> OpenGLFactory::CreateFence(bool signaled)
    { return std::make_unique<OpenGLFence>(signaled, m_DebugLayer); }

    std::unique_ptr<ISemaphore> OpenGLFactory::CreateSemaphore(engine::core::u64 initialValue)
    { return std::make_unique<OpenGLSemaphore>(initialValue, m_DebugLayer); }

    std::unique_ptr<IDescriptorLayout> OpenGLFactory::CreateDescriptorLayout(const DescriptorLayoutDescription& desc)
    { return std::make_unique<OpenGLDescriptorLayout>(desc); }

    std::unique_ptr<IDescriptorPool> OpenGLFactory::CreateDescriptorPool(engine::core::u32 maxSets,
                                                                          const std::vector<DescriptorType>& allowedTypes)
    { return std::make_unique<OpenGLDescriptorPool>(maxSets, allowedTypes); }

} // namespace engine::opengl
