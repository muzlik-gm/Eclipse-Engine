// ============================================================================
// File: Editor/Source/Panels/ConsolePanel.cpp
// ============================================================================
#include "Editor/Panels/ConsolePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Theme/ThemeManager.h"

#include <imgui.h>
#include <chrono>
#include <ctime>
#include <functional>

namespace editor {

    ConsolePanel::ConsolePanel() = default;

    void ConsolePanel::AddMessage(const std::string& level, const std::string& message)
    {
        std::lock_guard lock(m_Mutex);

        // Generate timestamp.
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t, &tm);
        char ts[32];
        std::strftime(ts, sizeof(ts), "%H:%M:%S", &tm);

        u32 hash = static_cast<u32>(std::hash<std::string>{}(level + message));

        // Collapse duplicates if enabled.
        if (m_CollapseDuplicates && !m_Entries.empty())
        {
            auto& last = m_Entries.back();
            if (last.Hash == hash)
            {
                ++last.Count;
                return;
            }
        }

        LogEntry entry;
        entry.Level = level;
        entry.Message = message;
        entry.Timestamp = ts;
        entry.Hash = hash;
        m_Entries.push_back(std::move(entry));

        // Cap the number of entries.
        if (m_Entries.size() > 10000)
            m_Entries.erase(m_Entries.begin());
    }

    void ConsolePanel::OnRender(EditorContext& context)
    {
        // Filter bar.
        ImGui::Checkbox("Info", &m_ShowInfo); ImGui::SameLine();
        ImGui::Checkbox("Warn", &m_ShowWarning); ImGui::SameLine();
        ImGui::Checkbox("Error", &m_ShowError); ImGui::SameLine();

        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputTextWithHint("##Filter", "Filter...", m_FilterBuffer, sizeof(m_FilterBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            std::lock_guard lock(m_Mutex);
            m_Entries.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
        ImGui::SameLine();
        ImGui::Checkbox("Collapse", &m_CollapseDuplicates);

        ImGui::Separator();

        // Messages.
        ImGui::BeginChild("##Messages", ImVec2(0, 0), false,
                          ImGuiWindowFlags_HorizontalScrollbar);

        std::lock_guard lock(m_Mutex);
        for (const auto& entry : m_Entries)
        {
            // Level filter.
            bool show = false;
            if (entry.Level == "info"    && m_ShowInfo)    show = true;
            if (entry.Level == "warning" && m_ShowWarning) show = true;
            if (entry.Level == "error"   && m_ShowError)   show = true;
            if (entry.Level == "trace"   && m_ShowInfo)    show = true;
            if (entry.Level == "debug"   && m_ShowInfo)    show = true;
            if (!show)
                continue;

            // Text filter.
            if (m_FilterBuffer[0] != '\0')
            {
                if (entry.Message.find(m_FilterBuffer) == std::string::npos)
                    continue;
            }

            // Color by level.
            ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f);
            if (entry.Level == "warning") color = ImVec4(0.95f, 0.75f, 0.20f, 1.0f);
            if (entry.Level == "error")   color = ImVec4(0.90f, 0.30f, 0.30f, 1.0f);
            if (entry.Level == "trace")   color = ImVec4(0.50f, 0.50f, 0.55f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, color);

            // Format: [time] [LEVEL] message (count)
            if (entry.Count > 1)
                ImGui::Text("[%s] [%s] %s (x%u)", entry.Timestamp.c_str(),
                            entry.Level.c_str(), entry.Message.c_str(), entry.Count);
            else
                ImGui::Text("[%s] [%s] %s", entry.Timestamp.c_str(),
                            entry.Level.c_str(), entry.Message.c_str());

            ImGui::PopStyleColor();
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }

} // namespace editor
