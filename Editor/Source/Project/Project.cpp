// ============================================================================
// File: Editor/Source/Project/Project.cpp
// ============================================================================
#include "Editor/Project/Project.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/UUID.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

namespace editor {

    namespace fs = std::filesystem;
    using json = nlohmann::json;

    Project::Project() = default;
    Project::~Project()
    {
        Close();
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool Project::Create(const std::string& directoryPath, const std::string& name)
    {
        m_Info.Name = name;
        m_Info.UUID = engine::core::UUID{};
        m_Info.EngineVersion = "0.1.0";
        m_Info.RootPath = fs::path(directoryPath) / name;

        CreateDirectoryStructure();

        if (!SaveProjectInfo())
        {
            ENGINE_LOG_ERROR("Project — failed to save project file");
            return false;
        }

        m_Open = true;
        ENGINE_LOG_INFO("Project — created '{}' at '{}'", name, m_Info.RootPath.string());
        return true;
    }

    bool Project::Open(const std::string& projectFilePath)
    {
        if (!fs::exists(projectFilePath))
        {
            ENGINE_LOG_ERROR("Project — file not found: '{}'", projectFilePath);
            return false;
        }

        LoadProjectInfo(projectFilePath);

        // Ensure directories exist.
        CreateDirectoryStructure();

        m_Open = true;
        ENGINE_LOG_INFO("Project — opened '{}' ({})", m_Info.Name, m_Info.UUID.ToString());
        return true;
    }

    void Project::Close()
    {
        if (!m_Open)
            return;

        if (m_Dirty)
            Save();

        m_Open = false;
        m_Dirty = false;
        ENGINE_LOG_INFO("Project — closed '{}'", m_Info.Name);
    }

    bool Project::Save() const
    {
        if (!m_Open)
            return false;
        return SaveProjectInfo();
    }

    // ========================================================================
    // Path helpers
    // ========================================================================

    std::string Project::GetRelativePath(const std::string& absolutePath) const
    {
        auto rel = fs::relative(absolutePath, m_Info.RootPath);
        return rel.lexically_normal().string();
    }

    std::string Project::GetAbsolutePath(const std::string& relativePath) const
    {
        return (m_Info.RootPath / relativePath).lexically_normal().string();
    }

    // ========================================================================
    // Scene management
    // ========================================================================

    std::vector<std::string> Project::GetSceneFiles() const
    {
        std::vector<std::string> scenes;

        if (!m_Open || !fs::exists(m_Info.ScenePath))
            return scenes;

        for (const auto& entry : fs::recursive_directory_iterator(m_Info.ScenePath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".scene")
            {
                scenes.push_back(entry.path().string());
            }
        }

        return scenes;
    }

    // ========================================================================
    // Internal
    // ========================================================================

    void Project::CreateDirectoryStructure()
    {
        m_Info.AssetPath        = m_Info.RootPath / "Assets";
        m_Info.LibraryPath      = m_Info.RootPath / "Library";
        m_Info.CachePath        = m_Info.RootPath / "Cache";
        m_Info.ScenePath        = m_Info.RootPath / "Scenes";
        m_Info.SettingsPath     = m_Info.RootPath / "Settings";
        m_Info.PackagesPath     = m_Info.RootPath / "Packages";
        m_Info.BuildPath        = m_Info.RootPath / "Build";
        m_Info.TempPath         = m_Info.RootPath / "Temp";
        m_Info.UserSettingsPath = m_Info.RootPath / "UserSettings";

        fs::create_directories(m_Info.AssetPath);
        fs::create_directories(m_Info.LibraryPath);
        fs::create_directories(m_Info.CachePath);
        fs::create_directories(m_Info.ScenePath);
        fs::create_directories(m_Info.SettingsPath);
        fs::create_directories(m_Info.PackagesPath);
        fs::create_directories(m_Info.BuildPath);
        fs::create_directories(m_Info.TempPath);
        fs::create_directories(m_Info.UserSettingsPath);

        // Create a default .gitignore in the project root.
        auto gitignorePath = m_Info.RootPath / ".gitignore";
        if (!fs::exists(gitignorePath))
        {
            std::ofstream ofs(gitignorePath);
            ofs << "Library/\nCache/\nTemp/\nBuild/\nUserSettings/\n*.eproject.user\n";
        }

        ENGINE_LOG_DEBUG("Project — directory structure created at '{}'", m_Info.RootPath.string());
    }

    void Project::LoadProjectInfo(const std::string& filePath)
    {
        try
        {
            std::ifstream ifs(filePath);
            if (!ifs.is_open())
            {
                ENGINE_LOG_ERROR("Project — cannot open '{}'", filePath);
                return;
            }

            json j;
            ifs >> j;

            m_Info.Name          = j.value("name", "Untitled Project");
            m_Info.UUID          = j.contains("uuid")
                ? engine::core::UUID::FromString(j["uuid"].get<std::string>())
                : engine::core::UUID{};
            m_Info.EngineVersion = j.value("engine_version", "0.1.0");
            m_Info.RootPath      = fs::path(filePath).parent_path();

            // Subdirectory paths (relative to root).
            m_Info.AssetPath        = m_Info.RootPath / "Assets";
            m_Info.LibraryPath      = m_Info.RootPath / "Library";
            m_Info.CachePath        = m_Info.RootPath / "Cache";
            m_Info.ScenePath        = m_Info.RootPath / "Scenes";
            m_Info.SettingsPath     = m_Info.RootPath / "Settings";
            m_Info.PackagesPath     = m_Info.RootPath / "Packages";
            m_Info.BuildPath        = m_Info.RootPath / "Build";
            m_Info.TempPath         = m_Info.RootPath / "Temp";
            m_Info.UserSettingsPath = m_Info.RootPath / "UserSettings";
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("Project — load error: {}", e.what());
        }
    }

    bool Project::SaveProjectInfo() const
    {
        try
        {
            json j;
            j["name"]           = m_Info.Name;
            j["uuid"]           = m_Info.UUID.ToString();
            j["engine_version"] = m_Info.EngineVersion;

            auto filePath = m_Info.GetProjectFilePath();
            fs::create_directories(filePath.parent_path());

            std::ofstream ofs(filePath);
            if (!ofs.is_open())
            {
                ENGINE_LOG_ERROR("Project — cannot write '{}'", filePath.string());
                return false;
            }

            ofs << j.dump(4);
            ENGINE_LOG_INFO("Project — saved to '{}'", filePath.string());
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("Project — save error: {}", e.what());
            return false;
        }
    }

} // namespace editor
