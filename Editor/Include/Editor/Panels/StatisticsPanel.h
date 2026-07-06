// ============================================================================
// File: Editor/Include/Editor/Panels/StatisticsPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include <string>

namespace editor {

    /// @brief Statistics window — displays FPS, frame time, draw calls,
    ///        triangles, vertices, entities, textures, and memory.
    class StatisticsPanel final : public IPanel
    {
    public:
        StatisticsPanel();
        ~StatisticsPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Right; }

        void OnRender(EditorContext& context) override;

    private:
        std::string m_Name{"Statistics"};
        std::string m_Title{"Statistics"};
    };

} // namespace editor
