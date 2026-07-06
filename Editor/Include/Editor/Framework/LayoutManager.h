// ============================================================================
// File: Editor/Include/Editor/Framework/LayoutManager.h
// Saves and restores editor dock layout + window positions.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <vector>

namespace editor {

    class EditorContext;

    // ========================================================================
    // LayoutData — persisted layout configuration.
    // ========================================================================

    struct LayoutData
    {
        /// ImGui dockspace layout INI string (ImGui's internal format).
        std::string DockLayoutIni;

        /// List of open panel names.
        std::vector<std::string> OpenPanels;

        /// Main window position (x, y).
        engine::core::i32 WindowX{100};
        engine::core::i32 WindowY{100};

        /// Main window size (w, h).
        engine::core::i32 WindowW{1280};
        engine::core::i32 WindowH{720};

        /// True if the main window is maximized.
        bool Maximized{false};
    };

    // ========================================================================
    // LayoutManager — saves and restores editor layout.
    // ========================================================================

    /// @brief Saves the editor's dock layout, open panels, and window
    ///        position to disk and restores them at startup.
    class LayoutManager
    {
    public:
        LayoutManager();
        ~LayoutManager() = default;

        /// @brief Sets the directory where layout files are stored.
        void SetConfigDirectory(const std::string& dir) { m_ConfigDir = dir; }

        /// @brief Saves the current layout to disk.
        /// @param context The editor context (for panel open state).
        bool Save(EditorContext& context);

        /// @brief Loads the layout from disk.
        /// @param context The editor context (to apply panel open state).
        bool Load(EditorContext& context);

        /// @brief Resets the layout to defaults.
        void ResetToDefault();

        /// @brief Returns the last loaded layout data.
        [[nodiscard]] const LayoutData& GetLayout() const noexcept { return m_Layout; }

        /// @brief Sets the layout data (applied on next Load).
        void SetLayout(const LayoutData& layout) { m_Layout = layout; }

        /// @brief Returns the path to the layout file.
        [[nodiscard]] std::string GetLayoutFilePath() const;

    private:
        std::string m_ConfigDir{".editor"};
        LayoutData  m_Layout{};
    };

} // namespace editor
