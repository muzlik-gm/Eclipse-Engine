// ============================================================================
// File: Editor/Source/Rendering/SceneRenderer.cpp
// Actually renders entities + grid into the viewport framebuffer.
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
#include <vector>
#include <cmath>

namespace editor {

    using engine::core::u32;
    using engine::core::f32;
    using engine::math::Mat4;
    using engine::math::Vec3;
    using engine::math::Vec4;
    using engine::components::MeshComponent;
    using engine::components::MeshType;
    using engine::components::TransformComponent;
    using engine::components::CameraComponent;
    using engine::components::VisibilityComponent;

    // ========================================================================
    // Shader sources — use 460 core to match the GL context.
    // ========================================================================

    static const char* kMeshVertexShader = R"GLSL(
        #version 460 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        uniform mat4 u_ViewProj;
        uniform mat4 u_Model;
        out vec3 v_Normal;
        void main()
        {
            gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
            v_Normal = mat3(u_Model) * a_Normal;
        }
    )GLSL";

    static const char* kMeshFragmentShader = R"GLSL(
        #version 460 core
        in vec3 v_Normal;
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main()
        {
            vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
            float diff = max(dot(normalize(v_Normal), lightDir), 0.0);
            FragColor = vec4(u_Color.rgb * (0.3 + diff * 0.7), u_Color.a);
        }
    )GLSL";

    static const char* kGridVertexShader = R"GLSL(
        #version 460 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProj;
        out float v_Dist;
        void main()
        {
            gl_Position = u_ViewProj * vec4(a_Position, 1.0);
            v_Dist = length(a_Position.xz);
        }
    )GLSL";

    static const char* kGridFragmentShader = R"GLSL(
        #version 460 core
        in float v_Dist;
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main()
        {
            float fade = 1.0 - clamp(v_Dist / 50.0, 0.0, 1.0);
            FragColor = vec4(u_Color.rgb, u_Color.a * fade);
        }
    )GLSL";

    // ========================================================================
    // Cube data
    // ========================================================================

    struct Vertex { Vec3 Position; Vec3 Normal; };

    static const Vertex kCubeVertices[] = {
        {{-0.5f,-0.5f, 0.5f},{0,0,1}}, {{ 0.5f,-0.5f, 0.5f},{0,0,1}},
        {{ 0.5f, 0.5f, 0.5f},{0,0,1}}, {{-0.5f, 0.5f, 0.5f},{0,0,1}},
        {{-0.5f,-0.5f,-0.5f},{0,0,-1}}, {{ 0.5f,-0.5f,-0.5f},{0,0,-1}},
        {{ 0.5f, 0.5f,-0.5f},{0,0,-1}}, {{-0.5f, 0.5f,-0.5f},{0,0,-1}},
        {{-0.5f, 0.5f, 0.5f},{0,1,0}}, {{ 0.5f, 0.5f, 0.5f},{0,1,0}},
        {{ 0.5f, 0.5f,-0.5f},{0,1,0}}, {{-0.5f, 0.5f,-0.5f},{0,1,0}},
        {{-0.5f,-0.5f,-0.5f},{0,-1,0}}, {{ 0.5f,-0.5f,-0.5f},{0,-1,0}},
        {{ 0.5f,-0.5f, 0.5f},{0,-1,0}}, {{-0.5f,-0.5f, 0.5f},{0,-1,0}},
        {{ 0.5f,-0.5f, 0.5f},{1,0,0}}, {{ 0.5f,-0.5f,-0.5f},{1,0,0}},
        {{ 0.5f, 0.5f,-0.5f},{1,0,0}}, {{ 0.5f, 0.5f, 0.5f},{1,0,0}},
        {{-0.5f,-0.5f,-0.5f},{-1,0,0}}, {{-0.5f,-0.5f, 0.5f},{-1,0,0}},
        {{-0.5f, 0.5f, 0.5f},{-1,0,0}}, {{-0.5f, 0.5f,-0.5f},{-1,0,0}},
    };

    static const u32 kCubeIndices[] = {
        0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11,
        12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23,
    };

    // ========================================================================
    // Shader helper
    // ========================================================================

    static GLuint CompileShader(GLenum type, const char* source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE)
        {
            char log[1024];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            ENGINE_LOG_ERROR("SceneRenderer — shader compile error:\n{}", log);
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint LinkProgram(const char* vsSrc, const char* fsSrc)
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
        if (!vs || !fs) { if (vs) glDeleteShader(vs); if (fs) glDeleteShader(fs); return 0; }

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE)
        {
            char log[1024];
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            ENGINE_LOG_ERROR("SceneRenderer — program link error:\n{}", log);
            glDeleteProgram(program);
            program = 0;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

    // ========================================================================
    // GL state saver — saves ALL state that SceneRenderer modifies.
    // This is critical because we render INSIDE ImGui's frame.
    // ========================================================================

    struct GLStateSaver
    {
        GLint lastDrawFBO{0};
        GLint lastReadFBO{0};
        GLint lastViewport[4]{0,0,0,0};
        GLint lastProgram{0};
        GLint lastVAO{0};
        GLint lastArrayBuffer{0};
        GLint lastElementBuffer{0};
        GLboolean lastDepthTest{GL_FALSE};
        GLboolean lastDepthMask{GL_TRUE};
        GLint lastDepthFunc{GL_LESS};
        GLboolean lastBlend{GL_FALSE};
        GLboolean lastCullFace{GL_FALSE};
        GLboolean lastScissorTest{GL_FALSE};
        GLint lastActiveTexture{GL_TEXTURE0};
        GLint lastTexture2D{0};
        GLfloat lastClearColor[4]{0,0,0,0};
        GLboolean lastColorMask[4]{GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};

        GLStateSaver()
        {
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &lastDrawFBO);
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &lastReadFBO);
            glGetIntegerv(GL_VIEWPORT, lastViewport);
            glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVAO);
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
            glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastElementBuffer);
            lastDepthTest = glIsEnabled(GL_DEPTH_TEST);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &lastDepthMask);
            glGetIntegerv(GL_DEPTH_FUNC, &lastDepthFunc);
            lastBlend = glIsEnabled(GL_BLEND);
            lastCullFace = glIsEnabled(GL_CULL_FACE);
            lastScissorTest = glIsEnabled(GL_SCISSOR_TEST);
            glGetIntegerv(GL_ACTIVE_TEXTURE, &lastActiveTexture);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture2D);
            glGetFloatv(GL_COLOR_CLEAR_VALUE, lastClearColor);
            glGetBooleanv(GL_COLOR_WRITEMASK, lastColorMask);
        }

        ~GLStateSaver()
        {
            // Restore everything in reverse order.
            glColorMask(lastColorMask[0], lastColorMask[1], lastColorMask[2], lastColorMask[3]);
            glClearColor(lastClearColor[0], lastClearColor[1], lastClearColor[2], lastClearColor[3]);
            glActiveTexture(lastActiveTexture);
            glBindTexture(GL_TEXTURE_2D, lastTexture2D);
            if (lastScissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
            if (lastCullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
            if (lastBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            glDepthFunc(lastDepthFunc);
            glDepthMask(lastDepthMask);
            if (lastDepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastElementBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
            glBindVertexArray(lastVAO);
            glUseProgram(lastProgram);
            // Restore draw and read framebuffers.
            // Use GL_FRAMEBUFFER when both target the same FBO (common case).
            if (lastDrawFBO == lastReadFBO)
                glBindFramebuffer(GL_FRAMEBUFFER, lastDrawFBO);
            else
            {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lastDrawFBO);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, lastReadFBO);
            }
            glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
        }
    };

    // ========================================================================
    // SceneRenderer
    // ========================================================================

    SceneRenderer::SceneRenderer() = default;

    SceneRenderer::~SceneRenderer()
    {
        if (m_ShaderProgram) glDeleteProgram(m_ShaderProgram);
        if (m_GridShaderProgram) glDeleteProgram(m_GridShaderProgram);
        if (m_GridVAO) glDeleteVertexArrays(1, &m_GridVAO);
        if (m_GridVBO) glDeleteBuffers(1, &m_GridVBO);
        if (m_CubeVAO) glDeleteVertexArrays(1, &m_CubeVAO);
        if (m_CubeVBO) glDeleteBuffers(1, &m_CubeVBO);
        if (m_CubeIBO) glDeleteBuffers(1, &m_CubeIBO);
    }

    void SceneRenderer::EnsureShaders()
    {
        if (m_ShadersCompiled) return;

        // Clear any pending GL errors.
        while (glGetError() != GL_NO_ERROR) {}

        // Compile into temporaries first to avoid leaking on retry.
        GLuint newMesh    = LinkProgram(kMeshVertexShader, kMeshFragmentShader);
        GLuint newGrid    = LinkProgram(kGridVertexShader, kGridFragmentShader);

        if (newMesh && newGrid)
        {
            // Both succeeded — replace old programs and mark compiled.
            if (m_ShaderProgram) glDeleteProgram(m_ShaderProgram);
            if (m_GridShaderProgram) glDeleteProgram(m_GridShaderProgram);

            m_ShaderProgram = newMesh;
            m_GridShaderProgram = newGrid;

            m_uViewProj    = glGetUniformLocation(m_ShaderProgram, "u_ViewProj");
            m_uModel       = glGetUniformLocation(m_ShaderProgram, "u_Model");
            m_uColor       = glGetUniformLocation(m_ShaderProgram, "u_Color");
            m_uViewProjGrid = glGetUniformLocation(m_GridShaderProgram, "u_ViewProj");
            m_uGridColor   = glGetUniformLocation(m_GridShaderProgram, "u_Color");

            ENGINE_LOG_INFO("SceneRenderer — both shaders compiled (mesh={}, grid={})",
                           m_ShaderProgram, m_GridShaderProgram);
            m_ShadersCompiled = true;
        }
        else
        {
            // At least one failed — clean up temporaries and retry next frame.
            if (newMesh) glDeleteProgram(newMesh);
            if (newGrid) glDeleteProgram(newGrid);

            if (!newMesh)
                ENGINE_LOG_ERROR("SceneRenderer — mesh shader FAILED to compile/link");
            if (!newGrid)
                ENGINE_LOG_ERROR("SceneRenderer — grid shader FAILED to compile/link");
        }
    }

    void SceneRenderer::EnsureGridGeometry()
    {
        if (m_GridVAO) return;

        const float gridSize = 50.0f;
        const float step = 1.0f;
        const int halfCells = static_cast<int>(gridSize / step);

        std::vector<Vec3> vertices;
        for (int i = -halfCells; i <= halfCells; ++i)
        {
            float pos = static_cast<float>(i) * step;
            vertices.push_back({-gridSize, 0.0f, pos});
            vertices.push_back({ gridSize, 0.0f, pos});
            vertices.push_back({pos, 0.0f, -gridSize});
            vertices.push_back({pos, 0.0f,  gridSize});
        }

        m_GridLineCount = static_cast<int>(vertices.size());

        glGenVertexArrays(1, &m_GridVAO);
        glGenBuffers(1, &m_GridVBO);
        glBindVertexArray(m_GridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3),
                     vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), nullptr);
        glBindVertexArray(0);

        ENGINE_LOG_INFO("SceneRenderer — grid geometry created ({} vertices)", m_GridLineCount);
    }

    void SceneRenderer::EnsureCubeGeometry()
    {
        if (m_CubeVAO) return;

        glGenVertexArrays(1, &m_CubeVAO);
        glGenBuffers(1, &m_CubeVBO);
        glGenBuffers(1, &m_CubeIBO);

        glBindVertexArray(m_CubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, Normal)));
        glBindVertexArray(0);

        ENGINE_LOG_INFO("SceneRenderer — cube geometry created");
    }

    void SceneRenderer::RenderGrid(const Mat4& viewProjection)
    {
        if (!m_GridShaderProgram || !m_GridVAO) return;

        // Grid should render on top of everything, with blending for fade.
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);  // Grid always visible — no depth test.
        glDisable(GL_BLEND);       // No blending — solid lines.

        glUseProgram(m_GridShaderProgram);
        glUniformMatrix4fv(m_uViewProjGrid, 1, GL_FALSE, &viewProjection[0][0]);
        glUniform4f(m_uGridColor, 0.5f, 0.5f, 0.5f, 1.0f);  // Solid gray, full alpha.

        glBindVertexArray(m_GridVAO);
        glDrawArrays(GL_LINES, 0, m_GridLineCount);
        glBindVertexArray(0);
        glUseProgram(0);

        ++m_DrawCalls;
    }

    void SceneRenderer::RenderMeshes(EditorContext& context, const Mat4& viewProjection)
    {
        if (!m_ShaderProgram || !m_CubeVAO) return;

        auto* scene = context.GetActiveScene();
        if (!scene) return;

        auto& registry = scene->GetRegistry();
        auto view = registry.View<MeshComponent, TransformComponent>();

        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glUseProgram(m_ShaderProgram);
        glUniformMatrix4fv(m_uViewProj, 1, GL_FALSE, &viewProjection[0][0]);

        glBindVertexArray(m_CubeVAO);

        u32 meshCount = 0;
        for (auto entity : view)
        {
            if (!registry.IsValid(entity))
                continue;

            auto& mesh = registry.GetComponent<MeshComponent>(entity);
            auto& tf   = registry.GetComponent<TransformComponent>(entity);

            if (registry.HasComponent<VisibilityComponent>(entity))
            {
                if (!registry.GetComponent<VisibilityComponent>(entity).IsVisible)
                    continue;
            }

            if (mesh.Type == MeshType::None)
                continue;

            Mat4 model = tf.WorldMatrix;
            glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);

            Vec4 color(0.8f, 0.8f, 0.8f, 1.0f);
            if (mesh.Type == MeshType::Cube)
                color = Vec4(0.8f, 0.6f, 0.3f, 1.0f);
            else if (mesh.Type == MeshType::Sphere)
                color = Vec4(0.3f, 0.7f, 0.9f, 1.0f);
            else if (mesh.Type == MeshType::Plane)
                color = Vec4(0.5f, 0.8f, 0.4f, 1.0f);

            glUniform4f(m_uColor, color.x, color.y, color.z, color.w);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            ++m_DrawCalls;
            ++meshCount;
        }

        glBindVertexArray(0);
        glUseProgram(0);

        // Only log when mesh count changes, not every frame.
        if (meshCount != m_LastMeshCount)
        {
            ENGINE_LOG_INFO("SceneRenderer — rendered {} meshes (was {})", meshCount, m_LastMeshCount);
            m_LastMeshCount = meshCount;
        }
    }

    void SceneRenderer::RenderScene(EditorContext& context, ViewportFramebuffer& framebuffer,
                                     const Mat4& viewProjection, bool renderGrid)
    {
        m_DrawCalls = 0;

        if (!framebuffer.IsValid())
            return;

        EnsureShaders();
        EnsureGridGeometry();
        EnsureCubeGeometry();

        // Save ALL GL state before we touch anything.
        // This is critical because we're rendering inside ImGui's frame.
        GLStateSaver stateSaver;

        // Bind our framebuffer.
        framebuffer.Bind();

        // Clear the framebuffer.
        auto bg = context.GetTheme().GetColor("background");
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up GL state for 3D rendering.
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Render meshes first (with depth test).
        RenderMeshes(context, viewProjection);

        // Render grid on top (without depth test, so it's always visible).
        if (renderGrid && context.GetPreferences().GridVisible)
        {
            RenderGrid(viewProjection);
        }

        // Restore is handled by GLStateSaver destructor.
    }

    void SceneRenderer::RenderGameView(EditorContext& context, ViewportFramebuffer& framebuffer)
    {
        m_DrawCalls = 0;

        if (!framebuffer.IsValid())
            return;

        EnsureShaders();
        EnsureGridGeometry();
        EnsureCubeGeometry();

        GLStateSaver stateSaver;

        framebuffer.Bind();

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

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
                if (!registry.IsValid(entity))
                    continue;

                auto& cam = registry.GetComponent<CameraComponent>(entity);
                auto& tf  = registry.GetComponent<TransformComponent>(entity);

                if (cam.Primary || !hasCamera)
                {
                    viewMatrix = glm::inverse(tf.WorldMatrix);
                    projMatrix = cam.GetProjectionMatrix(
                        static_cast<f32>(framebuffer.GetWidth()) /
                        static_cast<f32>(framebuffer.GetHeight()));
                    hasCamera = true;
                    if (cam.Primary) break;
                }
            }

            if (hasCamera)
            {
                Mat4 vp = projMatrix * viewMatrix;
                RenderMeshes(context, vp);
            }
            else
            {
                // Default camera.
                viewMatrix = engine::math::LookAt(Vec3(0.0f, 5.0f, 10.0f),
                                                   Vec3(0.0f, 0.0f, 0.0f),
                                                   Vec3(0.0f, 1.0f, 0.0f));
                projMatrix = engine::math::Perspective(glm::radians(60.0f),
                    static_cast<f32>(framebuffer.GetWidth()) /
                    static_cast<f32>(framebuffer.GetHeight()), 0.1f, 1000.0f);
                Mat4 vp = projMatrix * viewMatrix;
                RenderMeshes(context, vp);
            }
        }

        // Restore is handled by GLStateSaver destructor.
    }

} // namespace editor
