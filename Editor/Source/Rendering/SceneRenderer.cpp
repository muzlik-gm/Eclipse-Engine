// ============================================================================
// File: Editor/Source/Rendering/SceneRenderer.cpp
// Actually renders entities + grid into the viewport framebuffer.
// ============================================================================
#include "Editor/Rendering/SceneRenderer.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Picking/EntityPicking.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Camera/EditorCamera.h"
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
        #version 330 core
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
        #version 330 core
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
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        void main()
        {
            gl_Position = vec4(a_Position, 1.0);
        }
    )GLSL";

    static const char* kGridFragmentShader = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 u_Color;
        uniform vec3 u_CameraPosition;
        uniform mat4 u_InverseVP;
        uniform mat4 u_ViewProj;
        uniform vec2 u_ViewportSize;
        void main()
        {
            // Convert fragment coord to NDC (-1 to 1).
            vec2 ndc = (gl_FragCoord.xy / u_ViewportSize) * 2.0 - 1.0;

            // Compute world-space ray through this pixel at near and far.
            vec4 clipNear = vec4(ndc, -1.0, 1.0);
            vec4 clipFar  = vec4(ndc,  1.0, 1.0);

            vec4 worldNear = u_InverseVP * clipNear;
            vec4 worldFar  = u_InverseVP * clipFar;

            vec3 nearPos = worldNear.xyz / worldNear.w;
            vec3 farPos  = worldFar.xyz  / worldFar.w;

            vec3 dir = normalize(farPos - nearPos);

            // Intersect with XZ plane (y=0).
            float t = -nearPos.y / dir.y;
            if (t < 0.0) discard;

            vec3 hit = nearPos + dir * t;

            // Grid lines — 1 world-unit spacing with anti-aliasing.
            float cellSize = 1.0;
            vec2 gridPos = hit.xz / cellSize;
            vec2 gridFrac = abs(fract(gridPos) - 0.5);
            float distToLine = min(gridFrac.x, gridFrac.y);

            // Anti-alias: ~1 pixel width lines based on world-space derivative.
            vec2 grad = fwidth(hit.xz);
            float minGrad = min(grad.x, grad.y);
            float lineWidth = minGrad * 1.0;
            float lineAlpha = 1.0 - smoothstep(0.0, lineWidth, distToLine);

            // Distance fade: start fading at 50 units, gone by 200 units.
            float dist = length(hit.xz);
            float fade = 1.0 - smoothstep(50.0, 200.0, dist);

            // Write the true depth of the grid intersection point so that
            // meshes in front of the plane correctly occlude the grid.
            vec4 clipPos = u_ViewProj * vec4(hit, 1.0);
            if (clipPos.w > 0.0)
                gl_FragDepth = (clipPos.z / clipPos.w) * 0.5 + 0.5;

            FragColor = vec4(u_Color.rgb, u_Color.a * lineAlpha * fade);
        }
    )GLSL";

    // ========================================================================
    // Line shader — flat color, no lighting (for selection wireframe).
    // ========================================================================

    static const char* kLineVertexShader = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProj;
        uniform mat4 u_Model;
        void main()
        {
            gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
        }
    )GLSL";

    static const char* kLineFragmentShader = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main()
        {
            FragColor = u_Color;
        }
    )GLSL";

    // ========================================================================
    // Pick shader — flat colour from entity ID encoding, no lighting.
    // ========================================================================

    static const char* kPickVertexShader = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProj;
        uniform mat4 u_Model;
        void main()
        {
            gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
        }
    )GLSL";

    static const char* kPickFragmentShader = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 u_EntityColor;
        void main()
        {
            FragColor = vec4(u_EntityColor, 1.0);
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
    // Wireframe cube data — 12 edges as GL_LINES (24 vertices).
    // ========================================================================

    static const float kWireframeVertices[] = {
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,
        // Verticals
        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
    };
    static const int kWireframeVertexCount = 24;

    // ========================================================================
    // Gizmo axis data — 3 axis lines with arrowheads as GL_LINES.
    // 6 vertices per axis: 1 main line (2 verts) + 2 arrowhead lines (4 verts)
    // Total: 18 vertices.
    // ========================================================================

    static const float kGizmoAxisVertices[] = {
        // X axis (red): main line + 2 arrowhead lines
         0.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,   0.85f, 0.15f, 0.0f,
         1.0f,  0.0f,  0.0f,   0.85f,-0.15f, 0.0f,
        // Y axis (green): main line + 2 arrowhead lines
         0.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,   0.15f, 0.85f, 0.0f,
         0.0f,  1.0f,  0.0f,  -0.15f, 0.85f, 0.0f,
        // Z axis (blue): main line + 2 arrowhead lines
         0.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,   0.0f,  0.15f, 0.85f,
         0.0f,  0.0f,  1.0f,   0.0f,-0.15f, 0.85f,
    };
    // 18 vertices total, 6 per axis
    static const int kGizmoVertsPerAxis = 6;

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
        if (m_LineProgram) glDeleteProgram(m_LineProgram);
        if (m_PickProgram) glDeleteProgram(m_PickProgram);
        if (m_GridVAO) glDeleteVertexArrays(1, &m_GridVAO);
        if (m_GridVBO) glDeleteBuffers(1, &m_GridVBO);
        if (m_CubeVAO) glDeleteVertexArrays(1, &m_CubeVAO);
        if (m_CubeVBO) glDeleteBuffers(1, &m_CubeVBO);
        if (m_CubeIBO) glDeleteBuffers(1, &m_CubeIBO);
        if (m_WireframeVAO) glDeleteVertexArrays(1, &m_WireframeVAO);
        if (m_WireframeVBO) glDeleteBuffers(1, &m_WireframeVBO);
        if (m_GizmoVAO) glDeleteVertexArrays(1, &m_GizmoVAO);
        if (m_GizmoVBO) glDeleteBuffers(1, &m_GizmoVBO);
        if (m_CircleVAO) glDeleteVertexArrays(1, &m_CircleVAO);
        if (m_CircleVBO) glDeleteBuffers(1, &m_CircleVBO);
    }

    void SceneRenderer::EnsureShaders()
    {
        if (m_ShadersCompiled) return;

        // Clear any pending GL errors.
        while (glGetError() != GL_NO_ERROR) {}

        // Compile into temporaries first to avoid leaking on retry.
        GLuint newMesh    = LinkProgram(kMeshVertexShader, kMeshFragmentShader);
        GLuint newGrid    = LinkProgram(kGridVertexShader, kGridFragmentShader);
        GLuint newLine    = LinkProgram(kLineVertexShader, kLineFragmentShader);
        GLuint newPick    = LinkProgram(kPickVertexShader, kPickFragmentShader);

        if (newMesh && newGrid && newLine && newPick)
        {
            // All succeeded — replace old programs and mark compiled.
            if (m_ShaderProgram) glDeleteProgram(m_ShaderProgram);
            if (m_GridShaderProgram) glDeleteProgram(m_GridShaderProgram);
            if (m_LineProgram) glDeleteProgram(m_LineProgram);
            if (m_PickProgram) glDeleteProgram(m_PickProgram);

            m_ShaderProgram = newMesh;
            m_GridShaderProgram = newGrid;
            m_LineProgram = newLine;
            m_PickProgram = newPick;

            // Mesh shader uniforms.
            m_uViewProj    = glGetUniformLocation(m_ShaderProgram, "u_ViewProj");
            m_uModel       = glGetUniformLocation(m_ShaderProgram, "u_Model");
            m_uColor       = glGetUniformLocation(m_ShaderProgram, "u_Color");

            // Grid shader uniforms.
            m_uViewProjGrid = glGetUniformLocation(m_GridShaderProgram, "u_ViewProj");
            m_uGridColor   = glGetUniformLocation(m_GridShaderProgram, "u_Color");
            m_uGridCamPos  = glGetUniformLocation(m_GridShaderProgram, "u_CameraPosition");
            m_uGridInvVP   = glGetUniformLocation(m_GridShaderProgram, "u_InverseVP");
            m_uGridViewportSize = glGetUniformLocation(m_GridShaderProgram, "u_ViewportSize");

            // Line shader uniforms.
            m_uLineViewProj = glGetUniformLocation(m_LineProgram, "u_ViewProj");
            m_uLineModel    = glGetUniformLocation(m_LineProgram, "u_Model");
            // m_uLineColor is set via glUniform4f directly in RenderOutline.

            // Pick shader uniforms.
            m_uPickViewProj    = glGetUniformLocation(m_PickProgram, "u_ViewProj");
            m_uPickModel       = glGetUniformLocation(m_PickProgram, "u_Model");
            m_uPickEntityColor = glGetUniformLocation(m_PickProgram, "u_EntityColor");

            ENGINE_LOG_INFO("SceneRenderer — all shaders compiled (mesh={}, grid={}, line={}, pick={})",
                           m_ShaderProgram, m_GridShaderProgram, m_LineProgram, m_PickProgram);
            m_ShadersCompiled = true;
        }
        else
        {
            // At least one failed — clean up temporaries and retry next frame.
            if (newMesh) glDeleteProgram(newMesh);
            if (newGrid) glDeleteProgram(newGrid);
            if (newLine) glDeleteProgram(newLine);
            if (newPick) glDeleteProgram(newPick);

            if (!newMesh)
                ENGINE_LOG_ERROR("SceneRenderer — mesh shader FAILED to compile/link");
            if (!newGrid)
                ENGINE_LOG_ERROR("SceneRenderer — grid shader FAILED to compile/link");
            if (!newLine)
                ENGINE_LOG_ERROR("SceneRenderer — line shader FAILED to compile/link");
            if (!newPick)
                ENGINE_LOG_ERROR("SceneRenderer — pick shader FAILED to compile/link");
        }
    }

    void SceneRenderer::EnsureGridGeometry()
    {
        if (m_GridVAO) return;

        // Full-screen NDC quad (triangle strip) for screen-space infinite grid.
        float vertices[] = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f
        };

        m_GridLineCount = 4;

        glGenVertexArrays(1, &m_GridVAO);
        glGenBuffers(1, &m_GridVBO);
        glBindVertexArray(m_GridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glBindVertexArray(0);

        ENGINE_LOG_INFO("SceneRenderer — screen-space grid geometry created");
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

    void SceneRenderer::EnsureWireframeGeometry()
    {
        if (m_WireframeVAO) return;

        glGenVertexArrays(1, &m_WireframeVAO);
        glGenBuffers(1, &m_WireframeVBO);

        glBindVertexArray(m_WireframeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_WireframeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kWireframeVertices), kWireframeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glBindVertexArray(0);

        ENGINE_LOG_INFO("SceneRenderer — wireframe geometry created");
    }

    void SceneRenderer::EnsureGizmoGeometry()
    {
        if (m_GizmoVAO) return;

        glGenVertexArrays(1, &m_GizmoVAO);
        glGenBuffers(1, &m_GizmoVBO);

        glBindVertexArray(m_GizmoVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_GizmoVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kGizmoAxisVertices), kGizmoAxisVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glBindVertexArray(0);

        // Generate circle geometry for rotation gizmo (3 circles, 64 segments each).
        const int segs = 64;
        std::vector<float> circleVerts;
        auto addCircle = [&](const Vec3& axis1, const Vec3& axis2, float radius)
        {
            for (int i = 0; i <= segs; ++i)
            {
                float a = static_cast<float>(i) / static_cast<float>(segs) * 2.0f * 3.14159265f;
                Vec3 p = axis1 * (radius * cosf(a)) + axis2 * (radius * sinf(a));
                circleVerts.push_back(p.x);
                circleVerts.push_back(p.y);
                circleVerts.push_back(p.z);
            }
        };
        // XY circle (Z-axis rotation handle)
        addCircle(Vec3(1,0,0), Vec3(0,1,0), 1.8f);
        // XZ circle (Y-axis rotation handle)
        addCircle(Vec3(1,0,0), Vec3(0,0,1), 1.8f);
        // YZ circle (X-axis rotation handle)
        addCircle(Vec3(0,1,0), Vec3(0,0,1), 1.8f);

        glGenVertexArrays(1, &m_CircleVAO);
        glGenBuffers(1, &m_CircleVBO);
        glBindVertexArray(m_CircleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_CircleVBO);
        glBufferData(GL_ARRAY_BUFFER, circleVerts.size() * sizeof(float),
                     circleVerts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glBindVertexArray(0);

        m_CircleVertexCount = (segs + 1) * 3; // 3 circles, (segs+1) verts each (ring, not loop)
        ENGINE_LOG_INFO("SceneRenderer — gizmo geometry created");
    }

    void SceneRenderer::RenderGrid(const Mat4& viewProjection, const Vec3& cameraPos,
                                    f32 viewportW, f32 viewportH)
    {
        if (!m_GridShaderProgram || !m_GridVAO) return;

        // Grid renders with depth test (LEQUAL) so meshes occlude behind-grid lines,
        // and alpha blending so the distance fade from the shader is visible.
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);  // Don't write depth — grid is screen-space.

        glUseProgram(m_GridShaderProgram);
        glUniform4f(m_uGridColor, 0.5f, 0.5f, 0.5f, 1.0f);  // Solid gray, full alpha.

        // Compute inverse view-projection for fragment shader ray reconstruction.
        Mat4 invVP = glm::inverse(viewProjection);
        glUniformMatrix4fv(m_uGridInvVP, 1, GL_FALSE, &invVP[0][0]);
        glUniformMatrix4fv(m_uViewProjGrid, 1, GL_FALSE, &viewProjection[0][0]);
        glUniform3fv(m_uGridCamPos, 1, &cameraPos[0]);
        glUniform2f(m_uGridViewportSize, viewportW, viewportH);

        glBindVertexArray(m_GridVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_GridLineCount);
        glBindVertexArray(0);
        glUseProgram(0);

        glDepthMask(GL_TRUE);

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
        EnsureWireframeGeometry();
        EnsureGizmoGeometry();

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

        // Render grid — screen-space infinite grid shader.
        if (renderGrid && context.GetPreferences().GridVisible)
        {
            auto& cam = context.GetCamera();
            f32 w = static_cast<f32>(framebuffer.GetWidth());
            f32 h = static_cast<f32>(framebuffer.GetHeight());
            RenderGrid(viewProjection, cam.GetPosition(), w, h);
        }

        // Render selection outline for the selected entity.
        // Must happen inside GLStateSaver scope — draws into the FBO.
        if (!context.GetSelection().IsEmpty())
            RenderSelectionOutline(context, viewProjection);

        // Render gizmo for the selected entity.
        RenderGizmos(context, viewProjection);

        // Restore is handled by GLStateSaver destructor.
    }

    void SceneRenderer::RenderSelectionOutline(EditorContext& context,
                                                const Mat4& viewProjection)
    {
        if (!m_LineProgram || !m_WireframeVAO)
            return;

        auto entity = context.GetSelection().GetPrimaryEntity();
        if (entity == engine::ecs::Invalid)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
            return;

        auto& registry = scene->GetRegistry();
        if (!registry.HasComponent<TransformComponent>(entity))
            return;

        auto& tf = registry.GetComponent<TransformComponent>(entity);

        // Selection outline colour: warm orange.
        Vec4 outlineColor(1.0f, 0.65f, 0.0f, 1.0f);

        // Use the entity's world matrix directly.
        glUseProgram(m_LineProgram);
        glUniformMatrix4fv(m_uLineViewProj, 1, GL_FALSE, &viewProjection[0][0]);
        glUniformMatrix4fv(m_uLineModel, 1, GL_FALSE, &tf.WorldMatrix[0][0]);
        GLint uLineColor = glGetUniformLocation(m_LineProgram, "u_Color");
        glUniform4f(uLineColor, outlineColor.x, outlineColor.y, outlineColor.z, outlineColor.w);

        // Draw without depth test so the outline is always visible.
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(m_WireframeVAO);
        glDrawArrays(GL_LINES, 0, kWireframeVertexCount);
        glBindVertexArray(0);
        glUseProgram(0);

        ++m_DrawCalls;
    }

    void SceneRenderer::RenderGizmos(EditorContext& context,
                                      const Mat4& viewProjection)
    {
        if (!m_LineProgram || !m_GizmoVAO)
            return;

        auto entity = context.GetSelection().GetPrimaryEntity();
        if (entity == engine::ecs::Invalid)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
            return;

        auto& registry = scene->GetRegistry();
        if (!registry.HasComponent<TransformComponent>(entity))
            return;

        auto& gizmos = context.GetGizmos();
        GizmoMode mode = gizmos.GetMode();
        if (mode == GizmoMode::None)
            return;

        auto& tf = registry.GetComponent<TransformComponent>(entity);

        // Extract translation from the world matrix.
        Vec3 pos(tf.WorldMatrix[3][0], tf.WorldMatrix[3][1], tf.WorldMatrix[3][2]);

        // Build a model matrix that positions the gizmo at the entity.
        Mat4 model(1.0f);
        model[3][0] = pos.x;
        model[3][1] = pos.y;
        model[3][2] = pos.z;

        // Axis colors.
        Vec4 red(1.0f, 0.2f, 0.2f, 1.0f);
        Vec4 green(0.2f, 1.0f, 0.3f, 1.0f);
        Vec4 blue(0.2f, 0.5f, 1.0f, 1.0f);
        Vec4 yellow(1.0f, 0.9f, 0.1f, 1.0f);

        // Determine gizmo scale based on distance from camera.
        f32 dist = glm::length(pos - context.GetCamera().GetPosition());
        f32 scale = dist * 0.15f;
        scale = std::max(scale, 0.5f);

        // Scale the model matrix.
        Mat4 scaledModel = glm::scale(model, Vec3(scale));

        glUseProgram(m_LineProgram);
        glUniformMatrix4fv(m_uLineViewProj, 1, GL_FALSE, &viewProjection[0][0]);

        // Disable depth test so gizmo is always visible on top.
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(m_GizmoVAO);

        if (mode == GizmoMode::Translate)
        {
            // Draw X axis (red) — first 6 vertices.
            glUniformMatrix4fv(m_uLineModel, 1, GL_FALSE, &scaledModel[0][0]);
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        red.x, red.y, red.z, red.w);
            glDrawArrays(GL_LINES, 0, 6);

            // Draw Y axis (green) — next 6 vertices.
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        green.x, green.y, green.z, green.w);
            glDrawArrays(GL_LINES, 6, 6);

            // Draw Z axis (blue) — next 6 vertices.
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        blue.x, blue.y, blue.z, blue.w);
            glDrawArrays(GL_LINES, 12, 6);

            ++m_DrawCalls;
        }
        else if (mode == GizmoMode::Rotate)
        {
            if (!m_CircleVAO) return;

            // Must set the model matrix — otherwise it keeps the previous
            // binding (the selection outline's tf.WorldMatrix) and the gizmo
            // circles would rotate/scale together with the object.
            glUniformMatrix4fv(m_uLineModel, 1, GL_FALSE, &scaledModel[0][0]);

            glBindVertexArray(m_CircleVAO);
            // Draw 3 circles with axis-aligned colors.
            int vertsPerCircle = m_CircleVertexCount / 3;

            // XY circle (Z axis) — blue
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        blue.x, blue.y, blue.z, blue.w);
            glDrawArrays(GL_LINE_STRIP, 0, vertsPerCircle);

            // XZ circle (Y axis) — green
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        green.x, green.y, green.z, green.w);
            glDrawArrays(GL_LINE_STRIP, vertsPerCircle, vertsPerCircle);

            // YZ circle (X axis) — red
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        red.x, red.y, red.z, red.w);
            glDrawArrays(GL_LINE_STRIP, vertsPerCircle * 2, vertsPerCircle);

            ++m_DrawCalls;
        }
        else if (mode == GizmoMode::Scale)
        {
            // Draw X axis (red) — first 6 vertices.
            glUniformMatrix4fv(m_uLineModel, 1, GL_FALSE, &scaledModel[0][0]);
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        red.x, red.y, red.z, red.w);
            glDrawArrays(GL_LINES, 0, 2);  // Just the main line, no arrowhead

            // Draw Y axis (green).
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        green.x, green.y, green.z, green.w);
            glDrawArrays(GL_LINES, 6, 2);

            // Draw Z axis (blue).
            glUniform4f(glGetUniformLocation(m_LineProgram, "u_Color"),
                        blue.x, blue.y, blue.z, blue.w);
            glDrawArrays(GL_LINES, 12, 2);

            ++m_DrawCalls;
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void SceneRenderer::RenderPicking(EditorContext& context,
                                       const Mat4& viewProjection)
    {
        if (!m_PickProgram || !m_CubeVAO)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
            return;

        auto& registry = scene->GetRegistry();
        auto view = registry.View<MeshComponent, TransformComponent>();

        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glUseProgram(m_PickProgram);
        glUniformMatrix4fv(m_uPickViewProj, 1, GL_FALSE, &viewProjection[0][0]);

        glBindVertexArray(m_CubeVAO);

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

            // Encode entity ID into an RGB color.
            Vec3 pickColor = EntityPicking::EncodeEntityID(entity);
            glUniform3f(m_uPickEntityColor, pickColor.x, pickColor.y, pickColor.z);

            glUniformMatrix4fv(m_uPickModel, 1, GL_FALSE, &tf.WorldMatrix[0][0]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            ++m_DrawCalls;
        }

        // Unbind cube VAO — switch to gizmo geometry for entity proxy picking.
        glBindVertexArray(0);

        // Render gizmo axes in pick pass so clicking a gizmo selects the entity.
        auto selectedEntity = context.GetSelection().GetPrimaryEntity();
        if (selectedEntity != engine::ecs::Invalid
            && registry.HasComponent<TransformComponent>(selectedEntity)
            && m_GizmoVAO)
        {
            auto& selectedTf = registry.GetComponent<TransformComponent>(selectedEntity);

            // Encode the selected entity's pick color for gizmo proxy picking.
            Vec3 pickColor = EntityPicking::EncodeEntityID(selectedEntity);
            glUniform3f(m_uPickEntityColor, pickColor.x, pickColor.y, pickColor.z);

            // Position gizmo at entity world position.
            Vec3 pos(selectedTf.WorldMatrix[3][0],
                     selectedTf.WorldMatrix[3][1],
                     selectedTf.WorldMatrix[3][2]);
            Mat4 gizmoModel(1.0f);
            gizmoModel[3][0] = pos.x;
            gizmoModel[3][1] = pos.y;
            gizmoModel[3][2] = pos.z;

            // Scale gizmo based on camera distance.
            f32 dist = glm::length(pos - context.GetCamera().GetPosition());
            f32 scale = std::max(dist * 0.15f, 0.5f);
            gizmoModel = glm::scale(gizmoModel, Vec3(scale));

            glUniformMatrix4fv(m_uPickModel, 1, GL_FALSE, &gizmoModel[0][0]);

            // Draw gizmo on top (no depth test).
            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(m_GizmoVAO);
            glDrawArrays(GL_LINES, 0, 18);  // 3 axes × 6 verts each (line + arrowhead)
            glBindVertexArray(0);

            ++m_DrawCalls;
        }

        glUseProgram(0);
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
