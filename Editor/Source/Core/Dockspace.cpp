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
        // If no dockspace exists, create a default layout.
        static bool firstTime = true;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
                               | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                               | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                               | ImGuiWindowFlags_NoNavFocus;

        if (m_Fullscreen)
            flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("##Dockspace", nullptr, flags);
        ImGui::PopStyleVar(3);

        // Create the dockspace.
        ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");

        if (firstTime)
        {
            firstTime = false;

            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

            // Default layout: Hierarchy (left) | Scene (center) | Inspector (right)
            //                 Content Browser (bottom-left) | Console (bottom-right)
            auto dockLeft   = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.15f, nullptr, &dockspaceId);
            auto dockRight  = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.20f, nullptr, &dockspaceId);
            auto dockBottom = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.25f, nullptr, &dockspaceId);

            auto dockBottomLeft = ImGui::DockBuilderSplitNode(dockBottom, ImGuiDir_Left, 0.6f, nullptr, &dockBottom);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
            ImGui::DockBuilderDockWindow("Scene View", dockspaceId);
            ImGui::DockBuilderDockWindow("Game View", dockspaceId);
            ImGui::DockBuilderDockWindow("Inspector", dockRight);
            ImGui::DockBuilderDockWindow("Statistics", dockRight);
            ImGui::DockBuilderDockWindow("Content Browser", dockBottomLeft);
            ImGui::DockBuilderDockWindow("Console", dockBottom);
            ImGui::DockBuilderDockWindow("Profiler", dockBottom);

            ImGui::DockBuilderFinish(dockspaceId);
        }

        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f),
                         m_Fullscreen ? ImGuiDockNodeFlags_PassthruCentralNode : 0);

        ImGui::End();
    }

} // namespace editor
