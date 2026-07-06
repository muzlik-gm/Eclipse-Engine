// ============================================================================
// File: Editor/Include/Editor/Panels/InspectorPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Scene/Scene.h"
#include <string>

namespace editor {

    /// @brief Displays properties of the selected entity — UUID, name,
    ///        transform, and all attached components.
    class InspectorPanel final : public IPanel
    {
    public:
        InspectorPanel();
        ~InspectorPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Right; }

        void OnRender(EditorContext& context) override;

    private:
        void RenderEntityInfo(EditorContext& context, engine::ecs::Entity entity, engine::scene::Scene& scene);
        void RenderTransform(EditorContext& context, engine::ecs::Entity entity, engine::scene::Scene& scene);
        void RenderComponents(EditorContext& context, engine::ecs::Entity entity, engine::scene::Scene& scene);

        std::string m_Name{"Inspector"};
        std::string m_Title{"Inspector"};
    };

} // namespace editor
