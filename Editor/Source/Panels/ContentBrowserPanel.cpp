// ============================================================================
// File: Editor/Source/Panels/ContentBrowserPanel.cpp
// ============================================================================
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Core/EditorIcons.h"
#include "Editor/Project/ProjectManager.h"

#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace editor {

    namespace fs = std::filesystem;
    using engine::core::u32;

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
        // If a project is open, use its asset directory.
        auto* proj = context.GetProjectManager().GetCurrentProject();
        if (proj && proj->IsOpen())
        {
            auto assetPath = proj->GetAssetPath();
            if (!assetPath.empty() && fs::exists(assetPath))
            {
                if (m_RootDir != assetPath)
                {
                    m_RootDir = assetPath;
                    m_CurrentDir = assetPath;
                }
            }
        }

        RenderToolbar();
        ImGui::Separator();
        RenderBreadcrumbs();
        ImGui::Separator();

        float leftWidth = 200.0f;
        ImGui::BeginChild("##Tree", ImVec2(leftWidth, 0), true);
        RenderDirectoryTree();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##Files", ImVec2(0, 0), true);
        RenderFileList();
        RenderContextMenu(context);
        ImGui::EndChild();
    }

    void ContentBrowserPanel::RenderToolbar()
    {
        // Grid / List toggle.
        if (ImGui::Button(m_GridView ? "Grid" : "List"))
            m_GridView = !m_GridView;

        ImGui::SameLine();

        // Search.
        char search[256] = {};
        std::copy(m_SearchFilter.begin(), m_SearchFilter.end(), search);
        ImGui::PushItemWidth(200);
        ImGui::InputTextWithHint("##Search", "Search assets...", search, sizeof(search));
        ImGui::PopItemWidth();
        m_SearchFilter = search;

        ImGui::SameLine();

        // Up one level.
        if (m_CurrentDir != m_RootDir && m_CurrentDir.has_parent_path())
        {
            if (ImGui::Button(".."))
                m_CurrentDir = m_CurrentDir.parent_path();
        }

        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
        {
            // No-op — content is scanned every frame.
        }
    }

    void ContentBrowserPanel::RenderBreadcrumbs()
    {
        auto rel = fs::relative(m_CurrentDir, m_RootDir);
        if (rel.empty() || rel.string() == ".")
        {
            ImGui::Text("Assets");
            return;
        }

        if (ImGui::Button("Assets"))
            m_CurrentDir = m_RootDir;
        ImGui::SameLine();
        ImGui::Text("/");

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

        std::function<void(const fs::path&)> renderDir = [&](const fs::path& dir)
        {
            auto name = dir.filename().string();
            if (name.empty())
                name = dir.string();

            bool isCurrent = (m_CurrentDir == dir);
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                     | ImGuiTreeNodeFlags_SpanAvailWidth;

            bool hasSubdirs = false;
            for (const auto& entry : fs::directory_iterator(dir))
            {
                if (entry.is_directory()) { hasSubdirs = true; break; }
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

        u32 itemCount = 0;

        // Collect entries.
        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(m_CurrentDir))
        {
            auto name = entry.path().filename().string();
            if (!m_SearchFilter.empty())
            {
                if (name.find(m_SearchFilter) == std::string::npos)
                    continue;
            }
            entries.push_back(entry);
        }

        // Sort: directories first, then by name.
        std::sort(entries.begin(), entries.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b)
            {
                if (a.is_directory() != b.is_directory())
                    return a.is_directory() > b.is_directory();
                return a.path().filename() < b.path().filename();
            });

        if (m_GridView)
        {
            // Grid view: items in a flowing layout.
            float cellSize = 80.0f;
            float availWidth = ImGui::GetContentRegionAvail().x;
            int perRow = std::max(1, static_cast<int>(availWidth / (cellSize + 8)));

            for (u32 i = 0; i < entries.size(); ++i)
            {
                if (i % perRow != 0)
                    ImGui::SameLine();

                RenderGridItem(entries[i].path(), entries[i].is_directory(), i);
                ++itemCount;
            }
        }
        else
        {
            // List view.
            for (const auto& entry : entries)
            {
                auto name = entry.path().filename().string();
                std::string icon = entry.is_directory() ? "[Folder] " : "[File] ";
                std::string label = icon + name;

                bool isSelected = (m_SelectedPath == entry.path());
                if (ImGui::Selectable(label.c_str(), isSelected,
                                       ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_SelectedPath = entry.path();

                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        if (entry.is_directory())
                            m_CurrentDir = entry.path();
                    }
                }

                if (ImGui::BeginPopupContextItem())
                {
                    m_SelectedPath = entry.path();
                    ImGui::EndPopup();
                }
                ++itemCount;
            }
        }

        // Right-click context menu on empty space.
        if (ImGui::BeginPopupContextWindow("##ContentBrowserContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            m_ShowContextMenu = true;
            ImGui::CloseCurrentPopup();
        }

        if (itemCount == 0 && m_SearchFilter.empty())
            ImGui::TextDisabled("Empty directory");
    }

    void ContentBrowserPanel::RenderGridItem(const fs::path& path, bool isDir, u32 index)
    {
        auto name = path.filename().string();

        ImGui::PushID(index);

        bool isSelected = (m_SelectedPath == path);
        if (isSelected)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.5f, 0.9f, 0.3f));

        ImGui::BeginChild("##GridItem", ImVec2(80, 80), true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Icon (text-based placeholder).
        const char* iconText = isDir ? "[F]" : "[*]";
        float iconWidth = ImGui::CalcTextSize(iconText).x;
        ImGui::SetCursorPosX((80 - iconWidth) * 0.5f);
        ImGui::Text("%s", iconText);

        // Name (truncated).
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 76);
        std::string truncated = name;
        if (truncated.length() > 12)
            truncated = truncated.substr(0, 10) + "..";
        ImGui::TextWrapped("%s", truncated.c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndChild();

        if (isSelected)
            ImGui::PopStyleColor();

        // Click handling.
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_SelectedPath = path;
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (isDir)
                m_CurrentDir = path;
        }

        // Right-click context menu for this item.
        if (ImGui::BeginPopupContextItem("##ItemContext"))
        {
            m_SelectedPath = path;
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    void ContentBrowserPanel::RenderContextMenu(EditorContext& context)
    {
        (void)context;

        // Right-click context menu on the file list area.
        if (ImGui::BeginPopupContextWindow("##ContentContext", ImGuiPopupFlags_MouseButtonRight))
        {
            if (!m_SelectedPath.empty() && fs::exists(m_SelectedPath))
            {
                if (ImGui::MenuItem("Rename"))
                {
                    m_IsRenaming = true;
                    m_RenamingPath = m_SelectedPath;
                    auto name = m_RenamingPath.filename().stem().string();
                    std::copy(name.begin(), name.end(), m_RenameBuffer);
                    m_RenameBuffer[name.size()] = '\0';
                }

                if (ImGui::MenuItem("Duplicate"))
                {
                    auto ext = m_SelectedPath.extension();
                    auto stem = m_SelectedPath.stem().string();
                    auto parent = m_SelectedPath.parent_path();
                    auto dest = parent / (stem + "_copy" + ext.string());

                    if (!fs::exists(dest))
                    {
                        if (fs::is_directory(m_SelectedPath))
                            fs::copy(m_SelectedPath, dest, fs::copy_options::recursive);
                        else
                            fs::copy_file(m_SelectedPath, dest);
                    }
                }

                if (ImGui::MenuItem("Delete"))
                {
                    if (fs::is_directory(m_SelectedPath))
                        fs::remove_all(m_SelectedPath);
                    else
                        fs::remove(m_SelectedPath);
                    m_SelectedPath.clear();
                }

                ImGui::Separator();
            }

            if (ImGui::MenuItem("Create Folder"))
            {
                auto newDir = m_CurrentDir / "NewFolder";
                u32 counter = 1;
                while (fs::exists(newDir))
                {
                    newDir = m_CurrentDir / ("NewFolder_" + std::to_string(counter++));
                }
                fs::create_directory(newDir);
                m_SelectedPath = newDir;
                m_IsRenaming = true;
                m_RenamingPath = newDir;
                std::string name = "NewFolder";
                std::copy(name.begin(), name.end(), m_RenameBuffer);
                m_RenameBuffer[name.size()] = '\0';
            }

            if (ImGui::MenuItem("Create Scene"))
            {
                auto scenePath = m_CurrentDir / "NewScene.scene";
                u32 counter = 1;
                while (fs::exists(scenePath))
                {
                    scenePath = m_CurrentDir / ("NewScene_" + std::to_string(counter++) + ".scene");
                }

                // Create an empty scene file.
                std::ofstream ofs(scenePath);
                ofs << "{\"name\":\"NewScene\",\"uuid\":\"00000000-0000-0000-0000-000000000000\",\"entities\":[]}";
            }

            if (ImGui::MenuItem("Refresh"))
            {
                // No-op — content is scanned every frame.
            }

            ImGui::EndPopup();
        }

        // Rename dialog.
        if (m_IsRenaming)
        {
            ImGui::OpenPopup("Rename Item##RenamePopup");
            m_IsRenaming = false;
        }

        if (ImGui::BeginPopupModal("Rename Item##RenamePopup", nullptr,
                                    ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("New name:");
            ImGui::InputText("##RenameInput", m_RenameBuffer, sizeof(m_RenameBuffer));

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                std::string newName(m_RenameBuffer);
                if (!newName.empty() && !m_RenamingPath.empty())
                {
                    auto ext = m_RenamingPath.extension();
                    auto parent = m_RenamingPath.parent_path();
                    auto dest = parent / (newName + ext.string());

                    if (!fs::exists(dest))
                        fs::rename(m_RenamingPath, dest);
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

} // namespace editor
