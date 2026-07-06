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
#include <cstring>

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
    // Shader sources
    // ========================================================================

    static const char* kMeshVertexShader = R"GLSL(
        #version 460 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        uniform mat4 u_ViewProj;
        uniform mat4 u_Model;
        out vec3 v_Normal;
        out vec3 v_WorldPos;
        void main()
        {
            vec4 worldPos = u_Model * vec4(a_Position, 1.0);
            gl_Position = u_ViewProj * worldPos;
            v_Normal = mat3(u_Model) * a_Normal;
            v_WorldPos = worldPos.xyz;
        }
    )GLSL";

    static const char* kMeshFragmentShader = R"GLSL(
        #version 460 core
        in vec3 v_Normal;
        in vec3 v_WorldPos;
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main()
        {
            vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
            vec3 normal = normalize(v_Normal);
            float diff = max(dot(normal, lightDir), 0.0);
            float ambient = 0.3;
            vec3 col = u_Color.rgb * (ambient + diff * 0.7);
            FragColor = vec4(col, u_Color.a);
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
        uniform vec4 u_GridColor;
        void main()
        {
            float fade = 1.0 - clamp(v_Dist / 50.0, 0.0, 1.0);
            FragColor = vec4(u_GridColor.rgb, u_GridColor.a * fade);
        }
    )GLSL";

    // ========================================================================
    // Cube data
    // ========================================================================

    struct Vertex
    {
        Vec3 Position;
        Vec3 Normal;
    };

    static const Vertex kCubeVertices[] = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        // Back face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        // Top face
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
        // Right face
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        // Left face
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    };

    static const u32 kCubeIndices[] = {
        0,  1,  2,   0,  2,  3,    // front
        4,  5,  6,   4,  6,  7,    // back
        8,  9,  10,  8,  10, 11,   // top
        12, 13, 14,  12, 14, 15,   // bottom
        16, 17, 18,  16, 18, 19,   // right
        20, 21, 22,  20, 22, 23,   // left
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
            char log[512];
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
            char log[512];
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

        GLenum err = glGetError();
        while (err != GL_NO_ERROR) err = glGetError();

        m_ShaderProgram = LinkProgram(kMeshVertexShader, kMeshFragmentShader);
        m_GridShaderProgram = LinkProgram(kGridVertexShader, kGridFragmentShader);

        if (m_ShaderProgram)
        {
            m_uViewProj = glGetUniformLocation(m_ShaderProgram, "u_ViewProj");
            m_uModel    = glGetUniformLocation(m_ShaderProgram, "u_Model");
            m_uColor    = glGetUniformLocation(m_ShaderProgram, "u_Color");
            if (m_uViewProj < 0) ENGINE_LOG_WARN("SceneRenderer — u_ViewProj not found in mesh shader");
            if (m_uModel < 0)    ENGINE_LOG_WARN("SceneRenderer — u_Model not found in mesh shader");
            if (m_uColor < 0)    ENGINE_LOG_WARN("SceneRenderer — u_Color not found in mesh shader");
            ENGINE_LOG_INFO("SceneRenderer — mesh shader compiled (program={})", m_ShaderProgram);
        }
        else
        {
            ENGINE_LOG_ERROR("SceneRenderer — mesh shader FAILED to compile/link");
        }

        if (m_GridShaderProgram)
        {
            m_uViewProjGrid = glGetUniformLocation(m_GridShaderProgram, "u_ViewProj");
            m_uGridColor    = glGetUniformLocation(m_GridShaderProgram, "u_GridColor");
            if (m_uViewProjGrid < 0) ENGINE_LOG_WARN("SceneRenderer — u_ViewProj not found in grid shader");
            if (m_uGridColor < 0)    ENGINE_LOG_WARN("SceneRenderer — u_GridColor not found in grid shader");
            ENGINE_LOG_INFO("SceneRenderer — grid shader compiled (program={})", m_GridShaderProgram);
        }
        else
        {
            ENGINE_LOG_ERROR("SceneRenderer — grid shader FAILED to compile/link");
        }

        // Only mark as compiled if BOTH programs succeeded.
        // Otherwise retry next frame so transient GL errors don't
        // leave us permanently stuck with no rendering.
        if (m_ShaderProgram && m_GridShaderProgram)
        {
            m_ShadersCompiled = true;
        }
        else
        {
            ENGINE_LOG_ERROR("SceneRenderer — shader compilation incomplete, will retry next frame");
            // Clean up any partially-created resources.
            if (m_ShaderProgram) { glDeleteProgram(m_ShaderProgram); m_ShaderProgram = 0; }
            if (m_GridShaderProgram) { glDeleteProgram(m_GridShaderProgram); m_GridShaderProgram = 0; }
        }

        err = glGetError();
        if (err != GL_NO_ERROR)
            ENGINE_LOG_ERROR("SceneRenderer — GL error 0x{:X} after EnsureShaders", err);
    }

    void SceneRenderer::EnsureGridGeometry()
    {
        if (m_GridVAO) return;

        // Generate a grid on the XZ plane centered at origin.
        const float gridSize = 50.0f;
        const float step = 1.0f;
        const int halfCells = static_cast<int>(gridSize / step);

        std::vector<Vec3> vertices;
        for (int i = -halfCells; i <= halfCells; ++i)
        {
            float pos = static_cast<float>(i) * step;

            // Line along X axis.
            vertices.push_back({-gridSize, 0.0f, pos});
            vertices.push_back({ gridSize, 0.0f, pos});

            // Line along Z axis.
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

        // Position attribute.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

        // Normal attribute.
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, Normal)));

        glBindVertexArray(0);
    }

    void SceneRenderer::RenderGrid(const Mat4& viewProjection)
    {
        if (!m_GridShaderProgram || !m_GridVAO) return;

        glUseProgram(m_GridShaderProgram);
        glUniformMatrix4fv(m_uViewProjGrid, 1, GL_FALSE, &viewProjection[0][0]);
        glUniform4f(m_uGridColor, 0.5f, 0.5f, 0.5f, 0.6f);

        glBindVertexArray(m_GridVAO);
        glDrawArrays(GL_LINES, 0, m_GridLineCount);
        glBindVertexArray(0);

        ++m_DrawCalls;
    }

    void SceneRenderer::RenderMeshes(EditorContext& context, const Mat4& viewProjection)
    {
        if (!m_ShaderProgram || !m_CubeVAO) return;

        auto* scene = context.GetActiveScene();
        if (!scene) return;

        auto& registry = scene->GetRegistry();
        auto view = registry.View<MeshComponent, TransformComponent>();

        glUseProgram(m_ShaderProgram);
        glUniformMatrix4fv(m_uViewProj, 1, GL_FALSE, &viewProjection[0][0]);

        glBindVertexArray(m_CubeVAO);

        for (auto entity : view)
        {
            // Validate entity before accessing components.
            if (!registry.IsValid(entity))
                continue;

            auto& mesh = registry.GetComponent<MeshComponent>(entity);
            auto& tf   = registry.GetComponent<TransformComponent>(entity);

            // Skip invisible entities.
            if (registry.HasComponent<VisibilityComponent>(entity))
            {
                if (!registry.GetComponent<VisibilityComponent>(entity).IsVisible)
                    continue;
            }

            // Skip non-mesh types (only render Cube, Sphere, Plane for now).
            if (mesh.Type == MeshType::None)
                continue;

            Mat4 model = tf.WorldMatrix;
            glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);

            // Color based on mesh type.
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
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void SceneRenderer::RenderScene(EditorContext& context, ViewportFramebuffer& framebuffer,
                                     const Mat4& viewProjection, bool renderGrid)
    {
        m_DrawCalls = 0;

        // Skip if framebuffer is not valid.
        if (!framebuffer.IsValid())
        {
            ENGINE_LOG_WARN("SceneRenderer::RenderScene — framebuffer invalid, skipping");
            return;
        }

        EnsureShaders();
        EnsureGridGeometry();
        EnsureCubeGeometry();

        // Cannot render without valid shader programs.
        if (!m_ShaderProgram && !m_GridShaderProgram)
        {
            ENGINE_LOG_WARN("SceneRenderer::RenderScene — no valid shader programs, skipping");
            return;
        }

        framebuffer.Bind();

        // Clear with editor background color.
        auto bg = context.GetTheme().GetColor("background");
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // Render grid if enabled.
        if (renderGrid && context.GetPreferences().GridVisible)
        {
            RenderGrid(viewProjection);
        }

        // Render entities with meshes.
        RenderMeshes(context, viewProjection);

        glDisable(GL_DEPTH_TEST);

        // Check for OpenGL errors.
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            ENGINE_LOG_ERROR("SceneRenderer — GL error 0x{:X} during scene render", err);
        }

        framebuffer.Unbind();

        if (m_DrawCalls == 0)
        {
            ENGINE_LOG_WARN("SceneRenderer — 0 draw calls produced. Grid={} shaders={}/{}",
                            renderGrid && context.GetPreferences().GridVisible,
                            (int)m_ShaderProgram, (int)m_GridShaderProgram);
        }
    }

    void SceneRenderer::RenderGameView(EditorContext& context, ViewportFramebuffer& framebuffer)
    {
        m_DrawCalls = 0;

        if (!framebuffer.IsValid())
            return;

        EnsureShaders();
        EnsureGridGeometry();
        EnsureCubeGeometry();

        framebuffer.Bind();

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

        glDisable(GL_DEPTH_TEST);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            ENGINE_LOG_ERROR("SceneRenderer — GL error 0x{:X} during game view render", err);
        }

        framebuffer.Unbind();
    }

} // namespace editor
