// ============================================================================
// File: Editor/Include/Editor/Panels/GamePanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Editor/Viewport/ViewportFramebuffer.h"
#include "Editor/Rendering/SceneRenderer.h"
#include <string>

namespace editor {

    /// @brief The Game View panel — renders the active runtime camera.
    class GamePanel final : public IPanel
    {
    public:
        GamePanel();
        ~GamePanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Center; }

        void OnRender(EditorContext& context) override;

    private:
        std::string m_Name{"Game View"};
        std::string m_Title{"Game View"};
        ViewportFramebuffer m_Framebuffer;
        SceneRenderer       m_SceneRenderer;
    };

} // namespace editor
