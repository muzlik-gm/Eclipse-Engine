// ============================================================================
// File: Engine/Include/Engine/Renderer/Pipeline/RenderPipeline.h
// Configurable render pipeline that orchestrates the full frame.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Graph/RenderGraph.h"
#include "Engine/Renderer/Queue/RenderQueue.h"
#include "Engine/Renderer/Visibility/VisibilitySystem.h"
#include "Engine/Renderer/Core/RenderSettings.h"

namespace engine::renderer {

    class RenderQueueManager;

    // ========================================================================
    // RenderPipeline — orchestrates the full rendering frame.
    // ========================================================================

    /// @brief The Render Pipeline is the top-level orchestrator.  Each
    ///        frame it: clears queues, runs visibility culling, submits
    ///        visible entities to queues, executes the render graph,
    ///        and finalizes the frame.
    class RenderPipeline
    {
    public:
        RenderPipeline();
        ~RenderPipeline();

        /// @brief Initializes the pipeline (registers default passes).
        void Initialize();

        /// @brief Shuts down the pipeline.
        void Shutdown();

        /// @brief Begins a new frame.  Clears queues, resets statistics.
        void BeginFrame(const Mat4& viewProjection, const Mat4& viewMatrix,
                        const Mat4& projectionMatrix, const Vec3& cameraPos,
                        u32 viewportWidth, u32 viewportHeight, f64 deltaTime);

        /// @brief Submits scene entities for rendering.
        /// @param registry  The ECS registry to iterate.
        void SubmitScene(engine::scene::Scene& scene);

        /// @brief Executes the render graph.
        void Execute();

        /// @brief Ends the frame.  Sorts queues, finalizes statistics.
        void EndFrame();

        // -- Accessors -----------------------------------------------------

        [[nodiscard]] RenderGraph&          GetGraph()          noexcept { return m_Graph; }
        [[nodiscard]] RenderQueueManager&   GetQueues()         noexcept { return *m_Queues; }
        [[nodiscard]] VisibilitySystem&     GetVisibility()     noexcept { return m_Visibility; }
        [[nodiscard]] const RenderSettings& GetSettings() const noexcept { return m_Settings; }

        void SetSettings(const RenderSettings& settings) { m_Settings = settings; }

        /// @brief Returns the render pass context for the current frame.
        [[nodiscard]] const RenderPassContext& GetContext() const noexcept
        { return m_Context; }

    private:
        void RegisterDefaultPasses();

        RenderGraph           m_Graph;
        std::unique_ptr<RenderQueueManager> m_Queues;
        VisibilitySystem      m_Visibility;
        RenderSettings        m_Settings;
        RenderPassContext     m_Context;
        bool                  m_Initialized{false};
    };

} // namespace engine::renderer
