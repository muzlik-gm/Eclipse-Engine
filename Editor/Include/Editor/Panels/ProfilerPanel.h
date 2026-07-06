// ============================================================================
// File: Editor/Include/Editor/Panels/ProfilerPanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Engine/Core/Types.h"
#include <deque>
#include <string>

namespace editor {

    /// @brief Profiler window — basic framework with timeline, frame
    ///        history, CPU/memory/GPU graphs.
    class ProfilerPanel final : public IPanel
    {
    public:
        ProfilerPanel();
        ~ProfilerPanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Bottom; }

        void OnRender(EditorContext& context) override;

    private:
        void RenderTimeline(EditorContext& context);
        void RenderCPUGraph(EditorContext& context);
        void RenderMemoryGraph();
        void RenderGPUGraph();

        std::string m_Name{"Profiler"};
        std::string m_Title{"Profiler"};

        static constexpr engine::core::u32 kMaxFrames = 240;
        std::deque<float> m_FrameTimes;
        std::deque<float> m_MemoryUsage;
        std::deque<float> m_GPUTimes;

        bool m_Paused{false};
    };

} // namespace editor
