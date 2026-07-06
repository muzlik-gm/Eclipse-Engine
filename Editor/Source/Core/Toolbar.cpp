// ============================================================================
// File: Editor/Source/Core/Toolbar.cpp
// ============================================================================
#include "Editor/Core/Toolbar.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Commands/EditorCommandSystem.h"
#include "Editor/Framework/PanelManager.h"
#include "Editor/Gizmos/GizmoManager.h"

#include <imgui.h>

namespace editor {

    void Toolbar::Render(EditorContext& context)
    {
        // The toolbar is rendered as a child window below the menu bar.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
        ImGui::BeginChild("##Toolbar", ImVec2(0, 32), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        RenderPlayControls(context);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        RenderTransformMode(context);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        RenderCameraControls(context);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        RenderSnapToggle(context);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        RenderQuickAccess(context);

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void Toolbar::RenderPlayControls(EditorContext& context)
    {
        auto mode = context.GetMode();

        // Play button.
        bool isPlaying = (mode == EditorMode::Play);
        if (isPlaying)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("> Play"))
            context.GetCommands().Execute("editor.play");
        if (isPlaying)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        // Pause button.
        bool isPaused = (mode == EditorMode::Pause);
        if (isPaused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("|| Pause"))
            context.GetCommands().Execute("editor.pause");
        if (isPaused)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        // Stop button.
        if (ImGui::Button("[] Stop"))
            context.GetCommands().Execute("editor.stop");

        ImGui::SameLine();

        // Step button.
        if (ImGui::Button(">| Step"))
            context.GetCommands().Execute("editor.step");
    }

    void Toolbar::RenderTransformMode(EditorContext& context)
    {
        auto& gizmos = context.GetGizmos();

        // Translate (W)
        if (gizmos.GetMode() == GizmoMode::Translate)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        if (ImGui::Button("Move"))
            gizmos.SetMode(GizmoMode::Translate);
        if (gizmos.GetMode() == GizmoMode::Translate)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        // Rotate (E)
        if (gizmos.GetMode() == GizmoMode::Rotate)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        if (ImGui::Button("Rotate"))
            gizmos.SetMode(GizmoMode::Rotate);
        if (gizmos.GetMode() == GizmoMode::Rotate)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        // Scale (R)
        if (gizmos.GetMode() == GizmoMode::Scale)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        if (ImGui::Button("Scale"))
            gizmos.SetMode(GizmoMode::Scale);
        if (gizmos.GetMode() == GizmoMode::Scale)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        // Local/World toggle.
        bool local = (gizmos.GetSpace() == GizmoSpace::Local);
        if (ImGui::Checkbox("Local", &local))
            gizmos.SetSpace(local ? GizmoSpace::Local : GizmoSpace::World);
    }

    void Toolbar::RenderCameraControls(EditorContext& context)
    {
        if (ImGui::Button("Focus (F)"))
            context.GetCommands().Execute("view.frame_selected");

        ImGui::SameLine();

        if (ImGui::Button("Reset Cam"))
            context.GetCommands().Execute("view.reset_camera");
    }

    void Toolbar::RenderSnapToggle(EditorContext& context)
    {
        auto& gizmos = context.GetGizmos();
        bool snap = gizmos.IsSnappingEnabled();
        if (ImGui::Checkbox("Snap", &snap))
            gizmos.SetSnapping(snap);
    }

    void Toolbar::RenderQuickAccess(EditorContext& context)
    {
        if (ImGui::Button("Stats"))
            context.GetPanels().TogglePanel("Statistics");

        ImGui::SameLine();

        if (ImGui::Button("Profiler"))
            context.GetPanels().TogglePanel("Profiler");

        ImGui::SameLine();

        if (ImGui::Button("Console"))
            context.GetPanels().TogglePanel("Console");
    }

} // namespace editor
