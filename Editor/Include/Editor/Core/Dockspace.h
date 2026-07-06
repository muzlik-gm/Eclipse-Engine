// ============================================================================
// File: Editor/Include/Editor/Core/Dockspace.h
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>

namespace editor {

    class EditorContext;

    /// @brief The main editor dockspace — hosts all dockable panels.
    class Dockspace
    {
    public:
        Dockspace();
        ~Dockspace() = default;

        /// @brief Renders the dockspace and all docked panels.
        /// @param context The editor context.
        void Render(EditorContext& context);

        /// @brief Returns the dockspace ID.
        [[nodiscard]] engine::core::u32 GetID() const noexcept { return m_DockspaceID; }

        /// @brief Returns true if the dockspace is fullscreen.
        [[nodiscard]] bool IsFullscreen() const noexcept { return m_Fullscreen; }
        void SetFullscreen(bool fullscreen) noexcept { m_Fullscreen = fullscreen; }

    private:
        engine::core::u32 m_DockspaceID{0};
        bool m_Fullscreen{true};
        bool m_Padding{false};
    };

} // namespace editor
