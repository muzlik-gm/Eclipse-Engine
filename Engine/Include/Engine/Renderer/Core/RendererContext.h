// ============================================================================
// File: Engine/Include/Engine/Renderer/Core/RendererContext.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/RHI.h"
#include "Engine/Renderer/Core/RendererConfiguration.h"
#include "Engine/Renderer/Core/RendererStatistics.h"

#include <memory>

namespace engine::renderer {

    /// @brief Holds all shared renderer state: the graphics device, factory,
    ///        command queue, swapchain, and per-frame statistics.
    ///
    /// The RendererContext is owned by the Renderer and passed to
    /// subsystems that need direct RHI access (e.g. the future material
    /// system, mesh system, etc.).
    class RendererContext
    {
    public:
        RendererContext() = default;
        ~RendererContext() = default;

        RendererContext(const RendererContext&)            = delete;
        RendererContext& operator=(const RendererContext&) = delete;
        RendererContext(RendererContext&&)                 = delete;
        RendererContext& operator=(RendererContext&&)      = delete;

        // -- Initialization ------------------------------------------------

        bool Initialize(const RendererConfiguration& config);
        void Shutdown();

        [[nodiscard]] bool IsInitialized() const noexcept { return m_Device != nullptr; }

        // -- Accessors -----------------------------------------------------

        [[nodiscard]] engine::rhi::IGraphicsDevice* GetDevice() const noexcept
        { return m_Device; }

        [[nodiscard]] engine::rhi::IGraphicsFactory& GetFactory() const noexcept
        { return m_Device->GetFactory(); }

        [[nodiscard]] engine::rhi::ICommandQueue* GetCommandQueue() const noexcept
        { return m_CommandQueue; }

        [[nodiscard]] engine::rhi::ISwapChain* GetSwapChain() const noexcept
        { return m_SwapChain.get(); }

        [[nodiscard]] engine::rhi::ICommandBuffer* GetCommandBuffer() const noexcept
        { return m_CommandBuffer.get(); }

        [[nodiscard]] const RendererConfiguration& GetConfig() const noexcept
        { return m_Config; }

        [[nodiscard]] RendererStatistics& GetStatistics() noexcept
        { return m_Statistics; }

        [[nodiscard]] const RendererStatistics& GetStatistics() const noexcept
        { return m_Statistics; }

    private:
        RendererConfiguration                            m_Config;
        std::unique_ptr<engine::rhi::IGraphicsInstance>  m_Instance;
        engine::rhi::IGraphicsDevice*                    m_Device{nullptr};
        engine::rhi::ICommandQueue*                      m_CommandQueue{nullptr};
        std::unique_ptr<engine::rhi::ISwapChain>         m_SwapChain;
        std::unique_ptr<engine::rhi::ICommandBuffer>     m_CommandBuffer;
        RendererStatistics                               m_Statistics;
    };

} // namespace engine::renderer
