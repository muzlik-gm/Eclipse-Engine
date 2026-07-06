// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Enums/GraphicsEnums.h
// Complete graphics enumerations for the Rendering Hardware Interface.
//
// Every graphics-API-agnostic enum lives here.  No OpenGL / Vulkan / D3D /
// Metal symbols may appear in this file or in any other public RHI header.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::f32;

    // ========================================================================
    // GraphicsBackend — selectable graphics API backends.
    // ========================================================================
    enum class GraphicsBackend : u32
    {
        None      = 0,
        OpenGL    = 1,
        Vulkan    = 2,
        Direct3D12 = 3,
        Metal     = 4,
        WebGPU    = 5
    };

    [[nodiscard]] constexpr const char* GraphicsBackendToString(GraphicsBackend backend) noexcept
    {
        switch (backend)
        {
            case GraphicsBackend::OpenGL:     return "OpenGL";
            case GraphicsBackend::Vulkan:     return "Vulkan";
            case GraphicsBackend::Direct3D12: return "Direct3D12";
            case GraphicsBackend::Metal:      return "Metal";
            case GraphicsBackend::WebGPU:     return "WebGPU";
            case GraphicsBackend::None:       break;
        }
        return "None";
    }

    // ========================================================================
    // GraphicsAPI — major API family (used for capability queries).
    // ========================================================================
    enum class GraphicsAPI : u32
    {
        None     = 0,
        Desktop  = 1,   // OpenGL / Vulkan / D3D12
        Mobile   = 2,   // Metal / GLES
        Web      = 3    // WebGPU
    };

    // ========================================================================
    // GraphicsVendor — GPU vendor identification.
    // ========================================================================
    enum class GraphicsVendor : u32
    {
        Unknown = 0,
        NVIDIA  = 1,
        AMD     = 2,
        Intel   = 3,
        Apple   = 4,
        ARM     = 5,
        Qualcomm = 6,
        Imagination = 7,
        Mesa    = 8   // Software / open-source drivers
    };

    [[nodiscard]] constexpr const char* GraphicsVendorToString(GraphicsVendor vendor) noexcept
    {
        switch (vendor)
        {
            case GraphicsVendor::NVIDIA:      return "NVIDIA";
            case GraphicsVendor::AMD:         return "AMD";
            case GraphicsVendor::Intel:       return "Intel";
            case GraphicsVendor::Apple:       return "Apple";
            case GraphicsVendor::ARM:         return "ARM";
            case GraphicsVendor::Qualcomm:    return "Qualcomm";
            case GraphicsVendor::Imagination: return "Imagination";
            case GraphicsVendor::Mesa:        return "Mesa";
            case GraphicsVendor::Unknown:     break;
        }
        return "Unknown";
    }

    // ========================================================================
    // GraphicsFormat — surface / buffer element formats.
    // ========================================================================
    enum class GraphicsFormat : u32
    {
        Undefined = 0,

        // -- Color formats (8-bit) -------------------------------------------
        R8_UNorm,
        R8_SNorm,
        R8_UInt,
        R8_SInt,
        RG8_UNorm,
        RG8_SNorm,
        RG8_UInt,
        RG8_SInt,
        RGBA8_UNorm,
        RGBA8_SNorm,
        RGBA8_UInt,
        RGBA8_SInt,
        RGBA8_sRGB,

        BGRA8_UNorm,
        BGRA8_sRGB,

        // -- Color formats (16-bit) ------------------------------------------
        R16_UNorm,
        R16_SNorm,
        R16_UInt,
        R16_SInt,
        R16_Float,
        RG16_UNorm,
        RG16_Float,
        RGBA16_UNorm,
        RGBA16_Float,

        // -- Color formats (32-bit) ------------------------------------------
        R32_UInt,
        R32_SInt,
        R32_Float,
        RG32_UInt,
        RG32_SInt,
        RG32_Float,
        RGB32_UInt,
        RGB32_SInt,
        RGB32_Float,
        RGBA32_UInt,
        RGBA32_SInt,
        RGBA32_Float,

        // -- Packed formats --------------------------------------------------
        R5G6B5_UNorm,
        RGBA4_UNorm,
        RGB5A1_UNorm,
        RGB10A2_UNorm,
        RG11B10_Float,
        RGB9E5_Float,

        // -- Depth / stencil formats -----------------------------------------
        D16_UNorm,
        D24_UNorm_S8_UInt,
        D32_Float,
        D32_Float_S8_UInt,
        S8_UInt,

        // -- Compressed formats (BCn / ETC / ASTC) ---------------------------
        BC1_RGBA_UNorm,
        BC1_RGBA_sRGB,
        BC2_RGBA_UNorm,
        BC2_RGBA_sRGB,
        BC3_RGBA_UNorm,
        BC3_RGBA_sRGB,
        BC4_R_UNorm,
        BC4_R_SNorm,
        BC5_RG_UNorm,
        BC5_RG_SNorm,
        BC6H_RGB_UFloat,
        BC6H_RGB_SFloat,
        BC7_RGBA_UNorm,
        BC7_RGBA_sRGB
    };

    [[nodiscard]] constexpr u32 GraphicsFormatSize(GraphicsFormat format) noexcept
    {
        switch (format)
        {
            case GraphicsFormat::R8_UNorm:
            case GraphicsFormat::R8_SNorm:
            case GraphicsFormat::R8_UInt:
            case GraphicsFormat::R8_SInt:
            case GraphicsFormat::S8_UInt:
                return 1;

            case GraphicsFormat::RG8_UNorm:
            case GraphicsFormat::RG8_SNorm:
            case GraphicsFormat::RG8_UInt:
            case GraphicsFormat::RG8_SInt:
            case GraphicsFormat::R16_UNorm:
            case GraphicsFormat::R16_SNorm:
            case GraphicsFormat::R16_UInt:
            case GraphicsFormat::R16_SInt:
            case GraphicsFormat::R16_Float:
            case GraphicsFormat::D16_UNorm:
            case GraphicsFormat::R5G6B5_UNorm:
            case GraphicsFormat::RGBA4_UNorm:
            case GraphicsFormat::RGB5A1_UNorm:
                return 2;

            case GraphicsFormat::RGBA8_UNorm:
            case GraphicsFormat::RGBA8_SNorm:
            case GraphicsFormat::RGBA8_UInt:
            case GraphicsFormat::RGBA8_SInt:
            case GraphicsFormat::RGBA8_sRGB:
            case GraphicsFormat::BGRA8_UNorm:
            case GraphicsFormat::BGRA8_sRGB:
            case GraphicsFormat::RG16_UNorm:
            case GraphicsFormat::RG16_Float:
            case GraphicsFormat::R32_UInt:
            case GraphicsFormat::R32_SInt:
            case GraphicsFormat::R32_Float:
            case GraphicsFormat::D24_UNorm_S8_UInt:
            case GraphicsFormat::D32_Float:
            case GraphicsFormat::RGB10A2_UNorm:
            case GraphicsFormat::RG11B10_Float:
            case GraphicsFormat::RGB9E5_Float:
                return 4;

            case GraphicsFormat::RGBA16_UNorm:
            case GraphicsFormat::RGBA16_Float:
            case GraphicsFormat::RG32_UInt:
            case GraphicsFormat::RG32_SInt:
            case GraphicsFormat::RG32_Float:
            case GraphicsFormat::D32_Float_S8_UInt:
                return 8;

            case GraphicsFormat::RGB32_UInt:
            case GraphicsFormat::RGB32_SInt:
            case GraphicsFormat::RGB32_Float:
            case GraphicsFormat::RGBA32_UInt:
            case GraphicsFormat::RGBA32_SInt:
            case GraphicsFormat::RGBA32_Float:
                return 12;

            case GraphicsFormat::BC1_RGBA_UNorm:
            case GraphicsFormat::BC1_RGBA_sRGB:
            case GraphicsFormat::BC4_R_UNorm:
            case GraphicsFormat::BC4_R_SNorm:
                return 8; // per 4x4 block

            case GraphicsFormat::BC2_RGBA_UNorm:
            case GraphicsFormat::BC2_RGBA_sRGB:
            case GraphicsFormat::BC3_RGBA_UNorm:
            case GraphicsFormat::BC3_RGBA_sRGB:
            case GraphicsFormat::BC5_RG_UNorm:
            case GraphicsFormat::BC5_RG_SNorm:
            case GraphicsFormat::BC6H_RGB_UFloat:
            case GraphicsFormat::BC6H_RGB_SFloat:
            case GraphicsFormat::BC7_RGBA_UNorm:
            case GraphicsFormat::BC7_RGBA_sRGB:
                return 16; // per 4x4 block

            case GraphicsFormat::Undefined:
            default:
                return 0;
        }
    }

    [[nodiscard]] constexpr bool IsDepthFormat(GraphicsFormat format) noexcept
    {
        return format == GraphicsFormat::D16_UNorm
            || format == GraphicsFormat::D24_UNorm_S8_UInt
            || format == GraphicsFormat::D32_Float
            || format == GraphicsFormat::D32_Float_S8_UInt;
    }

    [[nodiscard]] constexpr bool IsStencilFormat(GraphicsFormat format) noexcept
    {
        return format == GraphicsFormat::D24_UNorm_S8_UInt
            || format == GraphicsFormat::D32_Float_S8_UInt
            || format == GraphicsFormat::S8_UInt;
    }

    [[nodiscard]] constexpr bool IsCompressedFormat(GraphicsFormat format) noexcept
    {
        return format >= GraphicsFormat::BC1_RGBA_UNorm
            && format <= GraphicsFormat::BC7_RGBA_sRGB;
    }

    [[nodiscard]] constexpr bool IsSRGBFormat(GraphicsFormat format) noexcept
    {
        return format == GraphicsFormat::RGBA8_sRGB
            || format == GraphicsFormat::BGRA8_sRGB
            || format == GraphicsFormat::BC1_RGBA_sRGB
            || format == GraphicsFormat::BC2_RGBA_sRGB
            || format == GraphicsFormat::BC3_RGBA_sRGB
            || format == GraphicsFormat::BC7_RGBA_sRGB;
    }

    // ========================================================================
    // PrimitiveTopology — how the vertex buffer is interpreted.
    // ========================================================================
    enum class PrimitiveTopology : u32
    {
        PointList       = 0,
        LineList        = 1,
        LineStrip       = 2,
        LineLoop        = 3,
        TriangleList    = 4,
        TriangleStrip   = 5,
        TriangleFan     = 6,
        LineListAdjacency       = 7,
        LineStripAdjacency      = 8,
        TriangleListAdjacency   = 9,
        TriangleStripAdjacency  = 10,
        PatchList       = 11  // for tessellation
    };

    // ========================================================================
    // BufferUsage — how a buffer will be used by the GPU.
    // ========================================================================
    enum class BufferUsage : u32
    {
        None            = 0,
        VertexBuffer    = (1u << 0),
        IndexBuffer     = (1u << 1),
        UniformBuffer   = (1u << 2),
        StorageBuffer   = (1u << 3),
        IndirectBuffer  = (1u << 4),
        TransferSource  = (1u << 5),
        TransferDest    = (1u << 6),
        Persistent      = (1u << 7),  // persistent mapping
        Dynamic         = (1u << 8),  // frequently updated
        Immutable       = (1u << 9)   // never updated after creation
    };

    // ========================================================================
    // TextureUsage — how a texture will be used by the GPU.
    // ========================================================================
    enum class TextureUsage : u32
    {
        None            = 0,
        Sampled         = (1u << 0),  // shader reads
        Storage         = (1u << 1),  // shader writes (image load/store)
        RenderTarget    = (1u << 2),  // color attachment
        DepthStencil    = (1u << 3),  // depth/stencil attachment
        TransferSource  = (1u << 4),
        TransferDest    = (1u << 5),
        CubeMap         = (1u << 6),
        GenerateMips    = (1u << 7)
    };

    // ========================================================================
    // TextureType — dimensionality of a texture.
    // ========================================================================
    enum class TextureType : u32
    {
        None        = 0,
        Texture1D   = 1,
        Texture2D   = 2,
        Texture3D   = 3,
        TextureCube = 4,
        Texture1DArray = 5,
        Texture2DArray = 6,
        TextureCubeArray = 7
    };

    // ========================================================================
    // TextureFormat — alias for GraphicsFormat restricted to texture-usable
    // formats.  Kept as a distinct enum per the Phase 3 spec.
    // ========================================================================
    using TextureFormat = GraphicsFormat;

    // ========================================================================
    // ShaderStage — pipeline stage a shader belongs to.
    // ========================================================================
    enum class ShaderStage : u32
    {
        None            = 0,
        Vertex          = (1u << 0),
        Fragment        = (1u << 1),
        Geometry        = (1u << 2),
        TessControl     = (1u << 3),
        TessEvaluation  = (1u << 4),
        Compute         = (1u << 5),
        Task            = (1u << 6),  // mesh shader pipeline (future)
        Mesh            = (1u << 7),  // mesh shader pipeline (future)
        RayGeneration   = (1u << 8),  // ray tracing (future)
        AnyHit          = (1u << 9),
        ClosestHit      = (1u << 10),
        Miss            = (1u << 11),
        Intersection    = (1u << 12),
        Callable        = (1u << 13),
        AllGraphics     = 0x0000003Fu,
        All             = 0xFFFFFFFFu
    };

    // ========================================================================
    // BlendFactor — source / destination blend coefficient.
    // ========================================================================
    enum class BlendFactor : u32
    {
        Zero                = 0,
        One                 = 1,
        SrcColor            = 2,
        OneMinusSrcColor    = 3,
        DstColor            = 4,
        OneMinusDstColor    = 5,
        SrcAlpha            = 6,
        OneMinusSrcAlpha    = 7,
        DstAlpha            = 8,
        OneMinusDstAlpha    = 9,
        ConstantColor       = 10,
        OneMinusConstantColor = 11,
        ConstantAlpha       = 12,
        OneMinusConstantAlpha = 13,
        SrcAlphaSaturate    = 14,
        Src1Color           = 15,
        OneMinusSrc1Color   = 16,
        Src1Alpha           = 17,
        OneMinusSrc1Alpha   = 18
    };

    // ========================================================================
    // BlendOperation — how source and dest are combined.
    // ========================================================================
    enum class BlendOperation : u32
    {
        Add             = 0,
        Subtract        = 1,
        ReverseSubtract = 2,
        Min             = 3,
        Max             = 4
    };

    // ========================================================================
    // CompareOperation — depth / stencil comparison functions.
    // ========================================================================
    enum class CompareOperation : u32
    {
        Never           = 0,
        Less            = 1,
        Equal           = 2,
        LessOrEqual     = 3,
        Greater         = 4,
        NotEqual        = 5,
        GreaterOrEqual  = 6,
        Always          = 7
    };

    // ========================================================================
    // CullMode — which triangle faces are culled.
    // ========================================================================
    enum class CullMode : u32
    {
        None        = 0,
        Front       = 1,
        Back        = 2,
        FrontAndBack = 3
    };

    // ========================================================================
    // FrontFace — vertex winding that defines a front-facing triangle.
    // ========================================================================
    enum class FrontFace : u32
    {
        CounterClockwise = 0,
        Clockwise        = 1
    };

    // ========================================================================
    // PolygonMode — how polygons are rasterized.
    // ========================================================================
    enum class PolygonMode : u32
    {
        Fill        = 0,
        Line        = 1,
        Point       = 2
    };

    // FillMode — alias kept for spec completeness.
    using FillMode = PolygonMode;

    // ========================================================================
    // DepthMode — alias for CompareOperation when used for depth tests.
    // ========================================================================
    using DepthMode = CompareOperation;

    // ========================================================================
    // StencilOperation — actions performed during stencil testing.
    // ========================================================================
    enum class StencilOperation : u32
    {
        Keep                = 0,
        Zero                = 1,
        Replace             = 2,
        IncrementClamp      = 3,
        DecrementClamp      = 4,
        Invert              = 5,
        IncrementWrap       = 6,
        DecrementWrap       = 7
    };

    // ========================================================================
    // FilterMode — minification / magnification / mip filters.
    // ========================================================================
    enum class FilterMode : u32
    {
        Nearest = 0,
        Linear  = 1
    };

    // ========================================================================
    // AddressMode — texture wrap behavior outside [0,1] UV range.
    // ========================================================================
    enum class AddressMode : u32
    {
        Repeat              = 0,
        MirroredRepeat      = 1,
        ClampToEdge         = 2,
        ClampToBorder       = 3,
        MirrorClampToEdge   = 4
    };

    // ========================================================================
    // AttachmentLoadOperation — what to do with an attachment at
    // BeginRenderPass time.
    // ========================================================================
    enum class AttachmentLoadOperation : u32
    {
        Load        = 0,  // preserve existing contents
        Clear       = 1,  // clear to a specified value
        DontCare    = 2   // contents may be discarded
    };

    // ========================================================================
    // AttachmentStoreOperation — what to do with an attachment at
    // EndRenderPass time.
    // ========================================================================
    enum class AttachmentStoreOperation : u32
    {
        Store       = 0,  // write contents back to memory
        DontCare    = 1,  // contents may be discarded
        None        = 2   // no-op (for input attachments)
    };

    // ========================================================================
    // ResourceState — tracks the current usage of a GPU resource for
    // barrier / transition operations.
    // ========================================================================
    enum class ResourceState : u32
    {
        Undefined               = 0,
        General                 = 1,
        VertexBuffer            = 2,
        IndexBuffer             = 3,
        UniformBuffer           = 4,
        StorageBuffer           = 5,
        TransferSource          = 6,
        TransferDest            = 7,
        Sampled                 = 8,
        StorageImage            = 9,
        ColorAttachment         = 10,
        DepthStencilAttachment  = 11,
        DepthRead               = 12,
        Present                 = 13,
        Common                  = 14
    };

    // ========================================================================
    // PresentMode — how the swapchain presents frames to the screen.
    // ========================================================================
    enum class PresentMode : u32
    {
        Immediate       = 0,  // no vsync, may tear
        Mailbox         = 1,  // vsync, replace pending frame
        FIFO            = 2,  // vsync (default)
        FIFORelaxed     = 3   // vsync, but allows tearing if late
    };

    // ========================================================================
    // CommandType — categorizes recorded commands for instrumentation.
    // ========================================================================
    enum class CommandType : u32
    {
        None            = 0,
        BeginFrame,
        EndFrame,
        BeginRenderPass,
        EndRenderPass,
        BindPipeline,
        BindVertexBuffer,
        BindIndexBuffer,
        BindTexture,
        BindSampler,
        BindDescriptorSet,
        BindUniformBuffer,
        PushConstants,
        SetViewport,
        SetScissor,
        ClearColor,
        ClearDepth,
        ClearStencil,
        Draw,
        DrawIndexed,
        DrawInstanced,
        DrawIndexedIndirect,
        Dispatch,
        CopyBuffer,
        CopyTexture,
        CopyBufferToTexture,
        CopyTextureToBuffer,
        GenerateMipmaps,
        TransitionResource,
        Present
    };

    // ========================================================================
    // QueueType — categorizes hardware command queues.
    // ========================================================================
    enum class QueueType : u32
    {
        Graphics    = 0,
        Compute     = 1,
        Transfer    = 2,
        Sparse      = 3
    };

    // ========================================================================
    // IndexType — element size of an index buffer.
    // ========================================================================
    enum class IndexType : u32
    {
        UInt16  = 0,
        UInt32  = 1
    };

    // ========================================================================
    // MapFlags — options for buffer mapping.
    // ========================================================================
    enum class MapFlags : u32
    {
        None            = 0,
        Read            = (1u << 0),
        Write           = (1u << 1),
        ReadWrite       = Read | Write,
        Persistent      = (1u << 2),
        Coherent        = (1u << 3),
        FlushExplicit   = (1u << 4),
        Unsynchronized  = (1u << 5)
    };

    // ========================================================================
    // ShaderLanguage — source language a shader is written in.
    // ========================================================================
    enum class ShaderLanguage : u32
    {
        None        = 0,
        GLSL        = 1,
        HLSL        = 2,
        MSL         = 3,
        SPIRV       = 4,
        WGSL        = 5
    };

    // ========================================================================
    // DescriptorType — what kind of resource a descriptor binding holds.
    // ========================================================================
    enum class DescriptorType : u32
    {
        None                = 0,
        Sampler             = 1,
        SampledImage        = 2,
        StorageImage        = 3,
        UniformBuffer       = 4,
        StorageBuffer       = 5,
        UniformTexelBuffer  = 6,
        StorageTexelBuffer  = 7,
        InputAttachment     = 8
    };

    // ========================================================================
    // Bitwise operators for flag enums.
    // ========================================================================
    [[nodiscard]] constexpr BufferUsage  operator|(BufferUsage  a, BufferUsage  b) noexcept
    { return static_cast<BufferUsage>(static_cast<u32>(a) | static_cast<u32>(b)); }
    [[nodiscard]] constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) noexcept
    { return static_cast<TextureUsage>(static_cast<u32>(a) | static_cast<u32>(b)); }
    [[nodiscard]] constexpr ShaderStage  operator|(ShaderStage  a, ShaderStage  b) noexcept
    { return static_cast<ShaderStage>(static_cast<u32>(a) | static_cast<u32>(b)); }
    [[nodiscard]] constexpr MapFlags     operator|(MapFlags     a, MapFlags     b) noexcept
    { return static_cast<MapFlags>(static_cast<u32>(a) | static_cast<u32>(b)); }

    [[nodiscard]] constexpr u32 operator&(BufferUsage a, BufferUsage b) noexcept
    { return static_cast<u32>(a) & static_cast<u32>(b); }
    [[nodiscard]] constexpr u32 operator&(TextureUsage a, TextureUsage b) noexcept
    { return static_cast<u32>(a) & static_cast<u32>(b); }
    [[nodiscard]] constexpr u32 operator&(ShaderStage a, ShaderStage b) noexcept
    { return static_cast<u32>(a) & static_cast<u32>(b); }
    [[nodiscard]] constexpr u32 operator&(MapFlags a, MapFlags b) noexcept
    { return static_cast<u32>(a) & static_cast<u32>(b); }

} // namespace engine::rhi
