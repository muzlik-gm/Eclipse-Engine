// ============================================================================
// File: Editor/Source/Panels/HierarchyPanel.cpp
// ============================================================================
#include "Editor/Panels/HierarchyPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"
#include "Editor/Theme/ThemeManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Engine/Components/RelationshipComponent.h"
#include "Engine/Hierarchy/HierarchyManager.h"

#include <imgui.h>

namespace editor {

    HierarchyPanel::HierarchyPanel() = default;

    void HierarchyPanel::OnRender(EditorContext& context)
    {
        // Search bar
        char searchBuf[256] = {};
        std::copy(m_SearchFilter.begin(), m_SearchFilter.end(), searchBuf);
        ImGui::InputTextWithHint("##Search", "Search entities...", searchBuf, sizeof(searchBuf));
        m_SearchFilter = searchBuf;

        ImGui::Separator();

        auto* scene = context.GetActiveScene();
        if (!scene)
        {
            ImGui::TextDisabled("No active scene");
            return;
        }

        RenderSceneNode(context, *scene);
    }

    void HierarchyPanel::RenderSceneNode(EditorContext& context, engine::scene::Scene& scene)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.65f, 0.90f, 1.0f));

        bool sceneOpen = ImGui::TreeNodeEx("##Scene",
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_AllowOverlap, "%s", scene.GetName().c_str());

        ImGui::PopStyleColor();

        if (sceneOpen)
        {
            auto& registry = scene.GetRegistry();

            // Render root entities (those without a parent or with Invalid parent).
            auto view = registry.template View<engine::components::HierarchyComponent>();
            for (auto entity : view)
            {
                auto& hier = registry.GetComponent<engine::components::HierarchyComponent>(entity);
                if (hier.Parent == engine::ecs::Invalid)
                {
                    RenderEntityNode(context, entity, scene);
                }
            }

            // Also render entities with TagComponent but no HierarchyComponent.
            auto tagView = registry.template View<engine::components::TagComponent>();
            for (auto entity : tagView)
            {
                if (!registry.HasComponent<engine::components::HierarchyComponent>(entity))
                {
                    RenderEntityNode(context, entity, scene);
                }
            }

            ImGui::TreePop();
        }
    }

    void HierarchyPanel::RenderEntityNode(EditorContext& context, engine::ecs::Entity entity, engine::scene::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        // Get entity name.
        std::string name = "Entity";
        if (registry.HasComponent<engine::components::TagComponent>(entity))
            name = registry.GetComponent<engine::components::TagComponent>(entity).Tag;
        else if (registry.HasComponent<engine::components::NameComponent>(entity))
            name = registry.GetComponent<engine::components::NameComponent>(entity).Name;

        // Filter by search.
        if (!m_SearchFilter.empty())
        {
            if (name.find(m_SearchFilter) == std::string::npos)
                return;
        }

        // Determine children for tree expansion.
        bool hasChildren = false;
        if (registry.HasComponent<engine::components::HierarchyComponent>(entity))
        {
            hasChildren = registry.GetComponent<engine::components::HierarchyComponent>(entity).FirstChild
                          != engine::ecs::Invalid;
        }

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                 | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool isSelected = context.GetSelection().IsSelected(entity);
        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool nodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity)),
                                           flags, "%s", name.c_str());

        if (ImGui::IsItemClicked())
        {
            context.GetSelection().SelectEntity(entity);
        }

        if (nodeOpen)
        {
            if (hasChildren)
            {
                // Render children.
                auto& hier = registry.GetComponent<engine::components::HierarchyComponent>(entity);
                engine::ecs::Entity child = hier.FirstChild;
                while (child != engine::ecs::Invalid)
                {
                    RenderEntityNode(context, child, scene);
                    if (registry.HasComponent<engine::components::HierarchyComponent>(child))
                        child = registry.GetComponent<engine::components::HierarchyComponent>(child).NextSibling;
                    else
                        break;
                }
            }
            ImGui::TreePop();
        }
    }

} // namespace editor
