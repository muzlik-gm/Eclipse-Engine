// ============================================================================
// File: Editor/Include/Editor/Panels/ScenePanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include <string>

namespace editor {

    /// @brief The Scene View panel — renders the editor world with the
    ///        editor camera, grid, and gizmos.
    class ScenePanel final : public IPanel
    {
    public:
        ScenePanel();
        ~ScenePanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Center; }

        void OnRender(EditorContext& context) override;

    private:
        void RenderViewportToolbar(EditorContext& context);
        void RenderScene(EditorContext& context);

        std::string m_Name{"Scene View"};
        std::string m_Title{"Scene View"};
        bool m_IsFocused{false};
        bool m_IsHovered{false};
    };

} // namespace editor
