// ============================================================================
// File: Editor/Source/Framework/PanelManager.cpp
// ============================================================================
#include "Editor/Framework/PanelManager.h"
#include "Editor/Core/EditorContext.h"
#include "Engine/Core/Log.h"

#include <imgui.h>

namespace editor {

    PanelManager::PanelManager() = default;

    PanelId PanelManager::Register(std::unique_ptr<IPanel> panel)
    {
        if (!panel)
        {
            ENGINE_LOG_ERROR("PanelManager — cannot register null panel");
            return 0;
        }

        std::string name = panel->GetName();
        if (m_ByName.count(name) > 0)
        {
            ENGINE_LOG_WARN("PanelManager — panel '{}' already registered", name);
            return 0;
        }

        IPanel* raw = panel.get();
        m_Panels.push_back(std::move(panel));
        m_ByName[name] = raw;

        return static_cast<PanelId>(m_Panels.size());
    }

    void PanelManager::OpenPanel(const std::string& name)
    {
        auto* p = FindPanel(name);
        if (p)
            p->SetOpen(true);
    }

    void PanelManager::ClosePanel(const std::string& name)
    {
        auto* p = FindPanel(name);
        if (p && p->CanClose())
            p->SetOpen(false);
    }

    void PanelManager::TogglePanel(const std::string& name)
    {
        auto* p = FindPanel(name);
        if (p && p->CanClose())
            p->SetOpen(!p->IsOpen());
    }

    bool PanelManager::IsOpen(const std::string& name) const
    {
        auto* p = FindPanel(name);
        return p ? p->IsOpen() : false;
    }

    IPanel* PanelManager::FindPanel(const std::string& name) const
    {
        auto it = m_ByName.find(name);
        return (it != m_ByName.end()) ? it->second : nullptr;
    }

    std::vector<IPanel*> PanelManager::GetAllPanels() const
    {
        std::vector<IPanel*> result;
        result.reserve(m_Panels.size());
        for (const auto& p : m_Panels)
            result.push_back(p.get());
        return result;
    }

    void PanelManager::RenderAll(EditorContext& context)
    {
        for (auto& panel : m_Panels)
        {
            if (!panel->IsOpen())
                continue;

            bool open = panel->IsOpen();
            ImGui::Begin(panel->GetName().c_str(), &open, panel->CanClose() ? 0 : ImGuiWindowFlags_NoCollapse);
            panel->OnRender(context);
            if (!open && panel->CanClose())
                panel->SetOpen(false);
            ImGui::End();
        }
    }

    engine::core::u32 PanelManager::GetCount() const noexcept
    {
        return static_cast<engine::core::u32>(m_Panels.size());
    }

} // namespace editor
