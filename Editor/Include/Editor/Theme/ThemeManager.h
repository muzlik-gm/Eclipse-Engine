// ============================================================================
// File: Editor/Include/Editor/Theme/ThemeManager.h
// Centralized editor theme — colors, fonts, spacing, styling.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

#include <string>
#include <unordered_map>
#include <vector>

struct ImGuiStyle;

namespace editor {

    using engine::core::u32;
    using engine::math::Vec4;

    // ========================================================================
    // EditorTheme — a complete set of theme colors and metrics.
    // ========================================================================

    struct EditorTheme
    {
        std::string Name{"Dark"};

        // -- Core colors ---------------------------------------------------
        Vec4 Background{0.10f, 0.10f, 0.12f, 1.00f};
        Vec4 PanelBackground{0.14f, 0.14f, 0.17f, 1.00f};
        Vec4 PopupBackground{0.16f, 0.16f, 0.20f, 1.00f};
        Vec4 Border{0.25f, 0.25f, 0.28f, 0.60f};

        // -- Text colors ---------------------------------------------------
        Vec4 Text{0.92f, 0.92f, 0.95f, 1.00f};
        Vec4 TextDisabled{0.50f, 0.50f, 0.55f, 1.00f};

        // -- Widget colors -------------------------------------------------
        Vec4 Widget{0.20f, 0.20f, 0.24f, 1.00f};
        Vec4 WidgetHovered{0.26f, 0.26f, 0.30f, 1.00f};
        Vec4 WidgetActive{0.30f, 0.30f, 0.35f, 1.00f};
        Vec4 WidgetDisabled{0.16f, 0.16f, 0.20f, 1.00f};

        // -- Accent colors -------------------------------------------------
        Vec4 Accent{0.26f, 0.59f, 0.98f, 1.00f};   // blue
        Vec4 AccentHovered{0.35f, 0.66f, 1.00f, 1.00f};
        Vec4 AccentActive{0.20f, 0.50f, 0.85f, 1.00f};

        // -- Selection -----------------------------------------------------
        Vec4 Selection{0.26f, 0.59f, 0.98f, 0.40f};
        Vec4 SelectionHovered{0.30f, 0.55f, 0.85f, 0.30f};

        // -- Status colors -------------------------------------------------
        Vec4 Success{0.30f, 0.75f, 0.35f, 1.00f};
        Vec4 Warning{0.95f, 0.75f, 0.20f, 1.00f};
        Vec4 Error{0.90f, 0.30f, 0.30f, 1.00f};
        Vec4 Info{0.30f, 0.65f, 0.95f, 1.00f};

        // -- Hierarchy / type colors ---------------------------------------
        Vec4 Entity{0.85f, 0.85f, 0.90f, 1.00f};
        Vec4 Component{0.65f, 0.75f, 0.90f, 1.00f};
        Vec4 Scene{0.75f, 0.65f, 0.90f, 1.00f};

        // -- Gizmo axis colors ---------------------------------------------
        Vec4 AxisX{0.90f, 0.25f, 0.25f, 1.00f};   // red
        Vec4 AxisY{0.25f, 0.90f, 0.35f, 1.00f};   // green
        Vec4 AxisZ{0.25f, 0.50f, 0.95f, 1.00f};   // blue

        // -- Spacing metrics -----------------------------------------------
        float WindowPadding{8.0f};
        float WindowRounding{4.0f};
        float FramePadding{4.0f};
        float FrameRounding{3.0f};
        float ItemSpacing{8.0f};
        float ItemInnerSpacing{4.0f};
        float IndentSpacing{16.0f};
        float ScrollbarSize{12.0f};
        float GrabMinSize{8.0f};
        float TabRounding{4.0f};
    };

    // ========================================================================
    // ThemeManager — manages editor themes.
    // ========================================================================

    /// @brief Centralized theme manager.  No panel may hardcode colors —
    ///        all colors are obtained through this manager.
    class ThemeManager
    {
    public:
        ThemeManager();
        ~ThemeManager() = default;

        /// @brief Returns the active theme.
        [[nodiscard]] const EditorTheme& GetActiveTheme() const noexcept
        { return m_Themes.at(m_ActiveThemeName); }

        /// @brief Returns the active theme name.
        [[nodiscard]] const std::string& GetActiveThemeName() const noexcept
        { return m_ActiveThemeName; }

        /// @brief Sets the active theme by name.
        /// @return True if the theme was found.
        bool SetActiveTheme(const std::string& name);

        /// @brief Registers a custom theme.
        void RegisterTheme(const std::string& name, const EditorTheme& theme);

        /// @brief Returns all registered theme names.
        [[nodiscard]] std::vector<std::string> GetThemeNames() const;

        /// @brief Applies the active theme to ImGui's global style.
        void ApplyToImGui() const;

        /// @brief Returns a color from the active theme by key name.
        ///        Keys: background, panel, text, accent, success, etc.
        [[nodiscard]] Vec4 GetColor(const std::string& key) const;

    private:
        void RegisterDefaultThemes();

        std::unordered_map<std::string, EditorTheme> m_Themes;
        std::string                                   m_ActiveThemeName{"Dark"};
    };

} // namespace editor
