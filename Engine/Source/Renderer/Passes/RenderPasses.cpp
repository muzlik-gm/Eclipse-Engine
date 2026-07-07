// ============================================================================
// File: Engine/Source/Renderer/Passes/RenderPasses.cpp
// All render pass implementations.
// ============================================================================
#include "Engine/Renderer/Passes/IRenderPass.h"
#include "Engine/Renderer/Core/RenderSettings.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>
#include <vector>
#include <cmath>

namespace engine::renderer {

    using engine::core::u32;
    using engine::math::Mat4;
    using engine::math::Vec3;
    using engine::math::Vec4;

    // ========================================================================
    // Shader sources (shared by GeometryPass and DebugRenderPass)
    // ========================================================================

    static const char* kMeshVS = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        uniform mat4 u_ViewProj;
        uniform mat4 u_Model;
        out vec3 v_Normal;
        void main() {
            gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
            v_Normal = mat3(u_Model) * a_Normal;
        }
    )GLSL";

    static const char* kMeshFS = R"GLSL(
        #version 330 core
        in vec3 v_Normal;
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main() {
            vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
            float diff = max(dot(normalize(v_Normal), lightDir), 0.0);
            FragColor = vec4(u_Color.rgb * (0.3 + diff * 0.7), u_Color.a);
        }
    )GLSL";

    static const char* kLineVS = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProj;
        out float v_Dist;
        void main() {
            gl_Position = u_ViewProj * vec4(a_Position, 1.0);
            v_Dist = length(a_Position.xz);
        }
    )GLSL";

    static const char* kLineFS = R"GLSL(
        #version 330 core
        in float v_Dist;
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main() {
            float fade = 1.0 - clamp(v_Dist / 50.0, 0.0, 1.0);
            FragColor = vec4(u_Color.rgb, u_Color.a * fade);
        }
    )GLSL";

    // ========================================================================
    // Shader helper
    // ========================================================================

    static GLuint CompileShader(GLenum type, const char* src)
    {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = GL_FALSE;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
            ENGINE_LOG_ERROR("RenderPass shader error: {}", log); glDeleteShader(s); return 0; }
        return s;
    }

    static GLuint LinkProgram(const char* vs, const char* fs)
    {
        GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
        GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
        if (!v || !f) { if(v) glDeleteShader(v); if(f) glDeleteShader(f); return 0; }
        GLuint p = glCreateProgram();
        glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
        GLint ok = GL_FALSE; glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok) { char log[512]; glGetProgramInfoLog(p, 512, nullptr, log);
            ENGINE_LOG_ERROR("RenderPass link error: {}", log); glDeleteProgram(p); p = 0; }
        glDeleteShader(v); glDeleteShader(f);
        return p;
    }

    // ========================================================================
    // Shared cube geometry
    // ========================================================================

    struct Vertex { Vec3 Position; Vec3 Normal; };

    static const Vertex kCubeVerts[] = {
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
    // GeometryPass
    // ========================================================================

    static GLuint s_MeshShader = 0;
    static GLuint s_CubeVAO = 0, s_CubeVBO = 0, s_CubeIBO = 0;
    static GLint  s_uViewProj = -1, s_uModel = -1, s_uColor = -1;

    void GeometryPass::Initialize()
    {
        if (s_MeshShader) return;
        s_MeshShader = LinkProgram(kMeshVS, kMeshFS);
        if (s_MeshShader) {
            s_uViewProj = glGetUniformLocation(s_MeshShader, "u_ViewProj");
            s_uModel    = glGetUniformLocation(s_MeshShader, "u_Model");
            s_uColor    = glGetUniformLocation(s_MeshShader, "u_Color");
        }

        glGenVertexArrays(1, &s_CubeVAO);
        glGenBuffers(1, &s_CubeVBO);
        glGenBuffers(1, &s_CubeIBO);
        glBindVertexArray(s_CubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_CubeIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, Normal)));
        glBindVertexArray(0);

        ENGINE_LOG_INFO("GeometryPass — initialized");
    }

    void GeometryPass::Execute(const RenderPassContext& ctx)
    {
        if (!s_MeshShader || !s_CubeVAO || !ctx.Queues) return;

        glEnable(GL_DEPTH_TEST);

        glUseProgram(s_MeshShader);
        glUniformMatrix4fv(s_uViewProj, 1, GL_FALSE, &ctx.ViewProjection[0][0]);
        glBindVertexArray(s_CubeVAO);

        const auto& entries = ctx.Queues->GetQueue(RenderQueueType::Opaque).GetEntries();
        for (const auto& e : entries)
        {
            if (!e.Visible) continue;

            glUniformMatrix4fv(s_uModel, 1, GL_FALSE, &e.WorldMatrix[0][0]);

            // Color based on mesh type.
            Vec4 color(0.8f, 0.8f, 0.8f, 1.0f);
            if (e.MeshType == 2) color = Vec4(0.8f, 0.6f, 0.3f, 1.0f); // Cube
            else if (e.MeshType == 3) color = Vec4(0.3f, 0.7f, 0.9f, 1.0f); // Sphere
            else if (e.MeshType == 4) color = Vec4(0.5f, 0.8f, 0.4f, 1.0f); // Plane
            glUniform4f(s_uColor, color.x, color.y, color.z, color.w);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_DEPTH_TEST);
    }

    void GeometryPass::Shutdown()
    {
        if (s_CubeVAO) { glDeleteVertexArrays(1, &s_CubeVAO); s_CubeVAO = 0; }
        if (s_CubeVBO) { glDeleteBuffers(1, &s_CubeVBO); s_CubeVBO = 0; }
        if (s_CubeIBO) { glDeleteBuffers(1, &s_CubeIBO); s_CubeIBO = 0; }
        if (s_MeshShader) { glDeleteProgram(s_MeshShader); s_MeshShader = 0; }
    }

    // ========================================================================
    // SkyPass — draws a gradient sky background
    // ========================================================================

    static GLuint s_SkyVAO = 0, s_SkyVBO = 0;
    static GLuint s_SkyShader = 0;

    void SkyPass::Initialize()
    {
        if (s_SkyShader) return;

        const char* skyVS = R"GLSL(
            #version 330 core
            out vec3 v_Dir;
            void main() {
                vec2 pos = vec2((gl_VertexID == 2) ? 3.0 : -1.0,
                                (gl_VertexID == 1) ? 3.0 : -1.0);
                gl_Position = vec4(pos, 0.999, 1.0);
                v_Dir = vec3(pos, -1.0);
            }
        )GLSL";

        const char* skyFS = R"GLSL(
            #version 330 core
            in vec3 v_Dir;
            out vec4 FragColor;
            void main() {
                float t = clamp(normalize(v_Dir).y * 0.5 + 0.5, 0.0, 1.0);
                vec3 top = vec3(0.2, 0.4, 0.8);
                vec3 bottom = vec3(0.5, 0.6, 0.9);
                FragColor = vec4(mix(bottom, top, t), 1.0);
            }
        )GLSL";

        s_SkyShader = LinkProgram(skyVS, skyFS);
        glGenVertexArrays(1, &s_SkyVAO);
        ENGINE_LOG_INFO("SkyPass — initialized");
    }

    void SkyPass::Execute(const RenderPassContext& /*ctx*/)
    {
        if (!s_SkyShader) return;

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glUseProgram(s_SkyShader);
        glBindVertexArray(s_SkyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }

    void SkyPass::Shutdown()
    {
        if (s_SkyVAO) { glDeleteVertexArrays(1, &s_SkyVAO); s_SkyVAO = 0; }
        if (s_SkyShader) { glDeleteProgram(s_SkyShader); s_SkyShader = 0; }
    }

    // ========================================================================
    // TransparentPass
    // ========================================================================

    void TransparentPass::Initialize() { ENGINE_LOG_INFO("TransparentPass — initialized"); }

    void TransparentPass::Execute(const RenderPassContext& ctx)
    {
        if (!s_MeshShader || !s_CubeVAO || !ctx.Queues) return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glUseProgram(s_MeshShader);
        glUniformMatrix4fv(s_uViewProj, 1, GL_FALSE, &ctx.ViewProjection[0][0]);
        glBindVertexArray(s_CubeVAO);

        const auto& entries = ctx.Queues->GetQueue(RenderQueueType::Transparent).GetEntries();
        for (const auto& e : entries)
        {
            if (!e.Visible) continue;
            glUniformMatrix4fv(s_uModel, 1, GL_FALSE, &e.WorldMatrix[0][0]);
            glUniform4f(s_uColor, 0.7f, 0.7f, 0.9f, 0.5f);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void TransparentPass::Shutdown() {}

    // ========================================================================
    // DebugRenderPass — renders the grid
    // ========================================================================

    static GLuint s_GridShader = 0;
    static GLuint s_GridVAO = 0, s_GridVBO = 0;
    static int    s_GridLineCount = 0;
    static GLint  s_uViewProjGrid = -1, s_uGridColor = -1;

    void DebugRenderPass::Initialize()
    {
        if (s_GridShader) return;

        s_GridShader = LinkProgram(kLineVS, kLineFS);
        if (s_GridShader) {
            s_uViewProjGrid = glGetUniformLocation(s_GridShader, "u_ViewProj");
            s_uGridColor    = glGetUniformLocation(s_GridShader, "u_Color");
        }

        // Generate grid geometry.
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
        s_GridLineCount = static_cast<int>(vertices.size());

        glGenVertexArrays(1, &s_GridVAO);
        glGenBuffers(1, &s_GridVBO);
        glBindVertexArray(s_GridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_GridVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3),
                     vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), nullptr);
        glBindVertexArray(0);

        ENGINE_LOG_INFO("DebugRenderPass — initialized");
    }

    void DebugRenderPass::Execute(const RenderPassContext& ctx)
    {
        if (!s_GridShader || !s_GridVAO) return;
        if (!ctx.Settings || !ctx.Settings->ShowGrid) return;

        glUseProgram(s_GridShader);
        glUniformMatrix4fv(s_uViewProjGrid, 1, GL_FALSE, &ctx.ViewProjection[0][0]);
        glUniform4f(s_uGridColor, 0.5f, 0.5f, 0.5f, 0.6f);

        glBindVertexArray(s_GridVAO);
        glDrawArrays(GL_LINES, 0, s_GridLineCount);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void DebugRenderPass::Shutdown()
    {
        if (s_GridVAO) { glDeleteVertexArrays(1, &s_GridVAO); s_GridVAO = 0; }
        if (s_GridVBO) { glDeleteBuffers(1, &s_GridVBO); s_GridVBO = 0; }
        if (s_GridShader) { glDeleteProgram(s_GridShader); s_GridShader = 0; }
    }

    // ========================================================================
    // UIPass
    // ========================================================================

    void UIPass::Initialize() { ENGINE_LOG_INFO("UIPass — initialized"); }

    void UIPass::Execute(const RenderPassContext& /*ctx*/)
    {
        // UI is rendered by ImGui — this pass is a framework placeholder
        // for future engine-internal UI rendering.
    }

    void UIPass::Shutdown() {}

    // ========================================================================
    // PresentPass
    // ========================================================================

    void PresentPass::Initialize() { ENGINE_LOG_INFO("PresentPass — initialized"); }

    void PresentPass::Execute(const RenderPassContext& /*ctx*/)
    {
        // Present is handled by the framebuffer swap in the editor/render loop.
        // This pass exists for framework completeness and future post-processing.
        glFlush();
    }

    void PresentPass::Shutdown() {}

} // namespace engine::renderer
