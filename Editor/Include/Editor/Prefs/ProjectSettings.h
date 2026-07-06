// ============================================================================
// File: Editor/Include/Editor/Prefs/ProjectSettings.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>
#include <vector>

namespace editor {

    // ========================================================================
    // ProjectSettings — per-project configuration.
    // ========================================================================

    /// @brief Stores project-specific settings.  These are per-project
    ///        (not per-user) and live alongside the project file.
    struct ProjectSettings
    {
        // -- Project info --------------------------------------------------
        std::string ProjectName{"Untitled Project"};
        std::string ProjectVersion{"1.0.0"};
        std::string ProjectPath{};

        // -- Renderer ------------------------------------------------------
        std::string RendererBackend{"OpenGL"};
        int         TargetFPS{60};
        bool        VSync{true};

        // -- Asset paths ---------------------------------------------------
        std::vector<std::string> AssetDirectories{"assets://"};
        std::vector<std::string> ScriptDirectories{};
        std::string               ShaderDirectory{"assets://shaders"};

        // -- Build settings ------------------------------------------------
        std::string BuildOutputDirectory{"build"};
        std::string BuildConfiguration{"Release"};

        // -- Scene settings ------------------------------------------------
        std::string DefaultScenePath{};

        // -- Scripting (future) -------------------------------------------
        std::string ScriptingRuntime{"None"};
        bool        EnableHotReload{true};

        /// @brief Loads project settings from a JSON file.
        bool Load(const std::string& filePath);

        /// @brief Saves project settings to a JSON file.
        bool Save(const std::string& filePath) const;
    };

} // namespace editor
