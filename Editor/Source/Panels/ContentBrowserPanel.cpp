// ============================================================================
// File: Editor/Source/Panels/ContentBrowserPanel.cpp
// ============================================================================
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Core/EditorIcons.h"

#include <imgui.h>
#include <algorithm>

namespace editor {

    using engine::core::u32;

    namespace fs = std::filesystem;

    ContentBrowserPanel::ContentBrowserPanel()
    {
        if (fs::exists("assets"))
            m_RootDir = m_CurrentDir = fs::absolute("assets");
    }

    void ContentBrowserPanel::SetRootDirectory(const std::string& path)
    {
        m_RootDir = path;
        m_CurrentDir = path;
    }

    void ContentBrowserPanel::OnRender(EditorContext& context)
    {
        RenderBreadcrumbs();
        ImGui::Separator();

        // Search bar.
        char search[256] = {};
        std::copy(m_SearchFilter.begin(), m_SearchFilter.end(), search);
        ImGui::InputTextWithHint("##Search", "Search assets...", search, sizeof(search));
        m_SearchFilter = search;
        ImGui::Separator();

        // Two-column layout: directory tree on left, file list on right.
        float leftWidth = 200.0f;
        ImGui::BeginChild("##Tree", ImVec2(leftWidth, 0), true);
        RenderDirectoryTree();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##Files", ImVec2(0, 0), true);
        RenderFileList();
        ImGui::EndChild();
    }

    void ContentBrowserPanel::RenderBreadcrumbs()
    {
        auto rel = fs::relative(m_CurrentDir, m_RootDir);
        if (rel.empty() || rel.string() == ".")
        {
            ImGui::Text("Assets");
            return;
        }

        // Root button.
        if (ImGui::Button("Assets"))
            m_CurrentDir = m_RootDir;
        ImGui::SameLine();
        ImGui::Text("/");

        // Path components.
        fs::path accumulated = m_RootDir;
        for (const auto& part : rel)
        {
            accumulated /= part;
            ImGui::SameLine();
            if (ImGui::SmallButton(part.string().c_str()))
                m_CurrentDir = accumulated;
            ImGui::SameLine();
            ImGui::Text("/");
        }
    }

    void ContentBrowserPanel::RenderDirectoryTree()
    {
        if (!fs::exists(m_RootDir))
        {
            ImGui::TextDisabled("No assets directory");
            return;
        }

        // Recursively render subdirectories.
        std::function<void(const fs::path&)> renderDir = [&](const fs::path& dir)
        {
            auto name = dir.filename().string();
            if (name.empty())
                name = dir.string();

            bool isCurrent = (m_CurrentDir == dir);
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                     | ImGuiTreeNodeFlags_SpanAvailWidth;

            // Check if this directory has subdirectories.
            bool hasSubdirs = false;
            for (const auto& entry : fs::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    hasSubdirs = true;
                    break;
                }
            }
            if (!hasSubdirs)
                flags |= ImGuiTreeNodeFlags_Leaf;

            if (isCurrent)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool open = ImGui::TreeNodeEx(name.c_str(), flags);

            if (ImGui::IsItemClicked())
                m_CurrentDir = dir;

            if (open)
            {
                for (const auto& entry : fs::directory_iterator(dir))
                {
                    if (entry.is_directory())
                        renderDir(entry.path());
                }
                ImGui::TreePop();
            }
        };

        for (const auto& entry : fs::directory_iterator(m_RootDir))
        {
            if (entry.is_directory())
                renderDir(entry.path());
        }
    }

    void ContentBrowserPanel::RenderFileList()
    {
        if (!fs::exists(m_CurrentDir))
        {
            ImGui::TextDisabled("Directory not found");
            return;
        }

        // Up-one-level button.
        if (m_CurrentDir != m_RootDir && m_CurrentDir.has_parent_path())
        {
            if (ImGui::Button(".."))
                m_CurrentDir = m_CurrentDir.parent_path();
        }

        u32 itemCount = 0;
        for (const auto& entry : fs::directory_iterator(m_CurrentDir))
        {
            auto name = entry.path().filename().string();

            // Apply search filter.
            if (!m_SearchFilter.empty())
            {
                if (name.find(m_SearchFilter) == std::string::npos)
                    continue;
            }

            // Determine icon.
            std::string icon = "file";
            if (entry.is_directory())
                icon = "folder";
            else
            {
                auto ext = entry.path().extension().string();
                if (ext == ".png" || ext == ".jpg" || ext == ".tga") icon = "texture";
                else if (ext == ".glsl" || ext == ".vert" || ext == ".frag") icon = "shader";
                else if (ext == ".mat") icon = "material";
                else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") icon = "mesh";
                else if (ext == ".scene") icon = "scene";
                else if (ext == ".prefab") icon = "prefab";
                else if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") icon = "audio";
                else if (ext == ".ttf" || ext == ".otf") icon = "font";
                else if (ext == ".lua" || ext == ".py") icon = "script";
            }

            // Display as a selectable item with icon prefix.
            std::string label = "[" + icon + "] " + name;
            if (ImGui::Selectable(label.c_str(), false,
                                   ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (entry.is_directory())
                        m_CurrentDir = entry.path();
                    // Double-click on a file — future: open in appropriate editor.
                }
            }

            ++itemCount;
        }

        if (itemCount == 0 && m_SearchFilter.empty())
            ImGui::TextDisabled("Empty directory");
    }

} // namespace editor
