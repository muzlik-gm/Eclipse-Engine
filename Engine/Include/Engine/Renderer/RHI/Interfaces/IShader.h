// ============================================================================
// File: Engine/Include/Engine/Renderer/RHI/Interfaces/IShader.h
// Abstract interfaces for shaders, shader modules, and shader programs.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Renderer/RHI/Interfaces/IGraphicsObject.h"
#include "Engine/Renderer/RHI/Descriptions/PipelineDescriptions.h"
#include "Engine/Renderer/RHI/Enums/GraphicsEnums.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace engine::rhi {

    using engine::core::u32;
    using engine::core::i32;
    using engine::core::usize;

    // ========================================================================
    // ShaderReflection — reflection metadata for a compiled shader.
    // ========================================================================

    /// @brief Describes a single uniform / UBO member discovered during
    ///        shader reflection.
    struct UniformInfo
    {
        std::string  Name;
        u32          Location{0};
        u32          Binding{0};
        u32          Offset{0};
        u32          Size{0};
        GraphicsFormat Format{GraphicsFormat::Undefined};
        ShaderStage  Stages{ShaderStage::None};
    };

    /// @brief Describes a uniform buffer object (UBO) binding.
    struct UniformBufferInfo
    {
        std::string  Name;
        u32          Binding{0};
        u32          Size{0};
        ShaderStage  Stages{ShaderStage::None};
    };

    /// @brief Describes a sampler / texture binding.
    struct SamplerBindingInfo
    {
        std::string  Name;
        u32          Binding{0};
        u32          Set{0};
        ShaderStage  Stages{ShaderStage::None};
        bool         IsCombinedSampler{true};
    };

    /// @brief Full reflection result for a shader program.
    struct ShaderReflection
    {
        std::vector<UniformInfo>          Uniforms;
        std::vector<UniformBufferInfo>    UniformBuffers;
        std::vector<SamplerBindingInfo>   Samplers;
        std::vector<PushConstantRange>    PushConstants;
    };

    // ========================================================================
    // IShaderModule — a single compiled shader stage.
    // ========================================================================

    /// @brief A single compiled shader stage (vertex, fragment, etc.).
    ///
    /// Modules are created by compiling source (or uploading bytecode)
    /// through the IGraphicsFactory.  They are combined into an
    /// IShaderProgram for pipeline use.
    class IShaderModule : public IGraphicsObject
    {
    public:
        [[nodiscard]] virtual ShaderStage    GetStage() const noexcept = 0;
        [[nodiscard]] virtual ShaderLanguage GetLanguage() const noexcept = 0;
        [[nodiscard]] virtual const std::string& GetEntryPoint() const noexcept = 0;
        [[nodiscard]] virtual const std::string& GetFilePath() const noexcept = 0;

        /// @brief Returns the source code if available, or an empty string.
        [[nodiscard]] virtual const std::string& GetSource() const noexcept = 0;

        /// @brief Returns SPIR-V bytecode if available, or an empty vector.
        [[nodiscard]] virtual const std::vector<u32>& GetBytecode() const noexcept = 0;

        /// @brief Returns true if the module compiled successfully.
        [[nodiscard]] virtual bool IsCompiled() const noexcept = 0;

        /// @brief Returns the compiler log / error messages.
        [[nodiscard]] virtual const std::string& GetCompileLog() const noexcept = 0;
    };

    // ========================================================================
    // IShader — a linked shader program (one or more modules).
    // ========================================================================

    /// @brief A linked shader program — the unit that pipelines consume.
    ///
    /// This interface is named IShader (rather than IShaderProgram) to
    /// match the Phase 3 spec's "Shader / ShaderModule / ShaderProgram"
    /// list while still being unambiguous.
    class IShader : public IGraphicsObject
    {
    public:
        /// @brief Returns the description this shader was created with.
        [[nodiscard]] virtual const ShaderDescription& GetDescription() const noexcept = 0;

        /// @brief Returns the list of modules that make up this program.
        [[nodiscard]] virtual const std::vector<IShaderModule*>& GetModules() const noexcept = 0;

        /// @brief Returns true if the program linked successfully.
        [[nodiscard]] virtual bool IsLinked() const noexcept = 0;

        /// @brief Returns the linker log / error messages.
        [[nodiscard]] virtual const std::string& GetLinkLog() const noexcept = 0;

        /// @brief Returns reflection metadata for this program.
        [[nodiscard]] virtual const ShaderReflection& GetReflection() const noexcept = 0;

        /// @brief Looks up a uniform location by name.
        /// @return -1 if not found.
        [[nodiscard]] virtual i32 FindUniformLocation(const std::string& name) const = 0;

        /// @brief Looks up a uniform buffer binding by name.
        /// @return -1 if not found.
        [[nodiscard]] virtual i32 FindUniformBufferBinding(const std::string& name) const = 0;

        /// @brief Looks up a sampler binding by name.
        /// @return -1 if not found.
        [[nodiscard]] virtual i32 FindSamplerBinding(const std::string& name) const = 0;
    };

    // Alias kept for spec completeness — both names are valid.
    using IShaderProgram = IShader;

} // namespace engine::rhi
