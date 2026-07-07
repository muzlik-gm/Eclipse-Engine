// ============================================================================
// File: Editor/Source/Core/Dockspace.cpp
// ============================================================================
#include "Editor/Core/Dockspace.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Framework/PanelManager.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace editor {

    using engine::core::u32;

    Dockspace::Dockspace() = default;

    void Dockspace::Render(EditorContext& context)
    {
        static bool firstTime = true;

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Always resolve the root dockspace ID from the string.
        m_DockspaceID = ImGui::GetID("EditorDockspace");

        if (firstTime)
        {
            firstTime = false;

            // Save the root ID before any split operations overwrite it.
            ImGuiID rootID = m_DockspaceID;

            ImGui::DockBuilderRemoveNode(rootID);
            ImGui::DockBuilderAddNode(rootID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(rootID, viewport->WorkSize);

            // Default layout:
            // Hierarchy (left) | Scene+Game (center) | Inspector (right)
            // Content Browser (bottom-left) | Console (bottom-right)
            ImGuiID dockLeft   = 0;
            ImGuiID dockRight  = 0;
            ImGuiID dockBottom = 0;
            ImGuiID dockCenter = 0;

            ImGui::DockBuilderSplitNode(rootID, ImGuiDir_Left, 0.15f, &dockLeft, &dockCenter);
            ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.20f, &dockRight, &dockCenter);
            ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Down, 0.25f, &dockBottom, &dockCenter);

            ImGuiID dockBottomLeft  = 0;
            ImGuiID dockBottomRight = 0;
            ImGui::DockBuilderSplitNode(dockBottom, ImGuiDir_Left, 0.6f, &dockBottomLeft, &dockBottomRight);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
            ImGui::DockBuilderDockWindow("Project Browser", dockLeft);

            ImGui::DockBuilderDockWindow("Inspector", dockRight);
            ImGui::DockBuilderDockWindow("Statistics", dockRight);

            // Dock Scene View LAST so it becomes the active (top) tab.
            ImGui::DockBuilderDockWindow("Game View", dockCenter);
            ImGui::DockBuilderDockWindow("Scene View", dockCenter);

            ImGui::DockBuilderDockWindow("Content Browser", dockBottomLeft);
            ImGui::DockBuilderDockWindow("Console", dockBottomRight);
            ImGui::DockBuilderDockWindow("Profiler", dockBottomRight);

            ImGui::DockBuilderFinish(rootID);

            // Keep m_DockspaceID as the root for the DockSpace() call.
            m_DockspaceID = rootID;
        }

        // PassthruCentralNode lets the dockspace be transparent in the
        // center area so panels can render their own backgrounds.
        ImGui::DockSpace(m_DockspaceID, ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_PassthruCentralNode);
    }

} // namespace editor
