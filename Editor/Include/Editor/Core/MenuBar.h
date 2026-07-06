// ============================================================================
// File: Editor/Include/Editor/Core/MenuBar.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace editor {

    class EditorContext;

    /// @brief The main menu bar.  All menu items invoke commands
    ///        through the command system — no direct UI logic.
    class MenuBar
    {
    public:
        MenuBar() = default;
        ~MenuBar() = default;

        /// @brief Renders the menu bar.
        void Render(EditorContext& context);

    private:
        void RenderFileMenu(EditorContext& context);
        void RenderEditMenu(EditorContext& context);
        void RenderViewMenu(EditorContext& context);
        void RenderWindowMenu(EditorContext& context);
        void RenderToolsMenu(EditorContext& context);
        void RenderHelpMenu(EditorContext& context);

        /// @brief Renders a menu item that invokes a command.
        void MenuItem(EditorContext& context, const char* label, const char* commandName);
    };

} // namespace editor
