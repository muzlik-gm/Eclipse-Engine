// ============================================================================
// File: Editor/Include/Editor/Panels/ScenePanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Editor/Viewport/ViewportFramebuffer.h"
#include "Editor/Rendering/SceneRenderer.h"
#include "Editor/Picking/EntityPicking.h"
#include "Engine/Math/Math.h"
#include <imgui.h>
#include <string>

struct GLFWwindow;

struct ImVec2;

namespace editor {

    /// @brief The Scene View panel — renders the editor world with the
    ///        editor camera, grid, gizmos, and entity picking.
    class ScenePanel final : public IPanel
    {
    public:
        explicit ScenePanel(GLFWwindow* window = nullptr);
        ~ScenePanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Center; }

        [[nodiscard]] unsigned int GetWindowFlags() const noexcept override { return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; }

        void OnRender(EditorContext& context) override;

    private:
        void RenderViewportToolbar(EditorContext& context);
        void RenderScene(EditorContext& context);
        void HandleMouseInput(EditorContext& context, ImVec2 imagePos, ImVec2 imageSize);

        std::string m_Name{"Scene View"};
        std::string m_Title{"Scene View"};
        bool m_IsFocused{false};
        bool m_IsHovered{false};
        bool m_IsFlying{false};

        GLFWwindow* m_Window{nullptr};

        // Gizmo drag interaction state.
        int     m_GizmoDragAxis{-1};      // -1 = not dragging, 0=X, 1=Y, 2=Z
        engine::math::Vec3 m_GizmoDragOrigin{0.0f}; // Entity position at drag start
        engine::math::Vec3 m_GizmoDragStart{0.0f};  // Hit point at drag start
        engine::math::Mat4 m_GizmoDragInvVP{1.0f};  // Inv VP at drag start

        ViewportFramebuffer m_Framebuffer;
        SceneRenderer       m_SceneRenderer;
        EntityPicking       m_Picking;
    };

} // namespace editor
