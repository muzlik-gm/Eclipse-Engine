// ============================================================================
// File: Editor/Source/Panels/GamePanel.cpp
// ============================================================================
#include "Editor/Panels/GamePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/TransformComponent.h"

#include <imgui.h>

namespace editor {

    GamePanel::GamePanel() = default;

    void GamePanel::OnRender(EditorContext& context)
    {
        // Toolbar with play/pause/resume controls.
        auto mode = context.GetMode();

        if (ImGui::Button(mode == EditorMode::Play ? "Pause" : "Play"))
        {
            context.SetMode(mode == EditorMode::Play ? EditorMode::Edit : EditorMode::Play);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
            context.SetMode(EditorMode::Edit);
        ImGui::SameLine();
        if (ImGui::Button("Step"))
            context.SetMode(EditorMode::Step);

        ImGui::Separator();

        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x <= 0 || size.y <= 0)
            return;

        // Render placeholder — actual runtime camera rendering is wired
        // during the rendering systems phase.
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);

        // Dark background to simulate game viewport.
        dl->AddRectFilled(p0, p1, ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.05f, 0.05f, 0.08f, 1.0f)));

        // Display mode label.
        const char* modeLabel = "Edit Mode";
        if (mode == EditorMode::Play)  modeLabel = "Playing";
        if (mode == EditorMode::Pause) modeLabel = "Paused";
        if (mode == EditorMode::Step)  modeLabel = "Stepping";

        dl->AddText(ImVec2(p0.x + 8, p0.y + 8),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.9f, 0.5f, 1.0f)),
            modeLabel);

        ImGui::Dummy(size);
    }

} // namespace editor
