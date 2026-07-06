// ============================================================================
// File: Editor/Include/Editor/Panels/ConsolePanel.h
// ============================================================================
#pragma once

#include "Editor/Framework/PanelManager.h"
#include "Engine/Core/Types.h"
#include <mutex>
#include <string>
#include <vector>

namespace editor {

    using engine::core::u32;

    /// @brief Console panel — displays engine log messages with
    ///        filtering, auto-scroll, clear, and collapse-duplicates.
    class ConsolePanel final : public IPanel
    {
    public:
        ConsolePanel();
        ~ConsolePanel() override = default;

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] const std::string& GetTitle() const noexcept override { return m_Title; }
        [[nodiscard]] PanelLocation GetDefaultLocation() const noexcept override { return PanelLocation::Bottom; }

        void OnRender(EditorContext& context) override;

        /// @brief Adds a log message to the console.
        void AddMessage(const std::string& level, const std::string& message);

    private:
        struct LogEntry
        {
            std::string Level;
            std::string Message;
            std::string Timestamp;
            u32         Count{1};
            u32         Hash{0};
        };

        std::string m_Name{"Console"};
        std::string m_Title{"Console"};
        std::vector<LogEntry> m_Entries;
        mutable std::mutex    m_Mutex;

        bool m_ShowInfo{true};
        bool m_ShowWarning{true};
        bool m_ShowError{true};
        bool m_AutoScroll{true};
        bool m_CollapseDuplicates{true};
        char m_FilterBuffer[256]{};
    };

} // namespace editor
