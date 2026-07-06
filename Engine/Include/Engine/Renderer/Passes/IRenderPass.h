// ============================================================================
// File: Engine/Include/Engine/Renderer/Passes/IRenderPass.h
// Base interface for all render passes.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/Renderer/Queue/RenderQueue.h"

#include <string>
#include <string_view>

namespace engine::renderer {

    using engine::core::f64;

    class RenderQueueManager;
    struct RenderSettings;

    // ========================================================================
    // RenderPassContext — data passed to each render pass during execution.
    // ========================================================================

    struct RenderPassContext
    {
        math::Mat4               ViewProjection{1.0f};
        math::Mat4               ViewMatrix{1.0f};
        math::Mat4               ProjectionMatrix{1.0f};
        math::Vec3               CameraPosition{0.0f};
        RenderQueueManager*      Queues{nullptr};
        const RenderSettings*    Settings{nullptr};
        u32                      ViewportWidth{0};
        u32                      ViewportHeight{0};
        f64                      DeltaTime{0.0};
    };

    // ========================================================================
    // IRenderPass — base interface for modular render passes.
    // ========================================================================

    /// @brief Base interface for all render passes.  Each pass is
    ///        responsible for one stage of the rendering pipeline
    ///        (e.g. Geometry, Sky, Transparent, Debug, UI, Present).
    class IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;

        /// @brief Returns the pass name (e.g. "GeometryPass").
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        /// @brief Returns the sort order (lower executes first).
        [[nodiscard]] virtual core::u32 GetSortOrder() const noexcept = 0;

        /// @brief Returns true if the pass should execute this frame.
        [[nodiscard]] virtual bool IsEnabled() const noexcept { return true; }

        /// @brief Initializes the pass (shaders, geometry, etc.).
        virtual void Initialize() {}

        /// @brief Executes the pass.
        virtual void Execute(const RenderPassContext& context) = 0;

        /// @brief Cleans up GPU resources.
        virtual void Shutdown() {}
    };

    // ========================================================================
    // Concrete render passes
    // ========================================================================

    /// @brief Renders opaque geometry from the opaque render queue.
    class GeometryPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "GeometryPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 100; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

    /// @brief Renders the sky (placeholder — draws a gradient background).
    class SkyPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "SkyPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 50; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

    /// @brief Renders transparent geometry from the transparent render queue.
    class TransparentPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "TransparentPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 200; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

    /// @brief Renders debug primitives (grid, bounding boxes, etc.).
    class DebugRenderPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "DebugRenderPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 300; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

    /// @brief Renders editor overlay (gizmos, selection outlines, icons).
    class UIPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "UIPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 400; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

    /// @brief Presents the final framebuffer to the screen.
    class PresentPass final : public IRenderPass
    {
    public:
        [[nodiscard]] std::string_view GetName() const noexcept override { return "PresentPass"; }
        [[nodiscard]] core::u32 GetSortOrder() const noexcept override { return 500; }
        void Initialize() override;
        void Execute(const RenderPassContext& context) override;
        void Shutdown() override;
    };

} // namespace engine::renderer
