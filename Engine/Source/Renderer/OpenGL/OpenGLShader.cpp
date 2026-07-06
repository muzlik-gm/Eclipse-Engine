// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLShader.cpp
// ============================================================================
#include "OpenGLShader.h"
#include "OpenGLTypes.h"
#include "OpenGLDebugLayer.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>

#include <unordered_map>

namespace engine::opengl {

    using namespace engine::rhi;
    using engine::core::u32;
    using engine::core::i32;

    // ========================================================================
    // OpenGLShaderModule
    // ========================================================================

    OpenGLShaderModule::OpenGLShaderModule(const ShaderStageDescription& desc,
                                            OpenGLDebugLayer* debugLayer)
        : m_Stage(desc.Stage)
        , m_Language(desc.Language)
        , m_Name(desc.FilePath.empty() ? "ShaderModule" : desc.FilePath)
        , m_EntryPoint(desc.EntryPoint)
        , m_FilePath(desc.FilePath)
        , m_Source(desc.Source)
        , m_Bytecode(desc.Bytecode)
        , m_DebugLayer(debugLayer)
    {
        Compile();

        if (m_DebugLayer && m_Handle != 0 && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_SHADER, m_Handle, m_Name);
        }
    }

    OpenGLShaderModule::~OpenGLShaderModule()
    {
        if (m_Handle != 0)
        {
            glDeleteShader(m_Handle);
            m_Handle = 0;
        }
    }

    void OpenGLShaderModule::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_SHADER, m_Handle, m_Name);
    }

    void OpenGLShaderModule::Compile()
    {
        if (m_Source.empty() && m_Bytecode.empty())
        {
            m_CompileLog = "No source or bytecode provided";
            ENGINE_LOG_ERROR("OpenGLShaderModule — no source or bytecode");
            return;
        }

        // SPIR-V bytecode path (future — requires GL_ARB_gl_spirv).
        if (!m_Bytecode.empty())
        {
            m_CompileLog = "SPIR-V bytecode not yet supported in OpenGL backend";
            ENGINE_LOG_ERROR("OpenGLShaderModule — SPIR-V not yet supported");
            return;
        }

        // Preprocess source (#include resolution).
        m_ProcessedSource = m_Preprocessor.Process(m_Source, m_FilePath);

        const GLuint shaderType = ToGLShaderType(m_Stage);
        m_Handle = glCreateShader(shaderType);
        if (m_Handle == 0)
        {
            m_CompileLog = "glCreateShader failed";
            ENGINE_LOG_ERROR("OpenGLShaderModule — glCreateShader failed");
            return;
        }

        const char* src = m_ProcessedSource.c_str();
        const GLint len  = static_cast<GLint>(m_ProcessedSource.size());
        glShaderSource(m_Handle, 1, &src, &len);
        glCompileShader(m_Handle);

        GLint success = GL_FALSE;
        glGetShaderiv(m_Handle, GL_COMPILE_STATUS, &success);

        GLint logLen = 0;
        glGetShaderiv(m_Handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            m_CompileLog.resize(static_cast<usize>(logLen));
            glGetShaderInfoLog(m_Handle, logLen, nullptr, m_CompileLog.data());
        }

        if (success != GL_TRUE)
        {
            ENGINE_LOG_ERROR("OpenGLShaderModule — compile failed for '{}':\n{}",
                             m_FilePath, m_CompileLog);
            m_Compiled = false;
        }
        else
        {
            ENGINE_LOG_DEBUG("OpenGLShaderModule — compiled '{}' (handle={})",
                             m_FilePath, m_Handle);
            m_Compiled = true;
        }
    }

    // ========================================================================
    // OpenGLShader
    // ========================================================================

    OpenGLShader::OpenGLShader(const ShaderDescription& desc, OpenGLDebugLayer* debugLayer)
        : m_Description(desc)
        , m_Name(desc.Name)
        , m_DebugLayer(debugLayer)
    {
        // Create and compile each module.
        for (const auto& stageDesc : desc.Stages)
        {
            auto module = std::make_unique<OpenGLShaderModule>(stageDesc, debugLayer);
            m_ModulePtrs.push_back(module.get());
            m_Modules.push_back(std::move(module));
        }

        // Link into a program.
        m_Handle = glCreateProgram();
        if (m_Handle == 0)
        {
            m_LinkLog = "glCreateProgram failed";
            ENGINE_LOG_ERROR("OpenGLShader — glCreateProgram failed");
            return;
        }

        // Attach all compiled modules.
        bool allCompiled = true;
        for (const auto& mod : m_Modules)
        {
            if (!mod->IsCompiled())
            {
                allCompiled = false;
                continue;
            }
            glAttachShader(m_Handle, mod->GetHandle());
        }

        if (!allCompiled)
        {
            ENGINE_LOG_ERROR("OpenGLShader — one or more modules failed to compile, skipping link");
            return;
        }

        Link();

        if (m_Linked)
        {
            Reflect();
        }

        if (m_DebugLayer && m_Handle != 0 && !m_Name.empty())
        {
            m_DebugLayer->SetObjectLabel(GL_PROGRAM, m_Handle, m_Name);
        }
    }

    OpenGLShader::~OpenGLShader()
    {
        // Detach and delete modules.
        if (m_Handle != 0)
        {
            for (const auto& mod : m_Modules)
            {
                if (mod->GetHandle() != 0)
                    glDetachShader(m_Handle, mod->GetHandle());
            }
            glDeleteProgram(m_Handle);
            m_Handle = 0;
        }
        // Modules are destroyed via unique_ptr destruction.
    }

    void OpenGLShader::SetDebugName(std::string_view name)
    {
        m_Name = std::string(name);
        if (m_DebugLayer && m_Handle != 0)
            m_DebugLayer->SetObjectLabel(GL_PROGRAM, m_Handle, m_Name);
    }

    void OpenGLShader::Link()
    {
        glLinkProgram(m_Handle);

        GLint success = GL_FALSE;
        glGetProgramiv(m_Handle, GL_LINK_STATUS, &success);

        GLint logLen = 0;
        glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            m_LinkLog.resize(static_cast<usize>(logLen));
            glGetProgramInfoLog(m_Handle, logLen, nullptr, m_LinkLog.data());
        }

        if (success != GL_TRUE)
        {
            ENGINE_LOG_ERROR("OpenGLShader — link failed for '{}':\n{}", m_Name, m_LinkLog);
            m_Linked = false;
        }
        else
        {
            ENGINE_LOG_DEBUG("OpenGLShader — linked '{}' (handle={})", m_Name, m_Handle);
            m_Linked = true;
        }
    }

    void OpenGLShader::Reflect()
    {
        if (!m_Linked || m_Handle == 0)
            return;

        // Reflect uniform buffers.
        GLint numUBOs = 0;
        glGetProgramInterfaceiv(m_Handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numUBOs);
        for (GLint i = 0; i < numUBOs; ++i)
        {
            UniformBufferInfo info{};

            GLsizei nameLen = 0;
            char nameBuf[256];
            glGetActiveUniformBlockName(m_Handle, i, sizeof(nameBuf), &nameLen, nameBuf);
            info.Name = std::string(nameBuf, nameLen);

            GLint binding = 0;
            glGetActiveUniformBlockiv(m_Handle, i, GL_UNIFORM_BLOCK_BINDING, &binding);
            info.Binding = static_cast<u32>(binding);

            GLint size = 0;
            glGetActiveUniformBlockiv(m_Handle, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
            info.Size = static_cast<u32>(size);

            info.Stages = m_Description.Stages.empty()
                ? ShaderStage::All
                : m_Description.Stages[0].Stage;

            m_UBOCache[info.Name] = static_cast<i32>(info.Binding);
            m_Reflection.UniformBuffers.push_back(info);
        }

        // Reflect samplers / textures via uniform reflection.
        GLint numUniforms = 0;
        glGetProgramInterfaceiv(m_Handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
        for (GLint i = 0; i < numUniforms; ++i)
        {
            const GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX, GL_ARRAY_SIZE };
            GLint values[5] = {0};
            glGetProgramResourceiv(m_Handle, GL_UNIFORM, i, 5, props, 5, nullptr, values);

            if (values[3] != -1) // Skip uniforms in blocks
                continue;

            const GLenum type = static_cast<GLenum>(values[1]);
            const GLint location = values[2];

            // Check if it's a sampler.
            if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE
                || type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_3D
                || type == GL_SAMPLER_2D_SHADOW || type == GL_SAMPLER_CUBE_SHADOW
                || type == GL_INT_SAMPLER_2D || type == GL_UNSIGNED_INT_SAMPLER_2D)
            {
                SamplerBindingInfo info{};
                info.Name = [&, nameLen = values[0]]() {
                    std::vector<char> n(static_cast<usize>(nameLen) + 1, '\0');
                    glGetProgramResourceName(m_Handle, GL_UNIFORM, i,
                                             static_cast<GLsizei>(n.size()), nullptr, n.data());
                    return std::string(n.data());
                }();
                info.Binding = static_cast<u32>(location);
                info.Set = 0;
                info.Stages = ShaderStage::All;
                info.IsCombinedSampler = true;
                m_SamplerCache[info.Name] = static_cast<i32>(info.Binding);
                m_Reflection.Samplers.push_back(info);
            }
            else
            {
                UniformInfo info{};
                info.Name = [&, nameLen = values[0]]() {
                    std::vector<char> n(static_cast<usize>(nameLen) + 1, '\0');
                    glGetProgramResourceName(m_Handle, GL_UNIFORM, i,
                                             static_cast<GLsizei>(n.size()), nullptr, n.data());
                    return std::string(n.data());
                }();
                info.Location = static_cast<u32>(location);
                info.Binding = 0;
                info.Stages = ShaderStage::All;
                m_UniformCache[info.Name] = static_cast<i32>(info.Location);
                m_Reflection.Uniforms.push_back(info);
            }
        }
    }

    i32 OpenGLShader::FindUniformLocation(const std::string& name) const
    {
        auto it = m_UniformCache.find(name);
        if (it != m_UniformCache.end())
            return it->second;
        const GLint loc = glGetUniformLocation(m_Handle, name.c_str());
        return loc;
    }

    i32 OpenGLShader::FindUniformBufferBinding(const std::string& name) const
    {
        auto it = m_UBOCache.find(name);
        if (it != m_UBOCache.end())
            return it->second;
        const GLuint idx = glGetUniformBlockIndex(m_Handle, name.c_str());
        if (idx == GL_INVALID_INDEX)
            return -1;
        GLint binding = 0;
        glGetActiveUniformBlockiv(m_Handle, idx, GL_UNIFORM_BLOCK_BINDING, &binding);
        return static_cast<i32>(binding);
    }

    i32 OpenGLShader::FindSamplerBinding(const std::string& name) const
    {
        auto it = m_SamplerCache.find(name);
        if (it != m_SamplerCache.end())
            return it->second;
        const GLint loc = glGetUniformLocation(m_Handle, name.c_str());
        return loc;
    }

    void OpenGLShader::Bind()
    {
        glUseProgram(m_Handle);
    }

} // namespace engine::opengl
