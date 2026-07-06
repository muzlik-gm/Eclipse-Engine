// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/ISync.h
// Abstract interfaces for GPU synchronization primitives (fences and
// semaphores), descriptor sets, descriptor layouts, and descriptor pools.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <chrono>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::u64;

    // ========================================================================
    // IFence — CPU-GPU synchronization primitive.
    // ========================================================================

    /// @brief A fence signals CPU code that the GPU has finished executing
    ///        up to a certain point.  Fences are used to safely recycle
    ///        per-frame command buffers and resources.
    class IFence : public IGraphicsObject
    {
    public:
        /// @brief Resets the fence to the unsignaled state.
        virtual void Reset() = 0;

        /// @brief Returns true if the fence has been signaled.
        [[nodiscard]] virtual bool IsSignaled() const = 0;

        /// @brief Blocks the calling thread until the fence is signaled.
        virtual void Wait() = 0;

        /// @brief Blocks for at most @p timeout.  Returns true if signaled.
        template <typename Rep, typename Period>
        bool WaitFor(std::chrono::duration<Rep, Period> timeout)
        {
            return WaitForNanoseconds(
                static_cast<u64>(std::chrono::duration_cast<
                    std::chrono::nanoseconds>(timeout).count()));
        }

        /// @brief Returns the current signaled value (for timeline
        ///        semaphores / fences that support values).
        [[nodiscard]] virtual u64 GetValue() const = 0;

    protected:
        /// @brief Implementation-specific wait with nanosecond timeout.
        virtual bool WaitForNanoseconds(u64 timeoutNs) = 0;
    };

    // ========================================================================
    // ISemaphore — GPU-GPU synchronization primitive.
    // ========================================================================

    /// @brief A semaphore signals one GPU queue that another queue has
    ///        finished executing.  Used for swapchain image acquisition
    ///        and cross-queue dependencies.
    class ISemaphore : public IGraphicsObject
    {
    public:
        /// @brief Returns the current signaled value.
        [[nodiscard]] virtual u64 GetValue() const = 0;

        /// @brief Resets the semaphore to the given value.
        virtual void Reset(u64 value = 0) = 0;
    };

    // ========================================================================
    // IDescriptorLayout — describes the layout of a descriptor set.
    // ========================================================================

    /// @brief Describes the bindings of a descriptor set.  Created from a
    ///        DescriptorLayoutDescription and shared across descriptor sets
    ///        that use the same layout.
    class IDescriptorLayout : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual const DescriptorLayoutDescription& GetDescription() const noexcept = 0;

        [[nodiscard]] virtual u32 GetBindingCount() const noexcept = 0;

        [[nodiscard]] virtual DescriptorType GetBindingType(u32 binding) const = 0;

        [[nodiscard]] virtual u32 GetBindingCount(u32 binding) const = 0;
    };

    // ========================================================================
    // IDescriptorSet — a concrete set of bound resources.
    // ========================================================================

    /// @brief A descriptor set — a concrete set of resources (textures,
    ///        samplers, UBOs, SSBOs) bound to specific slots, ready for
    ///        shader access.
    class IDescriptorSet : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual IDescriptorLayout* GetLayout() const noexcept = 0;

        /// @brief Binds a sampler to the given binding.
        virtual void BindSampler(u32 binding, ISampler* sampler) = 0;

        /// @brief Binds a texture to the given binding.
        virtual void BindTexture(u32 binding, ITexture* texture) = 0;

        /// @brief Binds a uniform buffer to the given binding.
        virtual void BindUniformBuffer(u32 binding, IUniformBuffer* buffer) = 0;

        /// @brief Binds a storage buffer to the given binding.
        virtual void BindStorageBuffer(u32 binding, IStorageBuffer* buffer) = 0;

        /// @brief Flushes any pending binding updates so the set is ready
        ///        for use by a command buffer.
        virtual void Update() = 0;
    };

    // ========================================================================
    // IDescriptorPool — allocates descriptor sets.
    // ========================================================================

    /// @brief A descriptor pool — allocates descriptor sets from a fixed
    ///        pool of memory.  Pools are reset to recycle all their sets
    ///        at once.
    class IDescriptorPool : public IGraphicsObject
    {
    public:
        /// @brief Allocates a descriptor set from the pool.
        /// @return The new descriptor set, or nullptr if the pool is full.
        [[nodiscard]] virtual IDescriptorSet* Allocate(IDescriptorLayout* layout) = 0;

        /// @brief Resets the pool, releasing all allocated sets.
        virtual void Reset() = 0;

        /// @brief Returns the number of currently allocated sets.
        [[nodiscard]] virtual u32 GetAllocatedCount() const noexcept = 0;

        /// @brief Returns the maximum number of sets this pool can hold.
        [[nodiscard]] virtual u32 GetMaxSets() const noexcept = 0;
    };

} // namespace engine::rhi
