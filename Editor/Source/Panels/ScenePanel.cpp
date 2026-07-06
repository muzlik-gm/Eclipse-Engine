// ============================================================================
// File: Editor/Source/Panels/ScenePanel.cpp
// ============================================================================
#include "Editor/Panels/ScenePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Camera/EditorCamera.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Editor/Rendering/DebugRenderer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Components/TransformComponent.h"

#include <imgui.h>
#include <glad/glad.h>

namespace editor {

    ScenePanel::ScenePanel() = default;

    void ScenePanel::OnRender(EditorContext& context)
    {
        RenderViewportToolbar(context);
        ImGui::Separator();

        RenderScene(context);
    }

    void ScenePanel::RenderViewportToolbar(EditorContext& context)
    {
        auto& gizmos = context.GetGizmos();

        if (ImGui::Button("Move (W)"))
            gizmos.SetMode(GizmoMode::Translate);
        ImGui::SameLine();
        if (ImGui::Button("Rotate (E)"))
            gizmos.SetMode(GizmoMode::Rotate);
        ImGui::SameLine();
        if (ImGui::Button("Scale (R)"))
            gizmos.SetMode(GizmoMode::Scale);

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        bool local = (gizmos.GetSpace() == GizmoSpace::Local);
        if (ImGui::Checkbox("Local", &local))
            gizmos.SetSpace(local ? GizmoSpace::Local : GizmoSpace::World);

        ImGui::SameLine();

        bool snap = gizmos.IsSnappingEnabled();
        if (ImGui::Checkbox("Snap", &snap))
            gizmos.SetSnapping(snap);
    }

    void ScenePanel::RenderScene(EditorContext& context)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x <= 0 || size.y <= 0)
            return;

        // Resize the framebuffer if needed.
        u32 w = static_cast<u32>(size.x);
        u32 h = static_cast<u32>(size.y);
        if (m_Framebuffer.NeedsResize(w, h) || !m_Framebuffer.IsValid())
        {
            m_Framebuffer.Resize(w, h);
            context.GetCamera().SetViewportSize(w, h);
        }

        // Don't render scene or display image if framebuffer is invalid.
        if (!m_Framebuffer.IsValid())
            return;

        // Update camera.
        auto& cam = context.GetCamera();

        // Render the scene into the framebuffer.
        Mat4 viewProjection = cam.GetProjectionMatrix() * cam.GetViewMatrix();
        m_SceneRenderer.RenderScene(context, m_Framebuffer, viewProjection,
                                     context.GetPreferences().GridVisible);

        // Display the framebuffer texture via ImGui.
        ImGui::Image(static_cast<ImTextureID>(m_Framebuffer.GetColorTextureID()),
                     size, ImVec2(0, 1), ImVec2(1, 0));

        // Handle mouse input for camera + picking.
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 imageSize = ImGui::GetItemRectSize();
        HandleMouseInput(context, imagePos, imageSize);

        m_IsFocused = ImGui::IsWindowFocused();
        m_IsHovered = ImGui::IsWindowHovered();

        // Render gizmos for the selected entity.
        context.GetGizmos().Render(context);
    }

    void ScenePanel::HandleMouseInput(EditorContext& context, ImVec2 imagePos, ImVec2 imageSize)
    {
        auto& cam = context.GetCamera();

        // Get mouse position relative to the viewport.
        ImVec2 mousePos = ImGui::GetMousePos();
        f32 mx = mousePos.x - imagePos.x;
        f32 my = mousePos.y - imagePos.y;

        // Check if mouse is inside the viewport.
        bool inside = mx >= 0 && mx < imageSize.x && my >= 0 && my < imageSize.y;

        if (!m_IsHovered || !inside)
            return;

        // Camera controls.
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
            (ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !ImGui::GetIO().KeyAlt))
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
            if (delta.x != 0 || delta.y != 0)
            {
                if (ImGui::GetIO().KeyShift)
                    cam.Pan(delta.x * 0.5f, delta.y * 0.5f);
                else
                    cam.Orbit(delta.x * 0.5f, delta.y * 0.5f);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
        }

        // Zoom with mouse wheel.
        f32 wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0)
            cam.Zoom(wheel);

        // Entity picking on left-click.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().KeyCtrl)
        {
            // Pick entity at mouse position.
            Mat4 vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
            m_Picking.RenderPickBuffer(m_Framebuffer, vp);

            u32 px = static_cast<u32>(mx);
            u32 py = static_cast<u32>(my);
            auto entity = m_Picking.PickAt(px, py);

            if (entity != engine::ecs::Invalid)
                context.GetSelection().SelectEntity(entity);
            else
                context.GetSelection().Clear();
        }
    }

} // namespace editor
