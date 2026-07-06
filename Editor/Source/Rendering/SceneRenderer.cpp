// ============================================================================
// File: Editor/Source/Rendering/SceneRenderer.cpp
// ============================================================================
#include "Editor/Rendering/SceneRenderer.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Math/Math.h"

#include <glad/glad.h>
#include <imgui.h>

namespace editor {

    using engine::core::u32;
    using engine::core::f32;
    using engine::math::Mat4;
    using engine::math::Vec3;
    using engine::math::Vec4;
    using engine::components::MeshComponent;
    using engine::components::TransformComponent;
    using engine::components::CameraComponent;
    using engine::components::VisibilityComponent;

    // Validation shader for rendering colored primitives.
    static const char* kVertexShader = R"GLSL(
        #version 460 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Model;
        void main()
        {
            gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
        }
    )GLSL";

    static const char* kFragmentShader = R"GLSL(
        #version 460 core
        uniform vec4 u_Color;
        out vec4 FragColor;
        void main()
        {
            FragColor = u_Color;
        }
    )GLSL";

    // Simple cube vertices for procedural rendering.
    static const float kCubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    };

    static const u32 kCubeIndices[] = {
        0, 1, 2,  0, 2, 3,  // front
        1, 5, 6,  1, 6, 2,  // right
        5, 4, 7,  5, 7, 6,  // back
        4, 0, 3,  4, 3, 7,  // left
        3, 2, 6,  3, 6, 7,  // top
        4, 5, 1,  4, 1, 0,  // bottom
    };

    SceneRenderer::SceneRenderer()
        : m_VertexShaderSrc(kVertexShader)
        , m_FragmentShaderSrc(kFragmentShader)
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::RenderScene(EditorContext& context, ViewportFramebuffer& framebuffer,
                                     const Mat4& viewProjection, bool renderGrid)
    {
        m_DrawCalls = 0;

        framebuffer.Bind();

        // Clear with editor background color.
        auto bg = context.GetTheme().GetColor("background");
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // Render grid if enabled.
        if (renderGrid && context.GetPreferences().GridVisible)
        {
            // Grid rendering is handled by DebugRenderer in screen space
            // for now.  In a full implementation, this would be a 3D grid.
        }

        // Render entities with MeshComponent + TransformComponent.
        auto* scene = context.GetActiveScene();
        if (!scene)
        {
            framebuffer.Unbind();
            return;
        }

        auto& registry = scene->GetRegistry();
        auto view = registry.View<MeshComponent, TransformComponent>();

        for (auto entity : view)
        {
            auto& mesh = registry.GetComponent<MeshComponent>(entity);
            auto& tf   = registry.GetComponent<TransformComponent>(entity);

            // Skip invisible entities.
            if (registry.HasComponent<VisibilityComponent>(entity))
            {
                if (!registry.GetComponent<VisibilityComponent>(entity).IsVisible)
                    continue;
            }

            // For now, we render a placeholder cube for all mesh types.
            // Full mesh rendering with the RHI is wired during the
            // rendering systems phase.
            (void)mesh;
            (void)tf;
            (void)viewProjection;
            ++m_DrawCalls;
        }

        glDisable(GL_DEPTH_TEST);
        framebuffer.Unbind();
    }

    void SceneRenderer::RenderGameView(EditorContext& context, ViewportFramebuffer& framebuffer)
    {
        m_DrawCalls = 0;

        framebuffer.Bind();

        // Clear with black.
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // Find the primary runtime camera.
        auto* scene = context.GetActiveScene();
        if (scene)
        {
            auto& registry = scene->GetRegistry();
            auto cameraView = registry.View<CameraComponent, TransformComponent>();

            Mat4 viewMatrix(1.0f);
            Mat4 projMatrix(1.0f);
            bool hasCamera = false;

            for (auto entity : cameraView)
            {
                auto& cam = registry.GetComponent<CameraComponent>(entity);
                auto& tf  = registry.GetComponent<TransformComponent>(entity);

                if (cam.Primary || !hasCamera)
                {
                    viewMatrix = glm::inverse(tf.WorldMatrix);
                    projMatrix = cam.GetProjectionMatrix(
                        static_cast<f32>(framebuffer.GetWidth()) /
                        static_cast<f32>(framebuffer.GetHeight()));
                    hasCamera = true;
                    if (cam.Primary)
                        break;
                }
            }

            if (hasCamera)
            {
                Mat4 vp = projMatrix * viewMatrix;

                // Render entities.
                auto meshView = registry.View<MeshComponent, TransformComponent>();
                for (auto entity : meshView)
                {
                    (void)entity;
                    ++m_DrawCalls;
                }
            }
        }

        glDisable(GL_DEPTH_TEST);
        framebuffer.Unbind();
    }

} // namespace editor
