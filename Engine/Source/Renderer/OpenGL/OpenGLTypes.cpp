// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLTypes.cpp
// Implementation of RHI enum → OpenGL constant conversions.
// ============================================================================
#include "OpenGLTypes.h"

namespace engine::opengl {

    // ========================================================================
    // Format conversions
    // ========================================================================

    u32 ToGLInternalFormat(GraphicsFormat format) noexcept
    {
        switch (format)
        {
            // 8-bit
            case GraphicsFormat::R8_UNorm:         return GL_R8;
            case GraphicsFormat::R8_SNorm:         return GL_R8_SNORM;
            case GraphicsFormat::R8_UInt:          return GL_R8UI;
            case GraphicsFormat::R8_SInt:          return GL_R8I;
            case GraphicsFormat::RG8_UNorm:        return GL_RG8;
            case GraphicsFormat::RG8_SNorm:        return GL_RG8_SNORM;
            case GraphicsFormat::RG8_UInt:         return GL_RG8UI;
            case GraphicsFormat::RG8_SInt:         return GL_RG8I;
            case GraphicsFormat::RGBA8_UNorm:      return GL_RGBA8;
            case GraphicsFormat::RGBA8_SNorm:      return GL_RGBA8_SNORM;
            case GraphicsFormat::RGBA8_UInt:       return GL_RGBA8UI;
            case GraphicsFormat::RGBA8_SInt:       return GL_RGBA8I;
            case GraphicsFormat::RGBA8_sRGB:       return GL_SRGB8_ALPHA8;
            case GraphicsFormat::BGRA8_UNorm:      return GL_RGBA8; // GL doesn't have BGRA8 internal
            case GraphicsFormat::BGRA8_sRGB:       return GL_SRGB8_ALPHA8;

            // 16-bit
            case GraphicsFormat::R16_UNorm:        return GL_R16;
            case GraphicsFormat::R16_SNorm:        return GL_R16_SNORM;
            case GraphicsFormat::R16_UInt:         return GL_R16UI;
            case GraphicsFormat::R16_SInt:         return GL_R16I;
            case GraphicsFormat::R16_Float:        return GL_R16F;
            case GraphicsFormat::RG16_UNorm:       return GL_RG16;
            case GraphicsFormat::RG16_Float:       return GL_RG16F;
            case GraphicsFormat::RGBA16_UNorm:     return GL_RGBA16;
            case GraphicsFormat::RGBA16_Float:     return GL_RGBA16F;

            // 32-bit
            case GraphicsFormat::R32_UInt:         return GL_R32UI;
            case GraphicsFormat::R32_SInt:         return GL_R32I;
            case GraphicsFormat::R32_Float:        return GL_R32F;
            case GraphicsFormat::RG32_UInt:        return GL_RG32UI;
            case GraphicsFormat::RG32_SInt:        return GL_RG32I;
            case GraphicsFormat::RG32_Float:       return GL_RG32F;
            case GraphicsFormat::RGB32_UInt:       return GL_RGB32UI;
            case GraphicsFormat::RGB32_SInt:       return GL_RGB32I;
            case GraphicsFormat::RGB32_Float:      return GL_RGB32F;
            case GraphicsFormat::RGBA32_UInt:      return GL_RGBA32UI;
            case GraphicsFormat::RGBA32_SInt:      return GL_RGBA32I;
            case GraphicsFormat::RGBA32_Float:     return GL_RGBA32F;

            // Packed
            case GraphicsFormat::R5G6B5_UNorm:     return GL_RGB565;
            case GraphicsFormat::RGBA4_UNorm:      return GL_RGBA4;
            case GraphicsFormat::RGB5A1_UNorm:     return GL_RGB5_A1;
            case GraphicsFormat::RGB10A2_UNorm:    return GL_RGB10_A2;
            case GraphicsFormat::RG11B10_Float:    return GL_R11F_G11F_B10F;
            case GraphicsFormat::RGB9E5_Float:     return GL_RGB9_E5;

            // Depth / stencil
            case GraphicsFormat::D16_UNorm:        return GL_DEPTH_COMPONENT16;
            case GraphicsFormat::D24_UNorm_S8_UInt: return GL_DEPTH24_STENCIL8;
            case GraphicsFormat::D32_Float:        return GL_DEPTH_COMPONENT32F;
            case GraphicsFormat::D32_Float_S8_UInt: return GL_DEPTH32F_STENCIL8;
            case GraphicsFormat::S8_UInt:          return GL_STENCIL_INDEX8;

            // Compressed (BCn)
            case GraphicsFormat::BC1_RGBA_UNorm:   return GL_RGBA8;
            case GraphicsFormat::BC1_RGBA_sRGB:    return GL_SRGB8_ALPHA8;
            case GraphicsFormat::BC3_RGBA_UNorm:   return GL_RGBA8;
            case GraphicsFormat::BC3_RGBA_sRGB:    return GL_SRGB8_ALPHA8;
            case GraphicsFormat::BC5_RG_UNorm:     return GL_COMPRESSED_RG_RGTC2;

            default:                               return GL_RGBA8;
        }
    }

    u32 ToGLFormat(GraphicsFormat format) noexcept
    {
        switch (format)
        {
            case GraphicsFormat::R8_UNorm:
            case GraphicsFormat::R8_SNorm:
            case GraphicsFormat::R16_UNorm:
            case GraphicsFormat::R16_SNorm:
            case GraphicsFormat::R16_Float:
            case GraphicsFormat::R32_Float:
                return GL_RED;

            case GraphicsFormat::R8_UInt:
            case GraphicsFormat::R8_SInt:
            case GraphicsFormat::R16_UInt:
            case GraphicsFormat::R16_SInt:
            case GraphicsFormat::R32_UInt:
            case GraphicsFormat::R32_SInt:
                return GL_RED_INTEGER;

            case GraphicsFormat::RG8_UNorm:
            case GraphicsFormat::RG8_SNorm:
            case GraphicsFormat::RG16_UNorm:
            case GraphicsFormat::RG16_Float:
            case GraphicsFormat::RG32_Float:
                return GL_RG;

            case GraphicsFormat::RG8_UInt:
            case GraphicsFormat::RG8_SInt:
            case GraphicsFormat::RG32_UInt:
            case GraphicsFormat::RG32_SInt:
                return GL_RG_INTEGER;

            case GraphicsFormat::RGBA8_UNorm:
            case GraphicsFormat::RGBA8_SNorm:
            case GraphicsFormat::RGBA16_UNorm:
            case GraphicsFormat::RGBA16_Float:
            case GraphicsFormat::RGBA32_Float:
                return GL_RGBA;

            case GraphicsFormat::RGBA8_UInt:
            case GraphicsFormat::RGBA8_SInt:
            case GraphicsFormat::RGBA32_UInt:
            case GraphicsFormat::RGBA32_SInt:
                return GL_RGBA_INTEGER;

            case GraphicsFormat::RGBA8_sRGB:
            case GraphicsFormat::BGRA8_UNorm:
            case GraphicsFormat::BGRA8_sRGB:
                return GL_BGRA;

            case GraphicsFormat::RGB32_Float:
            case GraphicsFormat::RGB32_UInt:
            case GraphicsFormat::RGB32_SInt:
                return GL_RGB;

            case GraphicsFormat::R5G6B5_UNorm:
            case GraphicsFormat::RGBA4_UNorm:
            case GraphicsFormat::RGB5A1_UNorm:
            case GraphicsFormat::RGB10A2_UNorm:
            case GraphicsFormat::RG11B10_Float:
            case GraphicsFormat::RGB9E5_Float:
                return GL_RGBA;

            case GraphicsFormat::D16_UNorm:
            case GraphicsFormat::D32_Float:
                return GL_DEPTH_COMPONENT;

            case GraphicsFormat::D24_UNorm_S8_UInt:
            case GraphicsFormat::D32_Float_S8_UInt:
                return GL_DEPTH_STENCIL;

            case GraphicsFormat::S8_UInt:
                return GL_STENCIL_INDEX;

            default:
                return GL_RGBA;
        }
    }

    u32 ToGLType(GraphicsFormat format) noexcept
    {
        switch (format)
        {
            case GraphicsFormat::R8_UNorm:
            case GraphicsFormat::RG8_UNorm:
            case GraphicsFormat::RGBA8_UNorm:
            case GraphicsFormat::BGRA8_UNorm:
            case GraphicsFormat::R5G6B5_UNorm:
            case GraphicsFormat::RGBA4_UNorm:
            case GraphicsFormat::RGB5A1_UNorm:
                return GL_UNSIGNED_BYTE;

            case GraphicsFormat::R8_SNorm:
            case GraphicsFormat::RG8_SNorm:
            case GraphicsFormat::RGBA8_SNorm:
                return GL_BYTE;

            case GraphicsFormat::R8_UInt:
            case GraphicsFormat::RG8_UInt:
            case GraphicsFormat::RGBA8_UInt:
                return GL_UNSIGNED_BYTE;

            case GraphicsFormat::R8_SInt:
            case GraphicsFormat::RG8_SInt:
            case GraphicsFormat::RGBA8_SInt:
                return GL_BYTE;

            case GraphicsFormat::R16_UNorm:
            case GraphicsFormat::RG16_UNorm:
            case GraphicsFormat::RGBA16_UNorm:
                return GL_UNSIGNED_SHORT;

            case GraphicsFormat::R16_SNorm:
                return GL_SHORT;

            case GraphicsFormat::R16_UInt:
                return GL_UNSIGNED_SHORT;

            case GraphicsFormat::R16_SInt:
                return GL_SHORT;

            case GraphicsFormat::R16_Float:
            case GraphicsFormat::RG16_Float:
            case GraphicsFormat::RGBA16_Float:
                return GL_HALF_FLOAT;

            case GraphicsFormat::R32_UInt:
            case GraphicsFormat::RG32_UInt:
            case GraphicsFormat::RGB32_UInt:
            case GraphicsFormat::RGBA32_UInt:
                return GL_UNSIGNED_INT;

            case GraphicsFormat::R32_SInt:
            case GraphicsFormat::RG32_SInt:
            case GraphicsFormat::RGB32_SInt:
            case GraphicsFormat::RGBA32_SInt:
                return GL_INT;

            case GraphicsFormat::R32_Float:
            case GraphicsFormat::RG32_Float:
            case GraphicsFormat::RGB32_Float:
            case GraphicsFormat::RGBA32_Float:
                return GL_FLOAT;

            case GraphicsFormat::D16_UNorm:
                return GL_UNSIGNED_SHORT;

            case GraphicsFormat::D32_Float:
                return GL_FLOAT;

            case GraphicsFormat::D24_UNorm_S8_UInt:
            case GraphicsFormat::D32_Float_S8_UInt:
                return GL_UNSIGNED_INT_24_8;

            case GraphicsFormat::S8_UInt:
                return GL_UNSIGNED_BYTE;

            case GraphicsFormat::RGB10A2_UNorm:
                return GL_UNSIGNED_INT_2_10_10_10_REV;

            case GraphicsFormat::RG11B10_Float:
                return GL_UNSIGNED_INT_10F_11F_11F_REV;

            case GraphicsFormat::RGB9E5_Float:
                return GL_UNSIGNED_INT_5_9_9_9_REV;

            default:
                return GL_UNSIGNED_BYTE;
        }
    }

    u32 ToGLFormatSize(GraphicsFormat format) noexcept
    {
        return engine::rhi::GraphicsFormatSize(format);
    }

    bool IsDepthFormat(GraphicsFormat format) noexcept
    {
        return engine::rhi::IsDepthFormat(format);
    }

    bool IsStencilFormat(GraphicsFormat format) noexcept
    {
        return engine::rhi::IsStencilFormat(format);
    }

    u32 ToGLDepthStencilFormat(GraphicsFormat format) noexcept
    {
        switch (format)
        {
            case GraphicsFormat::D16_UNorm:        return GL_DEPTH_COMPONENT16;
            case GraphicsFormat::D24_UNorm_S8_UInt: return GL_DEPTH24_STENCIL8;
            case GraphicsFormat::D32_Float:        return GL_DEPTH_COMPONENT32F;
            case GraphicsFormat::D32_Float_S8_UInt: return GL_DEPTH32F_STENCIL8;
            default:                               return GL_DEPTH24_STENCIL8;
        }
    }

    // ========================================================================
    // Buffer usage
    // ========================================================================

    u32 ToGLBufferUsage(BufferUsage usage, bool immutable, bool dynamic) noexcept
    {
        if (immutable)
            return GL_STATIC_DRAW;

        const bool dynamicUsage = (usage & BufferUsage::Dynamic) != 0;
        if (dynamic || dynamicUsage)
        {
            const bool read = (usage & BufferUsage::TransferSource) != 0;
            return read ? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW;
        }

        const bool read = (usage & BufferUsage::TransferSource) != 0;
        return read ? GL_STATIC_READ : GL_STATIC_DRAW;
    }

    u32 ToGLBufferTarget(BufferUsage usage) noexcept
    {
        if ((usage & BufferUsage::VertexBuffer) != 0)
            return GL_ARRAY_BUFFER;
        if ((usage & BufferUsage::IndexBuffer) != 0)
            return GL_ELEMENT_ARRAY_BUFFER;
        if ((usage & BufferUsage::UniformBuffer) != 0)
            return GL_UNIFORM_BUFFER;
        if ((usage & BufferUsage::StorageBuffer) != 0)
            return GL_SHADER_STORAGE_BUFFER;
        if ((usage & BufferUsage::IndirectBuffer) != 0)
            return GL_DRAW_INDIRECT_BUFFER;
        return GL_ARRAY_BUFFER;
    }

    // ========================================================================
    // Texture
    // ========================================================================

    u32 ToGLTextureTarget(TextureType type, bool isCubeMap) noexcept
    {
        if (isCubeMap)
        {
            switch (type)
            {
                case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
                default:                            return GL_TEXTURE_CUBE_MAP;
            }
        }

        switch (type)
        {
            case TextureType::Texture1D:        return GL_TEXTURE_1D;
            case TextureType::Texture2D:        return GL_TEXTURE_2D;
            case TextureType::Texture3D:        return GL_TEXTURE_3D;
            case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP;
            case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
            case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
            case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
            default:                            return GL_TEXTURE_2D;
        }
    }

    u32 ToGLTextureBinding(u32 slot) noexcept
    {
        return GL_TEXTURE0 + slot;
    }

    // ========================================================================
    // Sampler
    // ========================================================================

    u32 ToGLMinFilter(FilterMode min, FilterMode mip) noexcept
    {
        if (min == FilterMode::Nearest)
            return (mip == FilterMode::Linear) ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
        return (mip == FilterMode::Linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    }

    u32 ToGLMagFilter(FilterMode mag) noexcept
    {
        return (mag == FilterMode::Nearest) ? GL_NEAREST : GL_LINEAR;
    }

    u32 ToGLAddressMode(AddressMode mode) noexcept
    {
        switch (mode)
        {
            case AddressMode::Repeat:            return GL_REPEAT;
            case AddressMode::MirroredRepeat:    return GL_MIRRORED_REPEAT;
            case AddressMode::ClampToEdge:       return GL_CLAMP_TO_EDGE;
            case AddressMode::ClampToBorder:     return GL_CLAMP_TO_BORDER;
            case AddressMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
            default:                             return GL_REPEAT;
        }
    }

    // ========================================================================
    // Pipeline state
    // ========================================================================

    u32 ToGLPrimitiveTopology(PrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
            case PrimitiveTopology::PointList:       return GL_POINTS;
            case PrimitiveTopology::LineList:        return GL_LINES;
            case PrimitiveTopology::LineStrip:       return GL_LINE_STRIP;
            case PrimitiveTopology::LineLoop:        return GL_LINE_LOOP;
            case PrimitiveTopology::TriangleList:    return GL_TRIANGLES;
            case PrimitiveTopology::TriangleStrip:   return GL_TRIANGLE_STRIP;
            case PrimitiveTopology::TriangleFan:     return GL_TRIANGLE_FAN;
            case PrimitiveTopology::LineListAdjacency:       return GL_LINES_ADJACENCY;
            case PrimitiveTopology::LineStripAdjacency:      return GL_LINE_STRIP_ADJACENCY;
            case PrimitiveTopology::TriangleListAdjacency:   return GL_TRIANGLES_ADJACENCY;
            case PrimitiveTopology::TriangleStripAdjacency:  return GL_TRIANGLE_STRIP_ADJACENCY;
            case PrimitiveTopology::PatchList:       return GL_PATCHES;
            default:                                 return GL_TRIANGLES;
        }
    }

    u32 ToGLIndexType(IndexType type) noexcept
    {
        return (type == IndexType::UInt16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    }

    u32 ToGLCullMode(CullMode mode) noexcept
    {
        switch (mode)
        {
            case CullMode::None:         return 0;
            case CullMode::Front:        return GL_FRONT;
            case CullMode::Back:         return GL_BACK;
            case CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
            default:                     return GL_BACK;
        }
    }

    u32 ToGLFrontFace(FrontFace face) noexcept
    {
        return (face == FrontFace::CounterClockwise) ? GL_CCW : GL_CW;
    }

    u32 ToGLPolygonMode(PolygonMode mode) noexcept
    {
        switch (mode)
        {
            case PolygonMode::Fill:  return GL_FILL;
            case PolygonMode::Line:  return GL_LINE;
            case PolygonMode::Point: return GL_POINT;
            default:                 return GL_FILL;
        }
    }

    u32 ToGLBlendFactor(BlendFactor factor) noexcept
    {
        switch (factor)
        {
            case BlendFactor::Zero:                  return GL_ZERO;
            case BlendFactor::One:                   return GL_ONE;
            case BlendFactor::SrcColor:              return GL_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:      return GL_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColor:              return GL_DST_COLOR;
            case BlendFactor::OneMinusDstColor:      return GL_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:              return GL_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:              return GL_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:      return GL_ONE_MINUS_DST_ALPHA;
            case BlendFactor::ConstantColor:         return GL_CONSTANT_COLOR;
            case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
            case BlendFactor::ConstantAlpha:         return GL_CONSTANT_ALPHA;
            case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
            case BlendFactor::SrcAlphaSaturate:      return GL_SRC_ALPHA_SATURATE;
            default:                                 return GL_ONE;
        }
    }

    u32 ToGLBlendOperation(BlendOperation op) noexcept
    {
        switch (op)
        {
            case BlendOperation::Add:             return GL_FUNC_ADD;
            case BlendOperation::Subtract:        return GL_FUNC_SUBTRACT;
            case BlendOperation::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
            case BlendOperation::Min:             return GL_MIN;
            case BlendOperation::Max:             return GL_MAX;
            default:                              return GL_FUNC_ADD;
        }
    }

    u32 ToGLCompareOperation(CompareOperation op) noexcept
    {
        switch (op)
        {
            case CompareOperation::Never:          return GL_NEVER;
            case CompareOperation::Less:           return GL_LESS;
            case CompareOperation::Equal:          return GL_EQUAL;
            case CompareOperation::LessOrEqual:    return GL_LEQUAL;
            case CompareOperation::Greater:        return GL_GREATER;
            case CompareOperation::NotEqual:       return GL_NOTEQUAL;
            case CompareOperation::GreaterOrEqual: return GL_GEQUAL;
            case CompareOperation::Always:         return GL_ALWAYS;
            default:                               return GL_LESS;
        }
    }

    u32 ToGLStencilOperation(StencilOperation op) noexcept
    {
        switch (op)
        {
            case StencilOperation::Keep:           return GL_KEEP;
            case StencilOperation::Zero:           return GL_ZERO;
            case StencilOperation::Replace:        return GL_REPLACE;
            case StencilOperation::IncrementClamp: return GL_INCR;
            case StencilOperation::DecrementClamp: return GL_DECR;
            case StencilOperation::Invert:         return GL_INVERT;
            case StencilOperation::IncrementWrap:  return GL_INCR_WRAP;
            case StencilOperation::DecrementWrap:  return GL_DECR_WRAP;
            default:                               return GL_KEEP;
        }
    }

    // ========================================================================
    // Attachment
    // ========================================================================

    u32 ToGLAttachmentLoadOp(AttachmentLoadOperation op) noexcept
    {
        // OpenGL doesn't have explicit load/store ops like Vulkan;
        // these are handled implicitly by glClear / glInvalidateFramebuffer.
        // We return 0 (unused) — the command buffer handles clears directly.
        (void)op;
        return 0;
    }

    u32 ToGLClearMask(AttachmentLoadOperation colorLoad,
                      AttachmentLoadOperation depthLoad,
                      AttachmentLoadOperation stencilLoad) noexcept
    {
        u32 mask = 0;
        if (colorLoad == AttachmentLoadOperation::Clear)   mask |= GL_COLOR_BUFFER_BIT;
        if (depthLoad == AttachmentLoadOperation::Clear)   mask |= GL_DEPTH_BUFFER_BIT;
        if (stencilLoad == AttachmentLoadOperation::Clear) mask |= GL_STENCIL_BUFFER_BIT;
        return mask;
    }

    // ========================================================================
    // Mapping
    // ========================================================================

    u32 ToGLMapAccess(MapFlags flags) noexcept
    {
        u32 access = 0;
        if ((flags & MapFlags::Read) != 0)  access |= GL_MAP_READ_BIT;
        if ((flags & MapFlags::Write) != 0) access |= GL_MAP_WRITE_BIT;
        if ((flags & MapFlags::Persistent) != 0)
        {
            access |= GL_MAP_PERSISTENT_BIT;
            if ((flags & MapFlags::Coherent) != 0)
                access |= GL_MAP_COHERENT_BIT;
        }
        if ((flags & MapFlags::FlushExplicit) != 0)
            access |= GL_MAP_FLUSH_EXPLICIT_BIT;
        if ((flags & MapFlags::Unsynchronized) != 0)
            access |= GL_MAP_UNSYNCHRONIZED_BIT;

        // If mapping for write, require GL_MAP_WRITE_BIT and the invalidate
        // range bit so the driver can avoid stalls.
        if ((flags & MapFlags::Write) != 0
            && (flags & MapFlags::Persistent) == 0)
        {
            access |= GL_MAP_INVALIDATE_RANGE_BIT;
        }

        return access;
    }

    // ========================================================================
    // Shader stage
    // ========================================================================

    u32 ToGLShaderType(ShaderStage stage) noexcept
    {
        switch (stage)
        {
            case ShaderStage::Vertex:         return GL_VERTEX_SHADER;
            case ShaderStage::Fragment:       return GL_FRAGMENT_SHADER;
            case ShaderStage::Geometry:       return GL_GEOMETRY_SHADER;
            case ShaderStage::TessControl:    return GL_TESS_CONTROL_SHADER;
            case ShaderStage::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
            case ShaderStage::Compute:        return GL_COMPUTE_SHADER;
            default:                          return GL_VERTEX_SHADER;
        }
    }

    bool IsShaderStageSingle(ShaderStage stage) noexcept
    {
        switch (stage)
        {
            case ShaderStage::Vertex:
            case ShaderStage::Fragment:
            case ShaderStage::Geometry:
            case ShaderStage::TessControl:
            case ShaderStage::TessEvaluation:
            case ShaderStage::Compute:
                return true;
            default:
                return false;
        }
    }

} // namespace engine::opengl
