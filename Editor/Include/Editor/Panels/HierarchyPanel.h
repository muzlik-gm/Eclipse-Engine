// ============================================================================
// File: Editor/Include/Editor/Panels/HierarchyPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Scene/Scene.h"
#include <string>

namespace editor {

    /// @brief Displays the scene hierarchy — scenes, entities, and
    ///        parent-child relationships.  Supports selection, expansion,
    ///        collapse, and search.
    class HierarchyPanel final : public IPanel
    {
    public:
        HierarchyPanel();
        ~HierarchyPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Left; }

        void OnRender(EditorContext& context) override;

    private:
        void RenderEntityNode(EditorContext& context, engine::ecs::Entity entity, engine::scene::Scene& scene);
        void RenderSceneNode(EditorContext& context, engine::scene::Scene& scene);

        std::string m_Name{"Hierarchy"};
        std::string m_Title{"Hierarchy"};
        std::string m_SearchFilter;
        engine::ecs::Entity m_RenamingEntity{engine::ecs::Invalid};
    };

} // namespace editor
