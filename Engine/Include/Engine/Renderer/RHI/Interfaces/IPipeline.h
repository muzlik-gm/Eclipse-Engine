// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/IPipeline.h
// Abstract interfaces for pipelines, pipeline layouts, render passes,
// framebuffers, and render targets.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Interfaces/IShader.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"

namespace engine::rhi {

    using engine::core::u32;

    // ========================================================================
    // Forward declarations
    // ========================================================================

    class IRenderPass;

    // ========================================================================
    // IPipelineLayout
    // ========================================================================

    /// @brief Describes the resource layout of a pipeline — descriptor set
    ///        layouts plus push constant ranges.  Created from a
    ///        PipelineLayoutDescription and shared across pipelines that
    ///        use the same layout.
    class IPipelineLayout : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const PipelineLayoutDescription& GetDescription() const noexcept = 0;

        /// @brief Returns the number of descriptor set layouts.
        [[nodiscard]] virtual u32 GetSetLayoutCount() const noexcept = 0;

        /// @brief Returns the total size of all push constant ranges.
        [[nodiscard]] virtual u32 GetPushConstantsSize() const noexcept = 0;
    };

    // ========================================================================
    // IPipeline — base interface for graphics and compute pipelines.
    // ========================================================================

    class IPipeline : public IGraphicsObject
    {
    public:
        /// @brief Returns the pipeline layout used by this pipeline.
        [[nodiscard]] virtual IPipelineLayout* GetLayout() const noexcept = 0;

        /// @brief Returns the shader program bound to this pipeline.
        [[nodiscard]] virtual IShader* GetShader() const noexcept = 0;

        /// @brief Returns true if the pipeline was successfully compiled
        ///        and linked.
        [[nodiscard]] virtual bool IsValid() const noexcept = 0;

        /// @brief Returns a human-readable validation / link log.
        [[nodiscard]] virtual const std::string& GetValidationLog() const noexcept = 0;
    };

    // ========================================================================
    // IGraphicsPipeline
    // ========================================================================

    /// @brief A fully-baked graphics pipeline — vertex layout, shader
    ///        program, rasterizer / blend / depth-stencil state, etc.
    class IGraphicsPipeline : public IPipeline
    {
    public:
        [[nodiscard]] virtual const GraphicsPipelineDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual PrimitiveTopology          GetTopology() const noexcept = 0;
        [[nodiscard]] virtual const RasterizerStateDescription& GetRasterizerState() const noexcept = 0;
        [[nodiscard]] virtual const BlendStateDescription&      GetBlendState() const noexcept = 0;
        [[nodiscard]] virtual const DepthStencilDescription&    GetDepthStencilState() const noexcept = 0;
        [[nodiscard]] virtual const VertexLayoutDescription&    GetVertexLayout() const noexcept = 0;

        /// @brief Returns the render pass this pipeline is compatible with.
        [[nodiscard]] virtual IRenderPass* GetRenderPass() const noexcept = 0;
    };

    // ========================================================================
    // IComputePipeline
    // ========================================================================

    /// @brief A compute pipeline — a compute shader plus its layout.
    class IComputePipeline : public IPipeline
    {
    public:
        [[nodiscard]] virtual const ComputePipelineDescription& GetDescription() const noexcept = 0;
    };

    // ========================================================================
    // IRenderPass
    // ========================================================================

    /// @brief Describes the set of attachments, load/store operations,
    ///        and subpass dependencies that framebuffers must conform to.
    class IRenderPass : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const RenderPassDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual u32 GetColorAttachmentCount() const noexcept = 0;
        [[nodiscard]] virtual bool HasDepthStencilAttachment() const noexcept = 0;

        [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
        [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
    };

    // ========================================================================
    // IFramebuffer
    // ========================================================================

    /// @brief A concrete framebuffer — texture views bound to a render
    ///        pass.  Created from a FramebufferDescription plus an
    ///        IRenderPass.
    class IFramebuffer : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const FramebufferDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual IRenderPass* GetRenderPass() const noexcept = 0;

        [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
        [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
        [[nodiscard]] virtual u32 GetLayers() const noexcept = 0;

        /// @brief Returns the color attachment at @p index, or nullptr.
        [[nodiscard]] virtual ITexture* GetColorAttachment(u32 index) const noexcept = 0;

        /// @brief Returns the depth/stencil attachment, or nullptr.
        [[nodiscard]] virtual ITexture* GetDepthStencilAttachment() const noexcept = 0;
    };

    // ========================================================================
    // IRenderTarget
    // ========================================================================

    /// @brief A render target — a lightweight wrapper around a texture
    ///        that is usable as a color or depth-stencil attachment.
    ///        Render targets are the high-level "you can draw to this"
    ///        abstraction used by the renderer.
    class IRenderTarget : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual ITexture* GetTexture() const noexcept = 0;
        [[nodiscard]] virtual GraphicsFormat GetFormat() const noexcept = 0;
        [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
        [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
        [[nodiscard]] virtual bool IsDepthStencil() const noexcept = 0;
    };

} // namespace engine::rhi
