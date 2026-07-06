// ============================================================================
// File: Editor/Include/Editor/Project/ProjectManager.h
// Manages the currently open project and recent projects list.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Editor/Project/Project.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace editor {

    // ========================================================================
    // RecentProjectEntry — a single recent project entry.
    // ========================================================================

    struct RecentProjectEntry
    {
        std::string Name;
        std::string FilePath;
        std::string LastOpened;
    };

    // ========================================================================
    // ProjectManager — manages project lifecycle.
    // ========================================================================

    /// @brief Manages the currently open project and the list of recently
    ///        opened projects.  Only one project can be open at a time.
    class ProjectManager
    {
    public:
        ProjectManager();
        ~ProjectManager();

        /// @brief Creates a new project.
        bool CreateProject(const std::string& directory, const std::string& name);

        /// @brief Opens an existing project from a .eproject file.
        bool OpenProject(const std::string& filePath);

        /// @brief Closes the current project.
        void CloseProject();

        /// @brief Saves the current project.
        bool SaveProject();

        /// @brief Returns the current open project, or nullptr.
        [[nodiscard]] Project* GetCurrentProject() noexcept { return m_Current.get(); }

        /// @brief Returns true if a project is open.
        [[nodiscard]] bool HasProject() const noexcept { return m_Current != nullptr; }

        // -- Recent projects -----------------------------------------------

        /// @brief Returns the list of recently opened projects.
        [[nodiscard]] const std::vector<RecentProjectEntry>& GetRecentProjects() const noexcept
        { return m_Recent; }

        /// @brief Loads the recent projects list from disk.
        void LoadRecentProjects();

        /// @brief Saves the recent projects list to disk.
        void SaveRecentProjects();

        /// @brief Adds a project to the recent list.
        void AddRecentProject(const std::string& name, const std::string& filePath);

        /// @brief Removes a project from the recent list.
        void RemoveRecentProject(const std::string& filePath);

        // -- Callbacks -----------------------------------------------------

        using ProjectChangedCallback = std::function<void(Project*)>;

        /// @brief Registers a callback invoked when the current project changes.
        engine::core::u32 Subscribe(ProjectChangedCallback callback);

        /// @brief Removes a registered callback.
        void Unsubscribe(engine::core::u32 id);

    private:
        void NotifyChanged();

        std::unique_ptr<Project>                   m_Current;
        std::vector<RecentProjectEntry>             m_Recent;
        std::unordered_map<engine::core::u32,
            ProjectChangedCallback>                 m_Callbacks;
        engine::core::u32                           m_NextId{1};
        std::string                                 m_RecentFilePath{".editor/recent_projects.json"};
    };

} // namespace editor
