// ============================================================================
// File: Engine/Source/Renderer/OpenGL/OpenGLShaderPreprocessor.cpp
// ============================================================================
#include "OpenGLShaderPreprocessor.h"
#include "Engine/Core/Log.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace engine::opengl {

    namespace fs = std::filesystem;

    void OpenGLShaderPreprocessor::AddIncludePath(const std::string& path)
    {
        m_IncludePaths.push_back(path);
    }

    std::string OpenGLShaderPreprocessor::Process(std::string_view source,
                                                   const std::string& filePath)
    {
        m_IncludedFiles.clear();
        m_ProcessedFiles.clear();

        if (!filePath.empty())
            m_ProcessedFiles.insert(fs::path(filePath).lexically_normal().string());

        std::istringstream stream{std::string(source)};
        std::ostringstream output;
        std::string line;

        bool versionInjected = false;
        const std::string versionPragma = "#version 460 core\n";

        // Inject version pragma at the top if not present.
        {
            std::string src(source);
            if (src.find("#version") == std::string::npos)
            {
                output << versionPragma;
                versionInjected = true;
            }
        }

        while (std::getline(stream, line))
        {
            // Skip version pragma if we already injected one.
            if (versionInjected && line.find("#version") != std::string::npos)
                continue;

            // Handle #include "file" or #include <file>
            const auto pos = line.find("#include");
            if (pos != std::string::npos)
            {
                std::string includePath;
                bool found = false;

                // Extract path between quotes or angle brackets.
                auto quotePos = line.find('"');
                auto anglePos = line.find('<');

                if (quotePos != std::string::npos)
                {
                    auto end = line.find('"', quotePos + 1);
                    if (end != std::string::npos)
                    {
                        includePath = line.substr(quotePos + 1, end - quotePos - 1);
                        found = true;
                    }
                }
                else if (anglePos != std::string::npos)
                {
                    auto end = line.find('>', anglePos + 1);
                    if (end != std::string::npos)
                    {
                        includePath = line.substr(anglePos + 1, end - anglePos - 1);
                        found = true;
                    }
                }

                if (found)
                {
                    std::string resolvedPath;
                    std::string includedContent;

                    // Try relative to current file first.
                    if (!filePath.empty())
                    {
                        fs::path relative = fs::path(filePath).parent_path() / includePath;
                        if (fs::exists(relative))
                        {
                            resolvedPath = fs::path(relative).lexically_normal().string();
                            std::ifstream ifs(relative);
                            std::stringstream ss;
                            ss << ifs.rdbuf();
                            includedContent = ss.str();
                        }
                    }

                    // Try include paths.
                    if (includedContent.empty())
                    {
                        for (const auto& searchPath : m_IncludePaths)
                        {
                            fs::path candidate = fs::path(searchPath) / includePath;
                            if (fs::exists(candidate))
                            {
                                resolvedPath = fs::path(candidate).lexically_normal().string();
                                std::ifstream ifs(candidate);
                                std::stringstream ss;
                                ss << ifs.rdbuf();
                                includedContent = ss.str();
                                break;
                            }
                        }
                    }

                    if (includedContent.empty())
                    {
                        ENGINE_LOG_ERROR("OpenGLShaderPreprocessor — include not found: {}", includePath);
                        output << "// #include \"" << includePath << "\" — NOT FOUND\n";
                        continue;
                    }

                    // Check for circular include.
                    if (m_ProcessedFiles.count(resolvedPath) > 0)
                    {
                        ENGINE_LOG_WARN("OpenGLShaderPreprocessor — circular include skipped: {}", resolvedPath);
                        continue;
                    }

                    m_ProcessedFiles.insert(resolvedPath);
                    m_IncludedFiles.push_back(resolvedPath);

                    // Recursively process the included file.
                    OpenGLShaderPreprocessor subProcessor;
                    subProcessor.m_IncludePaths = m_IncludePaths;
                    subProcessor.m_ProcessedFiles = m_ProcessedFiles;

                    std::string processed = subProcessor.Process(includedContent, resolvedPath);

                    // Merge included files list.
                    for (const auto& f : subProcessor.m_IncludedFiles)
                        m_IncludedFiles.push_back(f);

                    // Merge processed files.
                    for (const auto& f : subProcessor.m_ProcessedFiles)
                        m_ProcessedFiles.insert(f);

                    output << "// === BEGIN INCLUDE: " << resolvedPath << " ===\n";
                    output << processed;
                    output << "// === END INCLUDE: " << resolvedPath << " ===\n";
                    continue;
                }
            }

            output << line << '\n';
        }

        return output.str();
    }

} // namespace engine::opengl
