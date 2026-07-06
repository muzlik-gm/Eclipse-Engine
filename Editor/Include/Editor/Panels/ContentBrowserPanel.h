// ============================================================================
// File: Editor/Include/Editor/Panels/ContentBrowserPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Engine/Core/Types.h"
#include <filesystem>
#include <string>
#include <vector>

namespace editor {

    using engine::core::u32;

    /// @brief Content Browser panel — displays project assets with
    ///        folders, files, icons, breadcrumbs, search, context menu,
    ///        grid/list view toggle, rename/delete/duplicate.
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
        void RenderToolbar();
        void RenderBreadcrumbs();
        void RenderDirectoryTree();
        void RenderFileList();
        void RenderContextMenu(EditorContext& context);
        void RenderGridItem(const std::filesystem::path& path, bool isDir, u32 index);

        std::string m_Name{"Content Browser"};
        std::string m_Title{"Content Browser"};
        std::filesystem::path m_RootDir{"assets"};
        std::filesystem::path m_CurrentDir{"assets"};
        std::string m_SearchFilter;
        bool m_ShowHidden{false};
        bool m_GridView{true};

        // Context menu / rename state.
        bool m_ShowContextMenu{false};
        std::filesystem::path m_RenamingPath;
        bool m_IsRenaming{false};
        char m_RenameBuffer[256]{};

        // Selected item for context menu.
        std::filesystem::path m_SelectedPath;
    };

} // namespace editor
