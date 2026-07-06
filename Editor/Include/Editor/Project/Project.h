// ============================================================================
// File: Editor/Include/Editor/Project/Project.h
// Represents a game project on disk.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/UUID.h"

#include <filesystem>
#include <string>
#include <vector>

namespace editor {

    namespace fs = std::filesystem;

    // ========================================================================
    // ProjectInfo — metadata about a project.
    // ========================================================================

    struct ProjectInfo
    {
        std::string         Name{"Untitled Project"};
        engine::core::UUID  UUID{};
        std::string         EngineVersion{"0.1.0"};
        fs::path            RootPath;
        fs::path            AssetPath;
        fs::path            LibraryPath;
        fs::path            CachePath;
        fs::path            ScenePath;
        fs::path            SettingsPath;
        fs::path            PackagesPath;
        fs::path            BuildPath;
        fs::path            TempPath;
        fs::path            UserSettingsPath;

        /// Returns the path to the project file (.eproject).
        [[nodiscard]] fs::path GetProjectFilePath() const
        { return RootPath / (Name + ".eproject"); }

        /// Returns true if all required directories exist.
        [[nodiscard]] bool IsValid() const noexcept
        { return !RootPath.empty() && !Name.empty(); }
    };

    // ========================================================================
    // Project — represents an open project.
    // ========================================================================

    /// @brief Represents a game project on disk.  A project owns its
    ///        directory structure, asset database, scene list, and
    ///        settings.  Only one project can be open at a time.
    class Project
    {
    public:
        Project();
        ~Project();

        Project(const Project&)            = delete;
        Project& operator=(const Project&) = delete;
        Project(Project&&)                 = delete;
        Project& operator=(Project&&)      = delete;

        // -- Lifecycle -----------------------------------------------------

        /// @brief Creates a new project at @p directoryPath with @p name.
        ///        Creates all required subdirectories.
        bool Create(const std::string& directoryPath, const std::string& name);

        /// @brief Opens an existing project from a .eproject file.
        bool Open(const std::string& projectFilePath);

        /// @brief Closes the project, saving any unsaved state.
        void Close();

        /// @brief Saves the project configuration.
        bool Save() const;

        /// @brief Returns true if a project is open.
        [[nodiscard]] bool IsOpen() const noexcept { return m_Open; }

        // -- Accessors -----------------------------------------------------

        [[nodiscard]] const ProjectInfo& GetInfo() const noexcept { return m_Info; }

        [[nodiscard]] const std::string& GetName() const noexcept { return m_Info.Name; }
        [[nodiscard]] const engine::core::UUID& GetUUID() const noexcept { return m_Info.UUID; }
        [[nodiscard]] const fs::path& GetRootPath() const noexcept { return m_Info.RootPath; }
        [[nodiscard]] const fs::path& GetAssetPath() const noexcept { return m_Info.AssetPath; }
        [[nodiscard]] const fs::path& GetScenePath() const noexcept { return m_Info.ScenePath; }

        /// @brief Returns the relative path of @p absolutePath from the
        ///        project root.
        [[nodiscard]] std::string GetRelativePath(const std::string& absolutePath) const;

        /// @brief Resolves a relative path to an absolute path within the project.
        [[nodiscard]] std::string GetAbsolutePath(const std::string& relativePath) const;

        // -- Scene management ----------------------------------------------

        /// @brief Returns a list of scene files in the project.
        [[nodiscard]] std::vector<std::string> GetSceneFiles() const;

        /// @brief Returns true if the project has unsaved changes.
        [[nodiscard]] bool IsDirty() const noexcept { return m_Dirty; }

        /// @brief Marks the project as having unsaved changes.
        void SetDirty(bool dirty) noexcept { m_Dirty = dirty; }

    private:
        void CreateDirectoryStructure();
        void LoadProjectInfo(const std::string& filePath);
        bool SaveProjectInfo() const;

        ProjectInfo  m_Info;
        bool         m_Open{false};
        bool         m_Dirty{false};
    };

} // namespace editor
