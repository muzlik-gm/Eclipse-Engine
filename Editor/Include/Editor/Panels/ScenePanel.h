// ============================================================================
// File: Editor/Include/Editor/Panels/ScenePanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Editor/Viewport/ViewportFramebuffer.h"
#include "Editor/Rendering/SceneRenderer.h"
#include "Editor/Picking/EntityPicking.h"
#include <string>

struct ImVec2;

namespace editor {

    /// @brief The Scene View panel — renders the editor world with the
    ///        editor camera, grid, gizmos, and entity picking.
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
        void HandleMouseInput(EditorContext& context, ImVec2 imagePos, ImVec2 imageSize);

        std::string m_Name{"Scene View"};
        std::string m_Title{"Scene View"};
        bool m_IsFocused{false};
        bool m_IsHovered{false};

        ViewportFramebuffer m_Framebuffer;
        SceneRenderer       m_SceneRenderer;
        EntityPicking       m_Picking;
    };

} // namespace editor
