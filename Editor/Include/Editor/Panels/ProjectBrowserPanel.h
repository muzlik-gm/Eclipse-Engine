// ============================================================================
// File: Editor/Include/Editor/Panels/ProjectBrowserPanel.h
// Startup window for creating/opening projects.
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include <string>

namespace editor {

    /// @brief Startup project browser.  Allows the user to create a new
    ///        project, open an existing project, or select from recent
    ///        projects.  Once a project is opened, the editor launches.
    class ProjectBrowserPanel final : public IPanel
    {
    public:
        ProjectBrowserPanel();
        ~ProjectBrowserPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Center; }

        void OnRender(EditorContext& context) override;

        /// @brief Returns true if a project was selected and the browser
        ///        should close.
        [[nodiscard]] bool ShouldClose() const noexcept { return m_ShouldClose; }

    private:
        void RenderNewProjectTab(EditorContext& context);
        void RenderOpenProjectTab(EditorContext& context);
        void RenderRecentProjectsTab(EditorContext& context);

        std::string m_Name{"Project Browser"};
        std::string m_Title{"Project Browser"};
        bool        m_ShouldClose{false};

        // New project tab state.
        char m_NewProjectName[256]{"NewProject"};
        char m_NewProjectPath[512]{"."};
        int  m_ActiveTab{0}; // 0=New, 1=Open, 2=Recent
    };

} // namespace editor
