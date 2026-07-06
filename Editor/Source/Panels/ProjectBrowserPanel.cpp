// ============================================================================
// File: Editor/Source/Panels/ProjectBrowserPanel.cpp
// ============================================================================
#include "Editor/Panels/ProjectBrowserPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Project/ProjectManager.h"
#include "Engine/Core/Log.h"

#include <imgui.h>
#include <filesystem>

namespace editor {

    namespace fs = std::filesystem;

    ProjectBrowserPanel::ProjectBrowserPanel()
    {
        SetOpen(true);
    }

    void ProjectBrowserPanel::OnRender(EditorContext& context)
    {
        // Centered modal-like window.
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);

        if (ImGui::BeginTabBar("##ProjectBrowserTabs"))
        {
            if (ImGui::BeginTabItem("New Project"))
            {
                m_ActiveTab = 0;
                RenderNewProjectTab(context);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Open Project"))
            {
                m_ActiveTab = 1;
                RenderOpenProjectTab(context);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Recent Projects"))
            {
                m_ActiveTab = 2;
                RenderRecentProjectsTab(context);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void ProjectBrowserPanel::RenderNewProjectTab(EditorContext& context)
    {
        ImGui::Text("Create a new project");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Project Name:");
        ImGui::InputText("##ProjectName", m_NewProjectName, sizeof(m_NewProjectName));

        ImGui::Text("Location:");
        ImGui::InputText("##ProjectPath", m_NewProjectPath, sizeof(m_NewProjectPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse..."))
        {
            // In a full implementation, this would open a folder dialog.
            // For now, the user types the path.
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create Project", ImVec2(200, 30)))
        {
            std::string name(m_NewProjectName);
            std::string path(m_NewProjectPath);

            if (!name.empty() && !path.empty())
            {
                if (context.GetProjectManager().CreateProject(path, name))
                {
                    ENGINE_LOG_INFO("ProjectBrowser — created project '{}'", name);
                    m_ShouldClose = true;
                    SetOpen(false);
                }
            }
        }
    }

    void ProjectBrowserPanel::RenderOpenProjectTab(EditorContext& context)
    {
        ImGui::Text("Open an existing project");
        ImGui::Separator();
        ImGui::Spacing();

        static char filePath[512] = "";
        ImGui::Text("Project File (.eproject):");
        ImGui::InputText("##OpenPath", filePath, sizeof(filePath));
        ImGui::SameLine();
        if (ImGui::Button("Browse..."))
        {
            // Folder dialog would go here.
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Open Project", ImVec2(200, 30)))
        {
            std::string path(filePath);
            if (!path.empty() && fs::exists(path))
            {
                if (context.GetProjectManager().OpenProject(path))
                {
                    ENGINE_LOG_INFO("ProjectBrowser — opened project '{}'", path);
                    m_ShouldClose = true;
                    SetOpen(false);
                }
            }
        }
    }

    void ProjectBrowserPanel::RenderRecentProjectsTab(EditorContext& context)
    {
        ImGui::Text("Recent Projects");
        ImGui::Separator();
        ImGui::Spacing();

        const auto& recent = context.GetProjectManager().GetRecentProjects();

        if (recent.empty())
        {
            ImGui::TextDisabled("No recent projects.");
            return;
        }

        for (const auto& entry : recent)
        {
            ImGui::PushID(entry.FilePath.c_str());

            // Check if the file still exists.
            bool exists = fs::exists(entry.FilePath);

            if (!exists)
                ImGui::BeginDisabled();

            ImGui::Text("%s", entry.Name.c_str());
            ImGui::SameLine(200);
            ImGui::TextDisabled("%s", entry.LastOpened.c_str());
            ImGui::SameLine(350);

            if (ImGui::Button("Open"))
            {
                if (context.GetProjectManager().OpenProject(entry.FilePath))
                {
                    m_ShouldClose = true;
                    SetOpen(false);
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                context.GetProjectManager().RemoveRecentProject(entry.FilePath);
            }

            if (!exists)
                ImGui::EndDisabled();

            // Show full path.
            ImGui::TextDisabled("  %s", entry.FilePath.c_str());
            ImGui::Separator();

            ImGui::PopID();
        }
    }

} // namespace editor
