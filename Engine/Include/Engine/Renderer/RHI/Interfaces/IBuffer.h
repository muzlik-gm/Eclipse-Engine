// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/IBuffer.h
// Abstract interfaces for GPU buffers: vertex, index, uniform, storage.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <cstddef>
#include <span>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::usize;

    // ========================================================================
    // IBuffer — base interface for all GPU buffers.
    // ========================================================================

    /// @brief Base interface for every GPU buffer type.
    ///
    /// Buffers are created through IGraphicsFactory and destroyed by
    /// deleting their owning unique_ptr.  The backend's destructor is
    /// responsible for releasing GPU memory.
    class IBuffer : public IGraphicsObject
    {
    public:
        /// @brief Returns the description this buffer was created with.
        [[nodiscard]] virtual const BufferDescription& GetDescription() const noexcept = 0;

        /// @brief Returns the size of the buffer in bytes.
        [[nodiscard]] virtual usize GetSize() const noexcept = 0;

        /// @brief Returns the buffer's usage flags.
        [[nodiscard]] virtual BufferUsage GetUsage() const noexcept = 0;

        /// @brief Maps the buffer's storage into CPU address space.
        ///
        /// @param flags   Read / Write / combination.
        /// @param offset  Byte offset from the start of the buffer.
        /// @param size    Number of bytes to map (0 = up to end of buffer).
        /// @return Pointer to the mapped region, or nullptr on failure.
        [[nodiscard]] virtual void* Map(MapFlags flags, usize offset = 0, usize size = 0) = 0;

        /// @brief Unmaps a previously mapped buffer.
        virtual void Unmap() = 0;

        /// @brief Flushes a mapped range so GPU writes become visible.
        virtual void FlushMappedRange(usize offset, usize size) = 0;

        /// @brief Invalidates a mapped range so GPU writes are discarded.
        virtual void InvalidateMappedRange(usize offset, usize size) = 0;

        /// @brief Uploads data from CPU memory into the buffer.
        ///        This is a high-level convenience over Map/Unmap.
        /// @param data    Pointer to source data.
        /// @param size    Number of bytes to upload.
        /// @param offset  Destination offset inside the buffer.
        virtual void UpdateData(const void* data, usize size, usize offset = 0) = 0;
    };

    // ========================================================================
    // IVertexBuffer
    // ========================================================================

    /// @brief Vertex buffer — feeds per-vertex attributes into the
    ///        vertex shader through the pipeline's vertex layout.
    class IVertexBuffer : public virtual IBuffer
    {
    public:
        /// @brief Returns the stride between consecutive vertices.
        [[nodiscard]] virtual usize GetStride() const noexcept = 0;

        /// @brief Returns the number of vertices the buffer can hold.
        [[nodiscard]] virtual u32 GetVertexCount() const noexcept = 0;
    };

    // ========================================================================
    // IIndexBuffer
    // ========================================================================

    /// @brief Index buffer — provides indices into a vertex buffer for
    ///        indexed draw calls.
    class IIndexBuffer : public virtual IBuffer
    {
    public:
        /// @brief Returns the index type (UInt16 or UInt32).
        [[nodiscard]] virtual IndexType GetIndexType() const noexcept = 0;

        /// @brief Returns the number of indices in the buffer.
        [[nodiscard]] virtual u32 GetIndexCount() const noexcept = 0;
    };

    // ========================================================================
    // IUniformBuffer
    // ========================================================================

    /// @brief Uniform buffer — read-only structured data accessible from
    ///        any shader stage.  Also known as a constant buffer (D3D) or
    ///        UBO (OpenGL).
    class IUniformBuffer : public virtual IBuffer
    {
    public:
        /// @brief Returns the size of the uniform block in bytes.
        [[nodiscard]] virtual usize GetBlockSize() const noexcept = 0;

        /// @brief Sets the binding point this UBO should be bound to.
        virtual void SetBindingPoint(u32 binding) = 0;

        /// @brief Returns the current binding point.
        [[nodiscard]] virtual u32 GetBindingPoint() const noexcept = 0;
    };

    // ========================================================================
    // IStorageBuffer
    // ========================================================================

    /// @brief Storage buffer — read-write structured data accessible from
    ///        any shader stage.  Also known as an SSBO (OpenGL) or UAV (D3D).
    class IStorageBuffer : public virtual IBuffer
    {
    public:
        /// @brief Returns the stride of a single structured element, or
        ///        0 if the buffer is unstructured.
        [[nodiscard]] virtual usize GetElementStride() const noexcept = 0;

        /// @brief Returns the number of structured elements.
        [[nodiscard]] virtual u32 GetElementCount() const noexcept = 0;
    };

} // namespace engine::rhi
