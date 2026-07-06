// ============================================================================
// File: Engine/Source/Renderer/WorldRenderer.cpp
// Bridges the ECS World to the Renderer — extracts renderable data and
// submits draw calls through the RHI.
// ============================================================================
#include "Engine/Renderer/WorldRenderer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Core/Log.h"

namespace engine::renderer {

    using namespace engine::rhi;
    using namespace engine::components;
    using engine::core::u32;
    using engine::core::f32;

    // ========================================================================
    // Validation shader source — renders a colored triangle.
    // ========================================================================

    static const char* kValidationVertexShader = R"GLSL(
        #version 460 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Color;
        uniform mat4 u_ViewProjection;
        out vec3 v_Color;
        void main()
        {
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            v_Color = a_Color;
        }
    )GLSL";

    static const char* kValidationFragmentShader = R"GLSL(
        #version 460 core
        in vec3 v_Color;
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(v_Color, 1.0);
        }
    )GLSL";

    // ========================================================================
    // Triangle vertex data (position + color)
    // ========================================================================

    struct Vertex
    {
        math::Vec3 Position;
        math::Vec3 Color;
    };

    static const Vertex kTriangleVertices[3] = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };

    static const u32 kTriangleIndices[3] = {0, 1, 2};

    // ========================================================================
    // Initialize / Shutdown
    // ========================================================================

    bool WorldRenderer::Initialize(RendererContext& context)
    {
        m_Context = &context;

        CreateProceduralGeometry();

        m_Initialized = true;
        ENGINE_LOG_INFO("WorldRenderer — initialized");
        return true;
    }

    void WorldRenderer::Shutdown()
    {
        if (!m_Initialized)
            return;

        DestroyProceduralGeometry();
        m_Context = nullptr;
        m_Initialized = false;
        ENGINE_LOG_INFO("WorldRenderer — shut down");
    }

    void WorldRenderer::CreateProceduralGeometry()
    {
        if (!m_Context || !m_Context->GetDevice())
            return;

        auto& factory = m_Context->GetFactory();

        // Create vertex buffer.
        BufferDescription vbDesc{};
        vbDesc.Name   = "TriangleVB";
        vbDesc.Size   = sizeof(kTriangleVertices);
        vbDesc.Usage  = BufferUsage::VertexBuffer | BufferUsage::TransferDest;
        vbDesc.Stride = sizeof(Vertex);
        m_TriangleVB = factory.CreateVertexBuffer(vbDesc);
        if (m_TriangleVB)
            m_TriangleVB->UpdateData(kTriangleVertices, sizeof(kTriangleVertices));

        // Create index buffer.
        BufferDescription ibDesc{};
        ibDesc.Name       = "TriangleIB";
        ibDesc.Size       = sizeof(kTriangleIndices);
        ibDesc.Usage      = BufferUsage::IndexBuffer | BufferUsage::TransferDest;
        ibDesc.IndexFormat = IndexType::UInt32;
        m_TriangleIB = factory.CreateIndexBuffer(ibDesc);
        if (m_TriangleIB)
            m_TriangleIB->UpdateData(kTriangleIndices, sizeof(kTriangleIndices));

        // Create VAO.
        m_TriangleVAO = factory.CreateVertexArray();
        if (m_TriangleVAO)
        {
            m_TriangleVAO->BindVertexBuffer(0, m_TriangleVB.get(), 0);
            m_TriangleVAO->BindIndexBuffer(m_TriangleIB.get(), IndexType::UInt32, 0);

            VertexLayoutDescription layout{};
            layout.Bindings.push_back({0, sizeof(Vertex), false});
            layout.Attributes.push_back({0, 0, GraphicsFormat::RGB32_Float, offsetof(Vertex, Position), false});
            layout.Attributes.push_back({1, 0, GraphicsFormat::RGB32_Float, offsetof(Vertex, Color), false});
            m_TriangleVAO->SetVertexLayout(0, layout);
        }

        // Create shader.
        ShaderDescription shaderDesc{};
        shaderDesc.Name = "ValidationShader";
        shaderDesc.Stages.push_back({ShaderStage::Vertex, ShaderLanguage::GLSL, kValidationVertexShader, "main"});
        shaderDesc.Stages.push_back({ShaderStage::Fragment, ShaderLanguage::GLSL, kValidationFragmentShader, "main"});
        m_ValidationShader = factory.CreateShader(shaderDesc);

        // Create pipeline.
        if (m_ValidationShader && m_ValidationShader->IsLinked())
        {
            GraphicsPipelineDescription pipelineDesc{};
            pipelineDesc.Name = "ValidationPipeline";
            pipelineDesc.Shader = shaderDesc;
            pipelineDesc.TopologyValue = PrimitiveTopology::TriangleList;
            pipelineDesc.RasterizerState.CullModeValue = CullMode::None;
            pipelineDesc.DepthStencilState.DepthTestEnable = false;
            pipelineDesc.DepthStencilState.DepthWriteEnable = false;
            pipelineDesc.VertexLayout.Bindings.push_back({0, sizeof(Vertex), false});
            pipelineDesc.VertexLayout.Attributes.push_back({0, 0, GraphicsFormat::RGB32_Float, offsetof(Vertex, Position), false});
            pipelineDesc.VertexLayout.Attributes.push_back({1, 0, GraphicsFormat::RGB32_Float, offsetof(Vertex, Color), false});
            m_ValidationPipeline = factory.CreateGraphicsPipeline(pipelineDesc);
        }

        ENGINE_LOG_INFO("WorldRenderer — procedural geometry created");
    }

    void WorldRenderer::DestroyProceduralGeometry()
    {
        m_ValidationPipeline.reset();
        m_ValidationShader.reset();
        m_TriangleVAO.reset();
        m_TriangleIB.reset();
        m_TriangleVB.reset();
    }

    // ========================================================================
    // RenderScene — extract renderables and cameras, submit draw calls
    // ========================================================================

    void WorldRenderer::RenderScene(scene::Scene& scene)
    {
        if (!m_Initialized || !m_Context)
            return;

        m_RenderableCount = 0;
        m_CameraCount = 0;

        auto& registry = scene.GetRegistry();

        // -- Find the primary camera -----------------------------------------
        math::Mat4 viewMatrix(1.0f);
        math::Mat4 projectionMatrix(1.0f);
        bool hasCamera = false;

        auto cameraView = registry.View<CameraComponent, TransformComponent>();
        for (auto entity : cameraView)
        {
            auto& cam = registry.GetComponent<CameraComponent>(entity);
            auto& tf  = registry.GetComponent<TransformComponent>(entity);

            if (cam.Primary || !hasCamera)
            {
                // Build view matrix from transform (inverse of world matrix).
                viewMatrix = glm::inverse(tf.WorldMatrix);
                projectionMatrix = cam.GetProjectionMatrix(16.0f / 9.0f);
                hasCamera = true;
                ++m_CameraCount;
                if (cam.Primary)
                    break;
            }
        }

        if (!hasCamera)
        {
            // Use a default camera looking at the origin.
            viewMatrix = math::LookAt(math::Vec3(0.0f, 0.0f, 5.0f),
                                       math::Vec3(0.0f, 0.0f, 0.0f),
                                       math::Vec3(0.0f, 1.0f, 0.0f));
            projectionMatrix = math::Perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
        }

        const math::Mat4 viewProjection = projectionMatrix * viewMatrix;

        // -- Count renderable entities --------------------------------------
        auto meshView = registry.View<MeshComponent, TransformComponent>();
        for (auto entity : meshView)
        {
            // Check visibility.
            bool isVisible = true;
            if (registry.HasComponent<VisibilityComponent>(entity))
            {
                isVisible = registry.GetComponent<VisibilityComponent>(entity).IsVisible;
            }

            if (isVisible)
                ++m_RenderableCount;
        }

        // -- Submit a validation draw call (colored triangle) ---------------
        // This proves the renderer integration is functional.
        auto* cmdBuf = m_Context->GetCommandBuffer();
        if (cmdBuf && m_ValidationPipeline && m_TriangleVAO)
        {
            cmdBuf->Begin();

            // Set viewport to swapchain dimensions.
            auto* swapchain = m_Context->GetSwapChain();
            if (swapchain)
            {
                auto* tex = swapchain->GetCurrentBuffer();
                if (tex)
                {
                    cmdBuf->SetViewport(ViewportDescription{
                        0.0f, 0.0f,
                        static_cast<f32>(tex->GetWidth()),
                        static_cast<f32>(tex->GetHeight()),
                        0.0f, 1.0f
                    });
                }
            }

            // Clear the default framebuffer.
            cmdBuf->ClearColor(0, 0.1f, 0.1f, 0.1f, 1.0f);

            // Bind pipeline and VAO.
            cmdBuf->BindPipeline(m_ValidationPipeline.get());
            cmdBuf->BindVertexArray(m_TriangleVAO.get());

            // Draw the validation triangle.
            cmdBuf->Draw(3, 1, 0, 0);

            cmdBuf->End();
        }
    }

} // namespace engine::renderer
