// ============================================================================
// File: Engine/Include/Engine/Renderer/Core/Renderer.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Runtime/ISubsystem.h"
#include "Engine/Renderer/Core/RendererContext.h"

#include <memory>

namespace engine::renderer {

    /// @brief The Renderer is an ISubsystem that owns the RendererContext
    ///        and drives the per-frame render loop.  It is completely
    ///        backend-independent — all graphics API calls go through the
    ///        RHI interfaces held by the RendererContext.
    class Renderer final : public engine::runtime::ISubsystem
    {
    public:
        Renderer();
        ~Renderer() override;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;

        // -- ISubsystem interface ------------------------------------------

        [[nodiscard]] std::string_view GetName() const noexcept override
        { return "Renderer"; }

        [[nodiscard]] std::vector<std::string> GetDependencies() const override
        { return {}; }

        bool Initialize() override;
        void Shutdown() override;

        void Update(engine::core::f64 deltaTime) override;
        void FixedUpdate(engine::core::f64 fixedDeltaTime) override;
        void LateUpdate(engine::core::f64 deltaTime) override;

        // -- Renderer-specific API -----------------------------------------

        /// @brief Sets the renderer configuration.  Must be called before
        ///        Initialize().
        void SetConfiguration(const RendererConfiguration& config);

        /// @brief Returns the renderer context.
        [[nodiscard]] RendererContext& GetContext() noexcept { return m_Context; }
        [[nodiscard]] const RendererContext& GetContext() const noexcept { return m_Context; }

        /// @brief Begins a new frame.  Acquires the swapchain image and
        ///        begins recording the command buffer.
        void BeginFrame();

        /// @brief Ends the current frame.  Ends the command buffer, submits
        ///        it, and presents the swapchain image.
        void EndFrame();

        /// @brief Handles a window resize event.
        void OnResize(engine::core::u32 width, engine::core::u32 height);

        /// @brief Returns true if a frame is currently in progress.
        [[nodiscard]] bool IsFrameInProgress() const noexcept { return m_FrameInProgress; }

    private:
        RendererContext m_Context;
        RendererConfiguration m_Config;
        bool m_Initialized{false};
        bool m_FrameInProgress{false};
    };

} // namespace engine::renderer
