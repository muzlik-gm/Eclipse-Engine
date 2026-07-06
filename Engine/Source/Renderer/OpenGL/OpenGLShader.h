// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLShader.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IShader.h"
#include "OpenGLShaderPreprocessor.h"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::opengl {

    using engine::rhi::IShaderModule;

    class OpenGLDebugLayer;

    // ========================================================================
    // OpenGLShaderModule — a single compiled shader stage
    // ========================================================================

    class OpenGLShaderModule final : public engine::rhi::IShaderModule
    {
    public:
        OpenGLShaderModule(const engine::rhi::ShaderStageDescription& desc,
                           OpenGLDebugLayer* debugLayer);
        ~OpenGLShaderModule() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0; }

        engine::rhi::ShaderStage GetStage() const noexcept override { return m_Stage; }
        engine::rhi::ShaderLanguage GetLanguage() const noexcept override { return m_Language; }
        const std::string& GetEntryPoint() const noexcept override { return m_EntryPoint; }
        const std::string& GetFilePath() const noexcept override { return m_FilePath; }
        const std::string& GetSource() const noexcept override { return m_ProcessedSource; }
        const std::vector<engine::core::u32>& GetBytecode() const noexcept override { return m_Bytecode; }
        bool IsCompiled() const noexcept override { return m_Compiled; }
        const std::string& GetCompileLog() const noexcept override { return m_CompileLog; }

        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }

    private:
        void Compile();

        engine::rhi::ShaderStage    m_Stage{engine::rhi::ShaderStage::None};
        engine::rhi::ShaderLanguage m_Language{engine::rhi::ShaderLanguage::GLSL};
        GLuint                      m_Handle{0};
        std::string                 m_Name;
        std::string                 m_EntryPoint;
        std::string                 m_FilePath;
        std::string                 m_Source;
        std::string                 m_ProcessedSource;
        std::vector<engine::core::u32> m_Bytecode;
        std::string                 m_CompileLog;
        bool                        m_Compiled{false};
        OpenGLDebugLayer*           m_DebugLayer{nullptr};
        OpenGLShaderPreprocessor    m_Preprocessor;
    };

    // ========================================================================
    // OpenGLShader — a linked shader program
    // ========================================================================

    class OpenGLShader final : public engine::rhi::IShader
    {
    public:
        OpenGLShader(const engine::rhi::ShaderDescription& desc,
                     OpenGLDebugLayer* debugLayer);
        ~OpenGLShader() override;

        std::string_view GetDebugName() const noexcept override { return m_Name; }
        void SetDebugName(std::string_view name) override;
        engine::core::u64 GetNativeHandle() const noexcept override { return m_Handle; }
        bool IsValid() const noexcept override { return m_Handle != 0 && m_Linked; }

        const engine::rhi::ShaderDescription& GetDescription() const noexcept override
        { return m_Description; }

        const std::vector<IShaderModule*>& GetModules() const noexcept override
        { return m_ModulePtrs; }

        bool IsLinked() const noexcept override { return m_Linked; }
        const std::string& GetLinkLog() const noexcept override { return m_LinkLog; }
        const engine::rhi::ShaderReflection& GetReflection() const noexcept override
        { return m_Reflection; }

        engine::core::i32 FindUniformLocation(const std::string& name) const override;
        engine::core::i32 FindUniformBufferBinding(const std::string& name) const override;
        engine::core::i32 FindSamplerBinding(const std::string& name) const override;

        [[nodiscard]] GLuint GetHandle() const noexcept { return m_Handle; }

        void Bind();

    private:
        void Link();
        void Reflect();

        engine::rhi::ShaderDescription         m_Description;
        GLuint                                 m_Handle{0};
        std::string                            m_Name;
        std::vector<std::unique_ptr<OpenGLShaderModule>> m_Modules;
        std::vector<IShaderModule*>            m_ModulePtrs;
        std::string                            m_LinkLog;
        bool                                   m_Linked{false};
        engine::rhi::ShaderReflection          m_Reflection;
        OpenGLDebugLayer*                      m_DebugLayer{nullptr};

        // Cached uniform location lookups.
        std::unordered_map<std::string, engine::core::i32> m_UniformCache;
        std::unordered_map<std::string, engine::core::i32> m_UBOCache;
        std::unordered_map<std::string, engine::core::i32> m_SamplerCache;
    };

} // namespace engine::opengl
