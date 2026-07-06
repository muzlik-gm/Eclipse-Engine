// ============================================================================
// File: Engine/Source/Renderer/Pipeline/RenderPipeline.cpp
// ============================================================================
#include "Engine/Renderer/Pipeline/RenderPipeline.h"
#include "Engine/Renderer/Queue/RenderQueue.h"
#include "Engine/Renderer/Passes/IRenderPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Transforms/TransformUtils.h"
#include "Engine/Core/Log.h"

namespace engine::renderer {

    using engine::components::MeshComponent;
    using engine::components::MeshType;
    using engine::components::TransformComponent;
    using engine::components::VisibilityComponent;
    using engine::transforms::DecomposePosition;
    using engine::math::Vec4;

    RenderPipeline::RenderPipeline()
        : m_Queues(std::make_unique<RenderQueueManager>())
    {
    }

    RenderPipeline::~RenderPipeline()
    {
        Shutdown();
    }

    void RenderPipeline::Initialize()
    {
        if (m_Initialized) return;

        RegisterDefaultPasses();
        m_Graph.InitializeAll();

        m_Initialized = true;
        ENGINE_LOG_INFO("RenderPipeline — initialized with {} passes",
                        m_Graph.GetPassCount());
    }

    void RenderPipeline::Shutdown()
    {
        if (!m_Initialized) return;

        m_Graph.ShutdownAll();
        m_Initialized = false;
        ENGINE_LOG_INFO("RenderPipeline — shut down");
    }

    void RenderPipeline::RegisterDefaultPasses()
    {
        m_Graph.RegisterPass(std::make_unique<SkyPass>());
        m_Graph.RegisterPass(std::make_unique<GeometryPass>());
        m_Graph.RegisterPass(std::make_unique<TransparentPass>());
        m_Graph.RegisterPass(std::make_unique<DebugRenderPass>());
        m_Graph.RegisterPass(std::make_unique<UIPass>());
        m_Graph.RegisterPass(std::make_unique<PresentPass>());
    }

    void RenderPipeline::BeginFrame(const Mat4& viewProjection,
                                     const Mat4& viewMatrix,
                                     const Mat4& projectionMatrix,
                                     const Vec3& cameraPos,
                                     u32 viewportWidth, u32 viewportHeight,
                                     f64 deltaTime)
    {
        m_Queues->ClearAll();

        m_Context.ViewProjection  = viewProjection;
        m_Context.ViewMatrix       = viewMatrix;
        m_Context.ProjectionMatrix = projectionMatrix;
        m_Context.CameraPosition   = cameraPos;
        m_Context.Queues           = m_Queues.get();
        m_Context.Settings         = &m_Settings;
        m_Context.ViewportWidth    = viewportWidth;
        m_Context.ViewportHeight   = viewportHeight;
        m_Context.DeltaTime        = deltaTime;
    }

    void RenderPipeline::SubmitScene(engine::scene::Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        auto view = registry.View<MeshComponent, TransformComponent>();

        // Extract frustum from current view-projection.
        m_Visibility.m_Frustum.ExtractFromMatrix(m_Context.ViewProjection);

        u32 visible = 0, culled = 0;

        for (auto entity : view)
        {
            auto& mesh = registry.GetComponent<MeshComponent>(entity);
            auto& tf   = registry.GetComponent<TransformComponent>(entity);

            // Skip invisible entities.
            bool isVisible = true;
            if (registry.HasComponent<VisibilityComponent>(entity))
                isVisible = registry.GetComponent<VisibilityComponent>(entity).IsVisible;
            if (!isVisible) continue;

            // Skip None-type meshes.
            if (mesh.Type == MeshType::None)
                continue;

            // Frustum cull — test entity world position with a small radius.
            Vec3 worldPos = engine::transforms::DecomposePosition(tf.WorldMatrix);
            if (!m_Visibility.m_Frustum.IsSphereVisible(worldPos, 5.0f))
            {
                ++culled;
                continue;
            }

            // Submit to the opaque queue.
            RenderQueueEntry entry;
            entry.EntityHandle      = entity;
            entry.WorldMatrix    = tf.WorldMatrix;
            entry.ViewProjection = m_Context.ViewProjection;
            entry.MeshType       = static_cast<u32>(mesh.Type);
            entry.CastShadows    = mesh.CastShadows;
            entry.Visible        = true;
            entry.Distance       = glm::distance(m_Context.CameraPosition, worldPos);
            entry.SortKey        = static_cast<u32>(entry.Distance * 100.0f);

            m_Queues->Submit(RenderQueueType::Opaque, entry);
            ++visible;
        }

        m_Visibility.m_VisibleCount = visible;
        m_Visibility.m_CulledCount = culled;
    }

    void RenderPipeline::Execute()
    {
        // Sort all queues.
        m_Queues->SortAll();

        // Execute the render graph.
        m_Graph.Execute(m_Context);
    }

    void RenderPipeline::EndFrame()
    {
        // Frame finalization — statistics are updated by the renderer.
    }

} // namespace engine::renderer
