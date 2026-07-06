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
        // Create the dockspace on the CURRENT ImGui window (caller must
        // have already called ImGui::Begin).  We do NOT create our own
        // window here — the caller handles the window lifecycle.

        static bool firstTime = true;

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Create the dockspace.
        m_DockspaceID = ImGui::GetID("EditorDockspace");

        if (firstTime)
        {
            firstTime = false;

            ImGui::DockBuilderRemoveNode(m_DockspaceID);
            ImGui::DockBuilderAddNode(m_DockspaceID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_DockspaceID, viewport->WorkSize);

            // Default layout: Hierarchy (left) | Scene (center) | Inspector (right)
            //                 Content Browser (bottom-left) | Console (bottom-right)
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

            ImGui::DockBuilderFinish(m_DockspaceID);
        }

        ImGui::DockSpace(m_DockspaceID, ImVec2(0.0f, 0.0f),
                         m_Fullscreen ? ImGuiDockNodeFlags_PassthruCentralNode : 0);
    }

} // namespace editor
