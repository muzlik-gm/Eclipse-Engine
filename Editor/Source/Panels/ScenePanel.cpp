// ============================================================================
// File: Editor/Source/Panels/ScenePanel.cpp
// ============================================================================
#include "Editor/Panels/ScenePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Camera/EditorCamera.h"
#include "Editor/Gizmos/GizmoManager.h"
#include "Editor/Theme/ThemeManager.h"
#include "Editor/Prefs/EditorPreferences.h"
#include "Engine/Scene/Scene.h"

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
        // Transform mode buttons (W/E/R for Translate/Rotate/Scale)
        auto& gizmos = context.GetGizmos();

        if (ImGui::Button("Translate (W)"))
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

        // Local/World toggle
        bool local = (gizmos.GetSpace() == GizmoSpace::Local);
        if (ImGui::Checkbox("Local", &local))
            gizmos.SetSpace(local ? GizmoSpace::Local : GizmoSpace::World);

        ImGui::SameLine();

        // Snapping toggle
        bool snap = gizmos.IsSnappingEnabled();
        if (ImGui::Checkbox("Snap", &snap))
            gizmos.SetSnapping(snap);
    }

    void ScenePanel::RenderScene(EditorContext& context)
    {
        // Get the viewport size.
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x <= 0 || size.y <= 0)
            return;

        // Update the editor camera viewport.
        context.GetCamera().SetViewportSize(
            static_cast<u32>(size.x), static_cast<u32>(size.y));

        // Render a placeholder colored rectangle (actual scene rendering
        // is wired in during the rendering systems phase).
        // For now, we clear the viewport with the editor background color.
        auto& theme = context.GetTheme();
        auto bg = theme.GetColor("background");

        // Use ImGui's framebuffer to draw a colored rect.
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);
        dl->AddRectFilled(p0, p1, ImGui::ColorConvertFloat4ToU32(
            ImVec4(bg.x, bg.y, bg.z, bg.w)));

        // Draw a simple grid.
        if (context.GetPreferences().GridVisible)
        {
            float gridSize = context.GetPreferences().GridSize;
            int divisions = context.GetPreferences().GridDivisions;
            ImU32 gridColor = ImGui::ColorConvertFloat4ToU32(
                ImVec4(0.5f, 0.5f, 0.5f, 0.3f));

            // Simple screen-space grid (placeholder).
            for (int i = 0; i <= divisions; ++i)
            {
                float x = p0.x + (size.x / divisions) * i;
                dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), gridColor);
                float y = p0.y + (size.y / divisions) * i;
                dl->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), gridColor);
            }
        }

        // Origin indicator.
        ImVec2 center(p0.x + size.x * 0.5f, p0.y + size.y * 0.5f);
        dl->AddCircleFilled(center, 4.0f, ImGui::ColorConvertFloat4ToU32(
            ImVec4(1.0f, 1.0f, 1.0f, 0.8f)));

        // Reserve the space so the panel has the right size.
        ImGui::Dummy(size);

        m_IsFocused = ImGui::IsWindowFocused();
        m_IsHovered = ImGui::IsWindowHovered();

        // Render gizmos for the selected entity.
        context.GetGizmos().Render(context);
    }

} // namespace editor
