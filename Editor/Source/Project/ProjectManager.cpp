// ============================================================================
// File: Editor/Source/Project/ProjectManager.cpp
// ============================================================================
#include "Editor/Project/ProjectManager.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <filesystem>

namespace editor {

    namespace fs = std::filesystem;
    using json = nlohmann::json;

    ProjectManager::ProjectManager()
    {
        LoadRecentProjects();
    }

    ProjectManager::~ProjectManager()
    {
        SaveRecentProjects();
    }

    // ========================================================================
    // Project lifecycle
    // ========================================================================

    bool ProjectManager::CreateProject(const std::string& directory, const std::string& name)
    {
        CloseProject();

        m_Current = std::make_unique<Project>();
        if (!m_Current->Create(directory, name))
        {
            m_Current.reset();
            return false;
        }

        AddRecentProject(name, m_Current->GetInfo().GetProjectFilePath().string());
        SaveRecentProjects();
        NotifyChanged();
        return true;
    }

    bool ProjectManager::OpenProject(const std::string& filePath)
    {
        CloseProject();

        m_Current = std::make_unique<Project>();
        if (!m_Current->Open(filePath))
        {
            m_Current.reset();
            return false;
        }

        AddRecentProject(m_Current->GetName(), filePath);
        SaveRecentProjects();
        NotifyChanged();
        return true;
    }

    void ProjectManager::CloseProject()
    {
        if (!m_Current)
            return;

        m_Current->Close();
        m_Current.reset();
        NotifyChanged();
    }

    bool ProjectManager::SaveProject()
    {
        if (!m_Current)
            return false;
        return m_Current->Save();
    }

    // ========================================================================
    // Recent projects
    // ========================================================================

    void ProjectManager::LoadRecentProjects()
    {
        try
        {
            std::ifstream ifs(m_RecentFilePath);
            if (!ifs.is_open())
                return;

            json j;
            ifs >> j;

            m_Recent.clear();
            if (j.contains("projects"))
            {
                for (const auto& p : j["projects"])
                {
                    RecentProjectEntry entry;
                    entry.Name       = p.value("name", "");
                    entry.FilePath   = p.value("file_path", "");
                    entry.LastOpened = p.value("last_opened", "");
                    m_Recent.push_back(entry);
                }
            }
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_WARN("ProjectManager — failed to load recent projects: {}", e.what());
        }
    }

    void ProjectManager::SaveRecentProjects()
    {
        try
        {
            fs::create_directories(fs::path(m_RecentFilePath).parent_path());

            json j;
            json projects = json::array();
            for (const auto& p : m_Recent)
            {
                json entry;
                entry["name"]        = p.Name;
                entry["file_path"]   = p.FilePath;
                entry["last_opened"] = p.LastOpened;
                projects.push_back(entry);
            }
            j["projects"] = projects;

            std::ofstream ofs(m_RecentFilePath);
            ofs << j.dump(4);
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_WARN("ProjectManager — failed to save recent projects: {}", e.what());
        }
    }

    void ProjectManager::AddRecentProject(const std::string& name, const std::string& filePath)
    {
        // Remove existing entry with the same path.
        m_Recent.erase(
            std::remove_if(m_Recent.begin(), m_Recent.end(),
                [&](const RecentProjectEntry& e) { return e.FilePath == filePath; }),
            m_Recent.end());

        // Add to front.
        RecentProjectEntry entry;
        entry.Name = name;
        entry.FilePath = filePath;

        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t, &tm);
        char ts[32];
        std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", &tm);
        entry.LastOpened = ts;

        m_Recent.insert(m_Recent.begin(), entry);

        // Cap at 10.
        if (m_Recent.size() > 10)
            m_Recent.resize(10);
    }

    void ProjectManager::RemoveRecentProject(const std::string& filePath)
    {
        m_Recent.erase(
            std::remove_if(m_Recent.begin(), m_Recent.end(),
                [&](const RecentProjectEntry& e) { return e.FilePath == filePath; }),
            m_Recent.end());
    }

    // ========================================================================
    // Callbacks
    // ========================================================================

    engine::core::u32 ProjectManager::Subscribe(ProjectChangedCallback callback)
    {
        auto id = m_NextId++;
        m_Callbacks[id] = std::move(callback);
        return id;
    }

    void ProjectManager::Unsubscribe(engine::core::u32 id)
    {
        m_Callbacks.erase(id);
    }

    void ProjectManager::NotifyChanged()
    {
        for (const auto& [_, cb] : m_Callbacks)
        {
            if (cb)
                cb(m_Current.get());
        }
    }

} // namespace editor
