// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLBuffer.h
// Concrete OpenGL buffer implementations: vertex, index, uniform, storage.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"

#include <glad/glad.h>

#include <string>
#include <string_view>

namespace engine::opengl {

    class OpenGLDebugLayer;
    class OpenGLStateCache;

    // ========================================================================
    // OpenGLBuffer — base implementation
    // ========================================================================

    // NOTE: OpenGLBuffer does NOT inherit from engine::rhi::IBuffer.
    // The concrete subclasses (OpenGLVertexBuffer, etc.) inherit from both
    // OpenGLBuffer and the corresponding RHI interface.  This avoids the
    // diamond inheritance problem that would arise if OpenGLBuffer also
    // inherited from IBuffer.
    class OpenGLBuffer : public virtual engine::rhi::IBuffer
    {
    public:
        OpenGLBuffer(const engine::rhi::BufferDescription& desc,
                     OpenGLDebugLayer* debugLayer);
        ~OpenGLBuffer() override;

        OpenGLBuffer(const OpenGLBuffer&)            = delete;
        OpenGLBuffer& operator=(const OpenGLBuffer&) = delete;
        OpenGLBuffer(OpenGLBuffer&&)                 = delete;
        OpenGLBuffer& operator=(OpenGLBuffer&&)      = delete;

        // -- IGraphicsObject
        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        // -- IBuffer
        const engine::rhi::BufferDescription& GetDescription() const noexcept override
        { return m_Description; }

        engine::core::usize GetSize() const noexcept override { return m_Description.Size; }
        engine::rhi::BufferUsage GetUsage() const noexcept override { return m_Description.Usage; }

        void* Map(engine::rhi::MapFlags flags,
                  engine::core::usize offset = 0,
                  engine::core::usize size = 0) override;
        void Unmap() override;
        void FlushMappedRange(engine::core::usize offset, engine::core::usize size) override;
        void InvalidateMappedRange(engine::core::usize offset, engine::core::usize size) override;
        void UpdateData(const void* data, engine::core::usize size,
                        engine::core::usize offset = 0) override;

        // -- OpenGL-specific ------------------------------------------------
        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }
        [[nodiscard]] GLuint GetTarget() const noexcept { return m_Target; }
        void Bind(OpenGLStateCache& stateCache);

    protected:
        engine::rhi::BufferDescription m_Description;
        GLuint                          m_Handle{0};
        GLuint                          m_Target{GL_ARRAY_BUFFER};
        std::string                     m_Name;
        OpenGLDebugLayer*               m_DebugLayer{nullptr};
        void*                           m_MappedPointer{nullptr};
        bool                            m_PersistentMapped{false};
    };

    // ========================================================================
    // OpenGLVertexBuffer
    // ========================================================================

    class OpenGLVertexBuffer final : public OpenGLBuffer, public engine::rhi::IVertexBuffer
    {
    public:
        OpenGLVertexBuffer(const engine::rhi::BufferDescription& desc,
                           OpenGLDebugLayer* debugLayer);

        engine::core::usize GetStride() const noexcept override
        { return m_Description.Stride; }

        engine::core::u32 GetVertexCount() const noexcept override
        {
            return m_Description.Stride > 0
                ? static_cast<engine::core::u32>(m_Description.Size / m_Description.Stride)
                : 0;
        }
    };

    // ========================================================================
    // OpenGLIndexBuffer
    // ========================================================================

    class OpenGLIndexBuffer final : public OpenGLBuffer, public engine::rhi::IIndexBuffer
    {
    public:
        OpenGLIndexBuffer(const engine::rhi::BufferDescription& desc,
                          OpenGLDebugLayer* debugLayer);

        engine::rhi::IndexType GetIndexType() const noexcept override
        { return m_Description.IndexFormat; }

        engine::core::u32 GetIndexCount() const noexcept override
        {
            const engine::core::usize indexSize = (m_Description.IndexFormat == engine::rhi::IndexType::UInt16)
                ? 2 : 4;
            return static_cast<engine::core::u32>(m_Description.Size / indexSize);
        }
    };

    // ========================================================================
    // OpenGLUniformBuffer
    // ========================================================================

    class OpenGLUniformBuffer final : public OpenGLBuffer, public engine::rhi::IUniformBuffer
    {
    public:
        OpenGLUniformBuffer(const engine::rhi::BufferDescription& desc,
                            OpenGLDebugLayer* debugLayer);

        engine::core::usize GetBlockSize() const noexcept override
        { return m_Description.Size; }

        void SetBindingPoint(engine::core::u32 binding) override { m_BindingPoint = binding; }
        engine::core::u32 GetBindingPoint() const noexcept override { return m_BindingPoint; }

        void Bind(OpenGLStateCache& stateCache);

    private:
        engine::core::u32 m_BindingPoint{0};
    };

    // ========================================================================
    // OpenGLStorageBuffer
    // ========================================================================

    class OpenGLStorageBuffer final : public OpenGLBuffer, public engine::rhi::IStorageBuffer
    {
    public:
        OpenGLStorageBuffer(const engine::rhi::BufferDescription& desc,
                            OpenGLDebugLayer* debugLayer);

        engine::core::usize GetElementStride() const noexcept override
        { return m_Description.Stride; }

        engine::core::u32 GetElementCount() const noexcept override
        {
            return m_Description.Stride > 0
                ? static_cast<engine::core::u32>(m_Description.Size / m_Description.Stride)
                : 0;
        }

        void Bind(OpenGLStateCache& stateCache);
    };

} // namespace engine::opengl
