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

        m_DockspaceID = ImGui::GetID("EditorDockspace");

        if (firstTime)
        {
            firstTime = false;

            ImGui::DockBuilderRemoveNode(m_DockspaceID);
            ImGui::DockBuilderAddNode(m_DockspaceID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_DockspaceID, viewport->WorkSize);

            // Default layout:
            // Hierarchy (left) | Scene+Game (center) | Inspector (right)
            // Content Browser (bottom-left) | Console (bottom-right)
            auto dockLeft   = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Left, 0.15f, nullptr, &m_DockspaceID);
            auto dockRight  = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Right, 0.20f, nullptr, &m_DockspaceID);
            auto dockBottom = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Down, 0.25f, nullptr, &m_DockspaceID);

            auto dockBottomLeft = ImGui::DockBuilderSplitNode(dockBottom, ImGuiDir_Left, 0.6f, nullptr, &dockBottom);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
            ImGui::DockBuilderDockWindow("Scene View", m_DockspaceID);
            ImGui::DockBuilderDockWindow("Game View", m_DockspaceID);
            ImGui::DockBuilderDockWindow("Inspector", dockRight);
            ImGui::DockBuilderDockWindow("Statistics", dockRight);
            ImGui::DockBuilderDockWindow("Content Browser", dockBottomLeft);
            ImGui::DockBuilderDockWindow("Console", dockBottom);
            ImGui::DockBuilderDockWindow("Profiler", dockBottom);
            ImGui::DockBuilderDockWindow("Project Browser", dockLeft);

            ImGui::DockBuilderFinish(m_DockspaceID);
        }

        // PassthruCentralNode lets the dockspace be transparent in the
        // center area so panels can render their own backgrounds.
        ImGui::DockSpace(m_DockspaceID, ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_PassthruCentralNode);
    }

} // namespace editor
