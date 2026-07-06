// ============================================================================
// File: Editor/Source/Panels/ProfilerPanel.cpp
// ============================================================================
#include "Editor/Panels/ProfilerPanel.h"
#include "Editor/Core/EditorContext.h"

#include <imgui.h>
#include <algorithm>
#include <vector>

namespace editor {

    using engine::core::u32;

    ProfilerPanel::ProfilerPanel() = default;

    // Helper: copy deque to a vector for ImGui PlotHistogram.
    static std::vector<float> DequeToVector(const std::deque<float>& d)
    {
        return {d.begin(), d.end()};
    }

    void ProfilerPanel::OnRender(EditorContext& context)
    {
        if (ImGui::Button(m_Paused ? "Resume" : "Pause"))
            m_Paused = !m_Paused;

        ImGui::SameLine();
        ImGui::Text("Frames recorded: %u", static_cast<u32>(m_FrameTimes.size()));

        ImGui::Separator();

        // Record frame time.
        if (!m_Paused)
        {
            float frameMs = static_cast<float>(context.GetDeltaTime() * 1000.0);
            m_FrameTimes.push_back(frameMs);
            if (m_FrameTimes.size() > kMaxFrames)
                m_FrameTimes.pop_front();

            // Placeholder memory/GPU data.
            m_MemoryUsage.push_back(50.0f + static_cast<float>(std::rand() % 20));
            if (m_MemoryUsage.size() > kMaxFrames)
                m_MemoryUsage.pop_front();

            m_GPUTimes.push_back(frameMs * 0.6f);
            if (m_GPUTimes.size() > kMaxFrames)
                m_GPUTimes.pop_front();
        }

        RenderCPUGraph(context);
        ImGui::Separator();
        RenderMemoryGraph();
        ImGui::Separator();
        RenderGPUGraph();
        ImGui::Separator();
        RenderTimeline(context);
    }

    void ProfilerPanel::RenderCPUGraph(EditorContext& context)
    {
        if (ImGui::CollapsingHeader("CPU Frame Time", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_FrameTimes.empty())
            {
                ImGui::TextDisabled("No data");
                return;
            }

            float maxVal = *std::max_element(m_FrameTimes.begin(), m_FrameTimes.end());
            maxVal = std::max(maxVal, 16.7f); // At least show 60fps line

            char overlay[32];
            std::snprintf(overlay, sizeof(overlay), "%.2f ms", m_FrameTimes.back());
            ImGui::PlotHistogram("##CPUTime", DequeToVector(m_FrameTimes).data(),
                                  static_cast<int>(m_FrameTimes.size()),
                                  0, overlay, 0.0f, maxVal * 1.2f, ImVec2(0, 80));
        }
    }

    void ProfilerPanel::RenderMemoryGraph()
    {
        if (ImGui::CollapsingHeader("Memory Usage"))
        {
            if (m_MemoryUsage.empty())
            {
                ImGui::TextDisabled("No data");
                return;
            }

            float maxVal = *std::max_element(m_MemoryUsage.begin(), m_MemoryUsage.end());
            char overlay[32];
            std::snprintf(overlay, sizeof(overlay), "%.1f MB", m_MemoryUsage.back());
            ImGui::PlotLines("##Memory", DequeToVector(m_MemoryUsage).data(),
                              static_cast<int>(m_MemoryUsage.size()),
                              0, overlay, 0.0f, maxVal * 1.2f, ImVec2(0, 80));
        }
    }

    void ProfilerPanel::RenderGPUGraph()
    {
        if (ImGui::CollapsingHeader("GPU Time (framework)"))
        {
            if (m_GPUTimes.empty())
            {
                ImGui::TextDisabled("No data");
                return;
            }

            float maxVal = *std::max_element(m_GPUTimes.begin(), m_GPUTimes.end());
            maxVal = std::max(maxVal, 16.7f);
            char overlay[32];
            std::snprintf(overlay, sizeof(overlay), "%.2f ms", m_GPUTimes.back());
            ImGui::PlotHistogram("##GPUTime", DequeToVector(m_GPUTimes).data(),
                                  static_cast<int>(m_GPUTimes.size()),
                                  0, overlay, 0.0f, maxVal * 1.2f, ImVec2(0, 80));
        }
    }

    void ProfilerPanel::RenderTimeline(EditorContext& context)
    {
        if (ImGui::CollapsingHeader("Timeline"))
        {
            ImGui::Text("Timeline framework — actual profiling zones expand here.");
            ImGui::Text("Frame: %.2f ms", static_cast<float>(context.GetDeltaTime() * 1000.0));

            // Placeholder timeline bar.
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + 30.0f);
            dl->AddRectFilled(p0, p1, ImGui::ColorConvertFloat4ToU32(
                ImVec4(0.2f, 0.3f, 0.5f, 0.6f)));
            dl->AddRect(p0, p1, ImGui::ColorConvertFloat4ToU32(
                ImVec4(0.5f, 0.6f, 0.8f, 1.0f)));

            ImGui::Dummy(ImVec2(0, 35));
        }
    }

} // namespace editor
