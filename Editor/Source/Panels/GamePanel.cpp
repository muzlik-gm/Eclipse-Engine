// ============================================================================
// File: Editor/Source/Panels/GamePanel.cpp
// ============================================================================
#include "Editor/Panels/GamePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Commands/EditorCommandSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/TransformComponent.h"

#include <imgui.h>
#include <glad/glad.h>

namespace editor {

    using engine::core::u32;

    GamePanel::GamePanel() = default;

    void GamePanel::OnRender(EditorContext& context)
    {
        // Play/pause/stop/step controls.
        auto mode = context.GetMode();

        if (ImGui::Button(mode == EditorMode::Play ? "|| Pause" : "> Play"))
        {
            if (mode == EditorMode::Play)
                context.GetCommands().Execute("editor.pause");
            else
                context.GetCommands().Execute("editor.play");
        }
        ImGui::SameLine();
        if (ImGui::Button("[] Stop"))
            context.GetCommands().Execute("editor.stop");
        ImGui::SameLine();
        if (ImGui::Button(">| Step"))
            context.GetCommands().Execute("editor.step");

        ImGui::Separator();

        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x <= 0 || size.y <= 0)
            return;

        // Resize the framebuffer if needed.
        u32 w = static_cast<u32>(size.x);
        u32 h = static_cast<u32>(size.y);
        if (m_Framebuffer.NeedsResize(w, h) || !m_Framebuffer.IsValid())
            m_Framebuffer.Resize(w, h);

        if (!m_Framebuffer.IsValid())
            return;

        // Render the game view into the framebuffer.
        m_SceneRenderer.RenderGameView(context, m_Framebuffer);

        // Display the framebuffer texture.
        ImGui::Image(static_cast<ImTextureID>(m_Framebuffer.GetColorTextureID()),
                     size, ImVec2(0, 1), ImVec2(1, 0));

        // Display mode label overlay.
        const char* modeLabel = "Edit Mode";
        if (mode == EditorMode::Play)  modeLabel = "Playing";
        if (mode == EditorMode::Pause) modeLabel = "Paused";
        if (mode == EditorMode::Step)  modeLabel = "Stepping";

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetItemRectMin();
        dl->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + 100, p0.y + 20),
                          ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));
        dl->AddText(ImVec2(p0.x + 8, p0.y + 4),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.9f, 0.5f, 1.0f)),
                    modeLabel);
    }

} // namespace editor
