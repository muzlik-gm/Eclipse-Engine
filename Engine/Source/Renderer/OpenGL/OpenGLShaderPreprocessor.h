// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLShaderPreprocessor.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::opengl {

    /// @brief Preprocesses GLSL source: resolves #include directives,
    ///        injects version pragma, and returns the processed source.
    class OpenGLShaderPreprocessor
    {
    public:
        /// @brief Sets the search path for #include resolution.
        void AddIncludePath(const std::string& path);

        /// @brief Preprocesses GLSL source.
        /// @param source Raw GLSL source code.
        /// @param filePath Optional file path the source was loaded from.
        /// @return Preprocessed source with all includes inlined.
        [[nodiscard]] std::string Process(std::string_view source,
                                          const std::string& filePath = {});

        /// @brief Returns the list of files included during the last
        ///        Process() call.  Used for hot-reload.
        [[nodiscard]] const std::vector<std::string>& GetIncludedFiles() const noexcept
        { return m_IncludedFiles; }

    private:
        std::vector<std::string>              m_IncludePaths;
        std::vector<std::string>              m_IncludedFiles;
        std::unordered_set<std::string>       m_ProcessedFiles;
    };

} // namespace engine::opengl
