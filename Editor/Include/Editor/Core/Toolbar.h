// ============================================================================
// File: Editor/Include/Editor/Core/Toolbar.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace editor {

    class EditorContext;

    /// @brief The main editor toolbar.  Contains Play/Pause/Stop/Step
    ///        buttons, transform mode toggles, and quick-access buttons.
    class Toolbar
    {
    public:
        Toolbar() = default;
        ~Toolbar() = default;

        /// @brief Renders the toolbar.
        void Render(EditorContext& context);

    private:
        void RenderPlayControls(EditorContext& context);
        void RenderTransformMode(EditorContext& context);
        void RenderCameraControls(EditorContext& context);
        void RenderSnapToggle(EditorContext& context);
        void RenderQuickAccess(EditorContext& context);
    };

} // namespace editor
