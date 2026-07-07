// ============================================================================
// File: Editor/Include/Editor/Framework/PanelManager.h
// Manages editor panels — registration, visibility, docking.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct ImGuiDockSpace;

namespace editor {

    class EditorContext;

    // ========================================================================
    // PanelId — unique identifier for a panel instance.
    // ========================================================================

    using PanelId = engine::core::u32;

    // ========================================================================
    // PanelLocation — where a panel docks by default.
    // ========================================================================

    enum class PanelLocation : engine::core::u32
    {
        Left       = 0,
        Right      = 1,
        Bottom     = 2,
        Center     = 3,
        Floating   = 4
    };

    // ========================================================================
    // IPanel — base interface for all editor panels.
    // ========================================================================

    /// @brief Base interface for every editor panel.  Panels are
    ///        registered with PanelManager and rendered each frame.
    class IPanel
    {
    public:
        virtual ~IPanel() = default;

        /// @brief Returns the panel's unique name (used for docking IDs).
        [[nodiscard]] virtual const std::string& GetName() const noexcept = 0;

        /// @brief Returns the panel's display title (shown in the tab).
        [[nodiscard]] virtual const std::string& GetTitle() const noexcept = 0;

        /// @brief Returns the default dock location.
        [[nodiscard]] virtual PanelLocation GetDefaultLocation() const noexcept = 0;

        /// @brief Returns additional ImGui window flags for this panel.
        ///        Override to e.g. add ImGuiWindowFlags_NoScrollbar.
        [[nodiscard]] virtual unsigned int GetWindowFlags() const noexcept { return 0; }

        /// @brief Renders the panel's content.  Called every frame
        ///        while the panel is open.
        virtual void OnRender(EditorContext& context) = 0;

        /// @brief Called once when the editor initializes.
        virtual void OnInitialize(EditorContext& /*context*/) {}

        /// @brief Called when the panel is opened.
        virtual void OnOpen(EditorContext& /*context*/) {}

        /// @brief Called when the panel is closed.
        virtual void OnClose(EditorContext& /*context*/) {}

        /// @brief Returns true if the panel is currently open.
        [[nodiscard]] bool IsOpen() const noexcept { return m_Open; }

        /// @brief Opens or closes the panel.
        void SetOpen(bool open) noexcept { m_Open = open; }

        /// @brief Returns true if the panel can be closed by the user.
        [[nodiscard]] virtual bool CanClose() const noexcept { return true; }

    protected:
        bool m_Open{true};
    };

    // ========================================================================
    // PanelManager — registers and manages all panels.
    // ========================================================================

    /// @brief Manages the lifecycle of all editor panels.  Panels are
    ///        registered once at startup and toggled open/closed at
    ///        runtime through the Window menu.
    class PanelManager
    {
    public:
        PanelManager();
        ~PanelManager() = default;

        /// @brief Registers a panel.  Takes ownership.
        /// @return The panel's ID.
        PanelId Register(std::unique_ptr<IPanel> panel);

        /// @brief Opens a panel by name.
        void OpenPanel(const std::string& name);

        /// @brief Closes a panel by name.
        void ClosePanel(const std::string& name);

        /// @brief Toggles a panel's open state.
        void TogglePanel(const std::string& name);

        /// @brief Returns true if @p name is currently open.
        [[nodiscard]] bool IsOpen(const std::string& name) const;

        /// @brief Returns the panel with @p name, or nullptr.
        [[nodiscard]] IPanel* FindPanel(const std::string& name) const;

        /// @brief Returns all registered panels.
        [[nodiscard]] std::vector<IPanel*> GetAllPanels() const;

        /// @brief Renders all open panels.  Called every frame.
        void RenderAll(EditorContext& context);

        /// @brief Returns the number of registered panels.
        [[nodiscard]] engine::core::u32 GetCount() const noexcept;

    private:
        std::vector<std::unique_ptr<IPanel>>               m_Panels;
        std::unordered_map<std::string, IPanel*>            m_ByName;
    };

} // namespace editor
