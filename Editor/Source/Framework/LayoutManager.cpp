// ============================================================================
// File: Editor/Source/Framework/LayoutManager.cpp
// ============================================================================
#include "Editor/Framework/LayoutManager.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Framework/PanelManager.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace editor {

    namespace fs = std::filesystem;
    using json = nlohmann::json;

    LayoutManager::LayoutManager() = default;

    std::string LayoutManager::GetLayoutFilePath() const
    {
        return (fs::path(m_ConfigDir) / "editor_layout.json").string();
    }

    bool LayoutManager::Save(EditorContext& context)
    {
        // Capture the current ImGui layout.
        // ImGui stores its layout in imgui.ini; we copy it.
        m_Layout.DockLayoutIni = ""; // ImGui handles this internally.

        // Capture open panels.
        m_Layout.OpenPanels.clear();
        for (auto* panel : context.GetPanels().GetAllPanels())
        {
            if (panel->IsOpen())
                m_Layout.OpenPanels.push_back(panel->GetName());
        }

        try
        {
            fs::create_directories(m_ConfigDir);

            json j;
            j["open_panels"] = m_Layout.OpenPanels;
            j["window_x"] = m_Layout.WindowX;
            j["window_y"] = m_Layout.WindowY;
            j["window_w"] = m_Layout.WindowW;
            j["window_h"] = m_Layout.WindowH;
            j["maximized"] = m_Layout.Maximized;

            std::ofstream ofs(GetLayoutFilePath());
            if (!ofs.is_open())
            {
                ENGINE_LOG_ERROR("LayoutManager — cannot open '{}' for writing", GetLayoutFilePath());
                return false;
            }
            ofs << j.dump(4);
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("LayoutManager — save failed: {}", e.what());
            return false;
        }

        ENGINE_LOG_INFO("LayoutManager — layout saved to '{}'", GetLayoutFilePath());
        return true;
    }

    bool LayoutManager::Load(EditorContext& context)
    {
        try
        {
            std::ifstream ifs(GetLayoutFilePath());
            if (!ifs.is_open())
            {
                ENGINE_LOG_INFO("LayoutManager — no layout file found, using defaults");
                ResetToDefault();
                for (const auto& name : m_Layout.OpenPanels)
                    context.GetPanels().OpenPanel(name);
                return false;
            }

            json j;
            ifs >> j;

            m_Layout.OpenPanels.clear();
            if (j.contains("open_panels"))
            {
                for (const auto& p : j["open_panels"])
                    m_Layout.OpenPanels.push_back(p.get<std::string>());
            }

            m_Layout.WindowX = j.value("window_x", 100);
            m_Layout.WindowY = j.value("window_y", 100);
            m_Layout.WindowW = j.value("window_w", 1280);
            m_Layout.WindowH = j.value("window_h", 720);
            m_Layout.Maximized = j.value("maximized", false);

            // Apply open panel state.
            for (const auto& name : m_Layout.OpenPanels)
                context.GetPanels().OpenPanel(name);

            ENGINE_LOG_INFO("LayoutManager — layout loaded from '{}'", GetLayoutFilePath());
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("LayoutManager — load failed: {}", e.what());
            return false;
        }
    }

    void LayoutManager::ResetToDefault()
    {
        m_Layout = LayoutData{};
        m_Layout.OpenPanels = {
            "Hierarchy", "Inspector", "Scene View", "Game View",
            "Content Browser", "Console"
        };
    }

} // namespace editor
