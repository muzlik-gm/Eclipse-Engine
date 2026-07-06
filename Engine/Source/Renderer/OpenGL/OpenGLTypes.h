// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLTypes.h
// Conversion helpers between engine RHI enums and OpenGL constants.
// This file is PRIVATE to the OpenGL backend — never included outside it.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <glad/glad.h>

namespace engine::opengl {

    using engine::core::u32;
    using engine::rhi::GraphicsFormat;
    using engine::rhi::BufferUsage;
    using engine::rhi::TextureUsage;
    using engine::rhi::TextureType;
    using engine::rhi::PrimitiveTopology;
    using engine::rhi::IndexType;
    using engine::rhi::BlendFactor;
    using engine::rhi::BlendOperation;
    using engine::rhi::CompareOperation;
    using engine::rhi::CullMode;
    using engine::rhi::FrontFace;
    using engine::rhi::PolygonMode;
    using engine::rhi::StencilOperation;
    using engine::rhi::FilterMode;
    using engine::rhi::AddressMode;
    using engine::rhi::AttachmentLoadOperation;
    using engine::rhi::MapFlags;
    using engine::rhi::ShaderStage;
    using engine::rhi::ShaderLanguage;

    // ========================================================================
    // Format → OpenGL internal format / pixel format / type
    // ========================================================================

    [[nodiscard]] u32 ToGLInternalFormat(GraphicsFormat format) noexcept;
    [[nodiscard]] u32 ToGLFormat(GraphicsFormat format) noexcept;        // pixel format
    [[nodiscard]] u32 ToGLType(GraphicsFormat format) noexcept;          // pixel type
    [[nodiscard]] u32 ToGLFormatSize(GraphicsFormat format) noexcept;

    [[nodiscard]] bool IsDepthFormat(GraphicsFormat format) noexcept;
    [[nodiscard]] bool IsStencilFormat(GraphicsFormat format) noexcept;
    [[nodiscard]] u32  ToGLDepthStencilFormat(GraphicsFormat format) noexcept;

    // ========================================================================
    // Buffer usage → OpenGL usage hints
    // ========================================================================

    [[nodiscard]] u32 ToGLBufferUsage(BufferUsage usage, bool immutable, bool dynamic) noexcept;
    [[nodiscard]] u32 ToGLBufferTarget(BufferUsage usage) noexcept;

    // ========================================================================
    // Texture → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLTextureTarget(TextureType type, bool isCubeMap = false) noexcept;
    [[nodiscard]] u32 ToGLTextureBinding(u32 slot) noexcept;  // GL_TEXTURE0 + slot

    // ========================================================================
    // Sampler → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLMinFilter(FilterMode min, FilterMode mip) noexcept;
    [[nodiscard]] u32 ToGLMagFilter(FilterMode mag) noexcept;
    [[nodiscard]] u32 ToGLAddressMode(AddressMode mode) noexcept;

    // ========================================================================
    // Pipeline state → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLPrimitiveTopology(PrimitiveTopology topology) noexcept;
    [[nodiscard]] u32 ToGLIndexType(IndexType type) noexcept;
    [[nodiscard]] u32 ToGLCullMode(CullMode mode) noexcept;
    [[nodiscard]] u32 ToGLFrontFace(FrontFace face) noexcept;
    [[nodiscard]] u32 ToGLPolygonMode(PolygonMode mode) noexcept;
    [[nodiscard]] u32 ToGLBlendFactor(BlendFactor factor) noexcept;
    [[nodiscard]] u32 ToGLBlendOperation(BlendOperation op) noexcept;
    [[nodiscard]] u32 ToGLCompareOperation(CompareOperation op) noexcept;
    [[nodiscard]] u32 ToGLStencilOperation(StencilOperation op) noexcept;

    // ========================================================================
    // Attachment load/store → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLAttachmentLoadOp(AttachmentLoadOperation op) noexcept;
    [[nodiscard]] u32 ToGLClearMask(AttachmentLoadOperation colorLoad,
                                     AttachmentLoadOperation depthLoad,
                                     AttachmentLoadOperation stencilLoad) noexcept;

    // ========================================================================
    // Mapping flags → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLMapAccess(MapFlags flags) noexcept;

    // ========================================================================
    // Shader stage → OpenGL
    // ========================================================================

    [[nodiscard]] u32 ToGLShaderType(ShaderStage stage) noexcept;
    [[nodiscard]] bool IsShaderStageSingle(ShaderStage stage) noexcept;

} // namespace engine::opengl
