// ============================================================================
// File: Editor/Source/Prefs/ProjectSettings.cpp
// ============================================================================
#include "Editor/Prefs/ProjectSettings.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace editor {

    namespace fs = std::filesystem;
    using json = nlohmann::json;

    bool ProjectSettings::Load(const std::string& filePath)
    {
        try
        {
            std::ifstream ifs(filePath);
            if (!ifs.is_open())
                return false;

            json j;
            ifs >> j;

            ProjectName    = j.value("project_name", "Untitled Project");
            ProjectVersion = j.value("project_version", "1.0.0");
            ProjectPath    = j.value("project_path", "");

            RendererBackend = j.value("renderer_backend", "OpenGL");
            TargetFPS       = j.value("target_fps", 60);
            VSync           = j.value("vsync", true);

            AssetDirectories.clear();
            if (j.contains("asset_directories"))
            {
                for (const auto& d : j["asset_directories"])
                    AssetDirectories.push_back(d.get<std::string>());
            }
            if (AssetDirectories.empty())
                AssetDirectories.push_back("assets://");

            ScriptDirectories.clear();
            if (j.contains("script_directories"))
            {
                for (const auto& d : j["script_directories"])
                    ScriptDirectories.push_back(d.get<std::string>());
            }

            ShaderDirectory = j.value("shader_directory", "assets://shaders");

            BuildOutputDirectory = j.value("build_output_directory", "build");
            BuildConfiguration   = j.value("build_configuration", "Release");

            DefaultScenePath = j.value("default_scene_path", "");

            ScriptingRuntime  = j.value("scripting_runtime", "None");
            EnableHotReload   = j.value("enable_hot_reload", true);

            ENGINE_LOG_INFO("ProjectSettings — loaded '{}'", filePath);
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("ProjectSettings — load failed: {}", e.what());
            return false;
        }
    }

    bool ProjectSettings::Save(const std::string& filePath) const
    {
        try
        {
            fs::create_directories(fs::path(filePath).parent_path());

            json j;
            j["project_name"]    = ProjectName;
            j["project_version"] = ProjectVersion;
            j["project_path"]    = ProjectPath;
            j["renderer_backend"] = RendererBackend;
            j["target_fps"]       = TargetFPS;
            j["vsync"]            = VSync;

            json dirs = json::array();
            for (const auto& d : AssetDirectories)
                dirs.push_back(d);
            j["asset_directories"] = dirs;

            json sdirs = json::array();
            for (const auto& d : ScriptDirectories)
                sdirs.push_back(d);
            j["script_directories"] = sdirs;

            j["shader_directory"] = ShaderDirectory;
            j["build_output_directory"] = BuildOutputDirectory;
            j["build_configuration"] = BuildConfiguration;
            j["default_scene_path"] = DefaultScenePath;
            j["scripting_runtime"] = ScriptingRuntime;
            j["enable_hot_reload"] = EnableHotReload;

            std::ofstream ofs(filePath);
            if (!ofs.is_open())
            {
                ENGINE_LOG_ERROR("ProjectSettings — cannot open '{}'", filePath);
                return false;
            }
            ofs << j.dump(4);
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("ProjectSettings — save failed: {}", e.what());
            return false;
        }
    }

} // namespace editor
