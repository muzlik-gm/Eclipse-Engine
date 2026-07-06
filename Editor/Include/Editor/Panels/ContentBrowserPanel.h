// ============================================================================
// File: Editor/Include/Editor/Panels/ContentBrowserPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include <filesystem>
#include <string>
#include <vector>

namespace editor {

    /// @brief Content Browser panel — displays project assets with
    ///        folders, files, icons, breadcrumbs, and search.
    class ContentBrowserPanel final : public IPanel
    {
    public:
        ContentBrowserPanel();
        ~ContentBrowserPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Bottom; }

        void OnRender(EditorContext& context) override;

        /// @brief Sets the root directory for the content browser.
        void SetRootDirectory(const std::string& path);

    private:
        void RenderBreadcrumbs();
        void RenderDirectoryTree();
        void RenderFileList();

        std::string m_Name{"Content Browser"};
        std::string m_Title{"Content Browser"};
        std::filesystem::path m_RootDir{"assets"};
        std::filesystem::path m_CurrentDir{"assets"};
        std::string m_SearchFilter;
        bool m_ShowHidden{false};
    };

} // namespace editor
