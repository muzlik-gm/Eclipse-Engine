// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/IGraphicsDevice.h
// Top-level device, adapter, context, and instance interfaces.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Interfaces/IBuffer.h"
#include "Engine/Renderer/RHI/Interfaces/ITexture.h"
#include "Engine/Renderer/RHI/Interfaces/IShader.h"
#include "Engine/Renderer/RHI/Interfaces/IPipeline.h"
#include "Engine/Renderer/RHI/Interfaces/ICommandQueue.h"
#include "Engine/Renderer/RHI/Interfaces/ISync.h"
#include "Engine/Renderer/RHI/Interfaces/ICapabilities.h"
#include "Engine/Renderer/RHI/Descriptions/ResourceDescriptions.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"

#include <memory>
#include <span>
#include <string>

namespace engine::rhi {

    using engine::core::u32;

    // ========================================================================
    // IGraphicsInstance — top-level connection to a graphics API.
    // ========================================================================

    /// @brief Top-level connection to a graphics API (VkInstance, ID3D12Device,
    ///        OpenGL context, MTLDevice).  Created first; everything else is
    ///        derived from it.
    class IGraphicsInstance : public IGraphicsObject
    {
    public:
        /// @brief Returns the active graphics backend.
        [[nodiscard]] virtual GraphicsBackend GetBackend() const noexcept = 0;

        /// @brief Enumerates the available physical adapters.
        [[nodiscard]] virtual std::vector<class IGraphicsAdapter*> EnumerateAdapters() = 0;

        /// @brief Returns the adapter this instance was created on.
        [[nodiscard]] virtual IGraphicsAdapter* GetAdapter() const noexcept = 0;

        /// @brief Returns the logical graphics device created from the adapter.
        ///        For backends where the instance and device are the same
        ///        (e.g. OpenGL), this returns the instance itself.
        [[nodiscard]] virtual class IGraphicsDevice* GetDevice() const noexcept = 0;
    };

    // ========================================================================
    // IGraphicsAdapter — a physical GPU.
    // ========================================================================

    /// @brief A physical GPU.  Adapters are enumerated from an
    ///        IGraphicsInstance and used to create a logical device.
    class IGraphicsAdapter : public IGraphicsObject, public ICapabilities
    {
    public:
        /// @brief Returns the adapter's UUID (stable across reboots).
        [[nodiscard]] virtual const std::string& GetAdapterUUID() const noexcept = 0;

        /// @brief Returns the adapter's PCI device ID (0 if unknown).
        [[nodiscard]] virtual u32 GetDeviceID() const noexcept = 0;

        /// @brief Returns the amount of dedicated VRAM in bytes.
        [[nodiscard]] virtual u64 GetDedicatedVideoMemory() const noexcept = 0;

        /// @brief Returns the amount of shared system memory in bytes.
        [[nodiscard]] virtual u64 GetSharedSystemMemory() const noexcept = 0;
    };

    // ========================================================================
    // IGraphicsContext — an active rendering context.
    // ========================================================================

    /// @brief An active rendering context.  On OpenGL this is the GL
    ///        context; on other backends it is implicit and may be a no-op.
    class IGraphicsContext : public IGraphicsObject
    {
    public:
        /// @brief Makes this context current on the calling thread.
        virtual void MakeCurrent() = 0;

        /// @brief Releases the current context from the calling thread.
        virtual void ReleaseCurrent() = 0;

        /// @brief Returns true if this context is current on the calling
        ///        thread.
        [[nodiscard]] virtual bool IsCurrent() const noexcept = 0;

        /// @brief Swaps the front and back buffers (OpenGL-style).
        virtual void SwapBuffers() = 0;

        /// @brief Sets the vsync mode.
        virtual void SetVSync(bool enabled) = 0;

        /// @brief Returns true if vsync is enabled.
        [[nodiscard]] virtual bool IsVSyncEnabled() const noexcept = 0;
    };

    // ========================================================================
    // IGraphicsDevice — the logical graphics device.
    // ========================================================================

    /// @brief The logical graphics device — the central object that
    ///        creates all GPU resources and queues.  This is the primary
    ///        entry point for the renderer after initialization.
    class IGraphicsDevice : public IGraphicsObject
    {
    public:
        /// @brief Returns the instance this device was created from.
        [[nodiscard]] virtual IGraphicsInstance* GetInstance() const noexcept = 0;

        /// @brief Returns the adapter this device runs on.
        [[nodiscard]] virtual IGraphicsAdapter* GetAdapter() const noexcept = 0;

        /// @brief Returns the device's context.
        [[nodiscard]] virtual IGraphicsContext* GetContext() const noexcept = 0;

        /// @brief Returns the device's command queue of the given type.
        [[nodiscard]] virtual ICommandQueue* GetQueue(QueueType type) = 0;

        /// @brief Returns the device's capabilities.
        [[nodiscard]] virtual const ICapabilities& GetCapabilities() const noexcept = 0;

        // -- Resource creation (delegated to IGraphicsFactory) -----------

        /// @brief Returns the factory used to create GPU resources.
        [[nodiscard]] virtual class IGraphicsFactory& GetFactory() = 0;

        /// @brief Creates a swapchain for presenting to a window.
        ///        The swapchain is bound to the device's command queue.
        [[nodiscard]] virtual std::unique_ptr<class ISwapChain>
            CreateSwapChain(const class SwapChainDescription& desc) = 0;

        // -- Synchronization ----------------------------------------------

        /// @brief Waits for the device to become idle.
        virtual void WaitIdle() = 0;

        /// @brief Forces any pending GPU work to flush.
        virtual void Flush() = 0;
    };

} // namespace engine::rhi
