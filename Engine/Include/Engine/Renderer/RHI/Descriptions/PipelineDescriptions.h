// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h
// Immutable descriptions for graphics / compute pipelines and all of their
// fixed-function state.  No backend-specific members.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;
    using engine::core::usize;

    // ========================================================================
    // VertexAttributeDescription
    // ========================================================================

    /// @brief Describes a single attribute inside a vertex buffer binding.
    struct VertexAttributeDescription
    {
        /// Logical location / slot the attribute binds to in the shader.
        u32 Location{0};

        /// Which vertex buffer binding provides this attribute's data.
        u32 Binding{0};

        /// Element format of the attribute.
        GraphicsFormat Format{GraphicsFormat::RGBA32_Float};

        /// Byte offset of the attribute within a vertex.
        u32 Offset{0};

        /// True if the attribute should be interpreted as normalized
        /// (e.g. unorm8 → float [0,1]).
        bool Normalized{false};
    };

    // ========================================================================
    // VertexBindingDescription
    // ========================================================================

    /// @brief Describes how a single vertex buffer binding is laid out.
    struct VertexBindingDescription
    {
        /// Binding index this description applies to.
        u32 Binding{0};

        /// Stride between consecutive vertices, in bytes.
        u32 Stride{0};

        /// True if the binding is instanced (advance once per instance).
        bool Instanced{false};
    };

    // ========================================================================
    // VertexLayoutDescription
    // ========================================================================

    /// @brief Describes the full vertex input layout of a pipeline.
    struct VertexLayoutDescription
    {
        std::vector<VertexBindingDescription>   Bindings;
        std::vector<VertexAttributeDescription> Attributes;
    };

    // ========================================================================
    // RasterizerStateDescription
    // ========================================================================

    /// @brief Fixed-function rasterization state.
    struct RasterizerStateDescription
    {
        bool              DepthClampEnable{false};
        bool              RasterizerDiscardEnable{false};
        PolygonMode       PolygonModeValue{engine::rhi::PolygonMode::Fill};
        CullMode          CullModeValue{engine::rhi::CullMode::Back};
        FrontFace         FrontFaceValue{engine::rhi::FrontFace::CounterClockwise};
        bool              DepthBiasEnable{false};
        f32               DepthBiasConstantFactor{0.0f};
        f32               DepthBiasClamp{0.0f};
        f32               DepthBiasSlopeFactor{0.0f};
        f32               LineWidth{1.0f};
    };

    // ========================================================================
    // BlendAttachmentState
    // ========================================================================

    /// @brief Per-target blend state.  A pipeline may have one of these per
    ///        color attachment.
    struct BlendAttachmentState
    {
        bool           BlendEnable{false};
        BlendFactor    SrcColorBlendFactor{BlendFactor::One};
        BlendFactor    DstColorBlendFactor{BlendFactor::Zero};
        BlendOperation ColorBlendOp{BlendOperation::Add};
        BlendFactor    SrcAlphaBlendFactor{BlendFactor::One};
        BlendFactor    DstAlphaBlendFactor{BlendFactor::Zero};
        BlendOperation AlphaBlendOp{BlendOperation::Add};
        /// Color write mask (R | G | B | A bits).
        u32            ColorWriteMask{0xF};
    };

    // ========================================================================
    // BlendStateDescription
    // ========================================================================

    /// @brief Full blend state — per-target states plus a global blend
    ///        constant color.
    struct BlendStateDescription
    {
        bool                                 LogicOpEnable{false};
        std::array<f32, 4>                   BlendConstants{0.0f, 0.0f, 0.0f, 0.0f};
        std::vector<BlendAttachmentState>    Attachments;
    };

    // ========================================================================
    // StencilOpState
    // ========================================================================

    /// @brief Per-face stencil state.
    struct StencilOpState
    {
        StencilOperation  FailOp{StencilOperation::Keep};
        StencilOperation  PassOp{StencilOperation::Keep};
        StencilOperation  DepthFailOp{StencilOperation::Keep};
        CompareOperation  CompareOp{CompareOperation::Always};
        u32               CompareMask{0xFF};
        u32               WriteMask{0xFF};
        u32               Reference{0};
    };

    // ========================================================================
    // DepthStencilDescription
    // ========================================================================

    /// @brief Full depth / stencil state.
    struct DepthStencilDescription
    {
        bool              DepthTestEnable{true};
        bool              DepthWriteEnable{true};
        CompareOperation  DepthCompareOp{CompareOperation::Less};
        bool              DepthBoundsTestEnable{false};
        bool              StencilTestEnable{false};
        StencilOpState    FrontStencil{};
        StencilOpState    BackStencil{};
        f32               MinDepthBounds{0.0f};
        f32               MaxDepthBounds{1.0f};
    };

    // ========================================================================
    // ViewportDescription
    // ========================================================================

    struct ViewportDescription
    {
        f32 X{0.0f};
        f32 Y{0.0f};
        f32 Width{0.0f};
        f32 Height{0.0f};
        f32 MinDepth{0.0f};
        f32 MaxDepth{1.0f};
    };

    // ========================================================================
    // ScissorDescription
    // ========================================================================

    struct ScissorDescription
    {
        i32 X{0};
        i32 Y{0};
        u32 Width{0};
        u32 Height{0};
    };

    // ========================================================================
    // ShaderStageDescription
    // ========================================================================

    /// @brief Describes a single shader stage attached to a pipeline.
    struct ShaderStageDescription
    {
        ShaderStage      Stage{ShaderStage::None};
        ShaderLanguage   Language{ShaderLanguage::GLSL};

        /// Source code (GLSL / HLSL / MSL / WGSL) as a UTF-8 string.
        std::string      Source;

        /// Optional entry point name (e.g. "main", "vs_main").
        std::string      EntryPoint{"main"};

        /// Optional file path the source was loaded from (for diagnostics
        /// and hot-reload).
        std::string      FilePath;

        /// Optional SPIR-V bytecode (used when Language == SPIRV).
        std::vector<u32> Bytecode;
    };

    // ========================================================================
    // ShaderDescription
    // ========================================================================

    /// @brief Describes a complete shader program (one or more stages).
    struct ShaderDescription
    {
        std::string                     Name;
        std::vector<ShaderStageDescription> Stages;
        bool DebugNamed{true};
    };

    // ========================================================================
    // DescriptorBindingDescription
    // ========================================================================

    /// @brief Describes a single binding inside a descriptor set layout.
    struct DescriptorBindingDescription
    {
        u32             Binding{0};
        DescriptorType  Type{DescriptorType::None};
        u32             Count{1};
        ShaderStage     Stages{ShaderStage::All};
    };

    // ========================================================================
    // DescriptorLayoutDescription
    // ========================================================================

    /// @brief Describes the layout of a descriptor set.
    struct DescriptorLayoutDescription
    {
        std::vector<DescriptorBindingDescription> Bindings;
    };

    // ========================================================================
    // PushConstantRange
    // ========================================================================

    /// @brief Describes a range of push constants visible to a stage.
    struct PushConstantRange
    {
        ShaderStage Stages{ShaderStage::None};
        u32         Offset{0};
        u32         Size{0};
    };

    // ========================================================================
    // PipelineLayoutDescription
    // ========================================================================

    /// @brief Describes the resource layout of a pipeline — descriptor set
    ///        layouts plus push constant ranges.
    struct PipelineLayoutDescription
    {
        std::vector<DescriptorLayoutDescription> SetLayouts;
        std::vector<PushConstantRange>           PushConstants;
    };

    // ========================================================================
    // GraphicsPipelineDescription
    // ========================================================================

    /// @brief Immutable description of a graphics pipeline.
    ///
    /// Every fixed-function state block is a value member so the entire
    /// description can be copied freely.  The shader program is referenced
    /// by an opaque handle that the backend resolves to its concrete
    /// IShader implementation.
    struct GraphicsPipelineDescription
    {
        std::string                    Name;
        ShaderDescription              Shader;
        PipelineLayoutDescription      Layout;
        VertexLayoutDescription        VertexLayout;
        PrimitiveTopology              TopologyValue{engine::rhi::PrimitiveTopology::TriangleList};
        RasterizerStateDescription     RasterizerState;
        BlendStateDescription          BlendState;
        DepthStencilDescription        DepthStencilState;

        /// Render pass this pipeline is compatible with (opaque handle).
        const void*                    RenderPassHandle{nullptr};

        u32                            Subpass{0};

        /// Dynamic state flags — which state is set at draw time instead
        /// of being baked into the pipeline.
        bool                           DynamicViewport{true};
        bool                           DynamicScissor{true};
        bool                           DynamicLineWidth{false};
        bool                           DynamicDepthBias{false};
        bool                           DynamicBlendConstants{false};
        bool                           DynamicStencilReference{false};

        bool DebugNamed{true};
    };

    // ========================================================================
    // ComputePipelineDescription
    // ========================================================================

    /// @brief Immutable description of a compute pipeline.
    struct ComputePipelineDescription
    {
        std::string               Name;
        ShaderStageDescription    ComputeShader;
        PipelineLayoutDescription Layout;
        bool DebugNamed{true};
    };

    // ========================================================================
    // SwapChainDescription
    // ========================================================================

    /// @brief Immutable description of a swapchain.
    struct SwapChainDescription
    {
        u32           Width{0};
        u32           Height{0};
        u32           BufferCount{2};   // double-buffering by default
        GraphicsFormat Format{GraphicsFormat::RGBA8_UNorm};
        PresentMode   PresentModeValue{engine::rhi::PresentMode::FIFO};
        bool          VSync{true};
        /// Window handle (platform-specific).  Opaque to the RHI.
        void*         WindowHandle{nullptr};
    };

} // namespace engine::rhi
