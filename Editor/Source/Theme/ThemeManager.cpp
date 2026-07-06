// ============================================================================
// File: Editor/Source/Theme/ThemeManager.cpp
// ============================================================================
#include "Editor/Theme/ThemeManager.h"
#include "Engine/Core/Log.h"

#include <imgui.h>

namespace editor {

    ThemeManager::ThemeManager()
    {
        RegisterDefaultThemes();
    }

    void ThemeManager::RegisterDefaultThemes()
    {
        // Dark theme (default).
        EditorTheme dark;
        dark.Name = "Dark";
        RegisterTheme("Dark", dark);

        // Light theme.
        EditorTheme light;
        light.Name = "Light";
        light.Background = engine::math::Vec4(0.85f, 0.85f, 0.87f, 1.0f);
        light.PanelBackground = engine::math::Vec4(0.92f, 0.92f, 0.94f, 1.0f);
        light.PopupBackground = engine::math::Vec4(0.95f, 0.95f, 0.97f, 1.0f);
        light.Border = engine::math::Vec4(0.70f, 0.70f, 0.73f, 0.60f);
        light.Text = engine::math::Vec4(0.15f, 0.15f, 0.18f, 1.0f);
        light.TextDisabled = engine::math::Vec4(0.50f, 0.50f, 0.55f, 1.0f);
        light.Widget = engine::math::Vec4(0.80f, 0.80f, 0.83f, 1.0f);
        light.WidgetHovered = engine::math::Vec4(0.72f, 0.72f, 0.76f, 1.0f);
        light.WidgetActive = engine::math::Vec4(0.68f, 0.68f, 0.72f, 1.0f);
        light.WidgetDisabled = engine::math::Vec4(0.85f, 0.85f, 0.88f, 1.0f);
        RegisterTheme("Light", light);
    }

    bool ThemeManager::SetActiveTheme(const std::string& name)
    {
        if (m_Themes.find(name) == m_Themes.end())
        {
            ENGINE_LOG_WARN("ThemeManager — theme '{}' not found", name);
            return false;
        }
        m_ActiveThemeName = name;
        ApplyToImGui();
        return true;
    }

    void ThemeManager::RegisterTheme(const std::string& name, const EditorTheme& theme)
    {
        m_Themes[name] = theme;
    }

    std::vector<std::string> ThemeManager::GetThemeNames() const
    {
        std::vector<std::string> result;
        result.reserve(m_Themes.size());
        for (const auto& [name, _] : m_Themes)
            result.push_back(name);
        return result;
    }

    static ImVec4 ToImVec(const engine::math::Vec4& v)
    {
        return ImVec4(v.x, v.y, v.z, v.w);
    }

    void ThemeManager::ApplyToImGui() const
    {
        const auto& t = GetActiveTheme();
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding = t.WindowRounding;
        style.ChildRounding = t.WindowRounding;
        style.FrameRounding = t.FrameRounding;
        style.PopupRounding = t.WindowRounding;
        style.ScrollbarRounding = t.WindowRounding;
        style.GrabRounding = t.FrameRounding;
        style.TabRounding = t.TabRounding;

        style.WindowPadding = ImVec2(t.WindowPadding, t.WindowPadding);
        style.FramePadding = ImVec2(t.FramePadding, t.FramePadding);
        style.ItemSpacing = ImVec2(t.ItemSpacing, t.ItemSpacing);
        style.ItemInnerSpacing = ImVec2(t.ItemInnerSpacing, t.ItemInnerSpacing);
        style.IndentSpacing = t.IndentSpacing;
        style.ScrollbarSize = t.ScrollbarSize;
        style.GrabMinSize = t.GrabMinSize;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]         = ToImVec(t.PanelBackground);
        colors[ImGuiCol_ChildBg]          = ToImVec(t.PanelBackground);
        colors[ImGuiCol_PopupBg]          = ToImVec(t.PopupBackground);
        colors[ImGuiCol_Border]           = ToImVec(t.Border);
        colors[ImGuiCol_BorderShadow]     = ToImVec(t.Border);
        colors[ImGuiCol_Text]             = ToImVec(t.Text);
        colors[ImGuiCol_TextDisabled]     = ToImVec(t.TextDisabled);
        colors[ImGuiCol_FrameBg]          = ToImVec(t.Widget);
        colors[ImGuiCol_FrameBgHovered]   = ToImVec(t.WidgetHovered);
        colors[ImGuiCol_FrameBgActive]    = ToImVec(t.WidgetActive);
        colors[ImGuiCol_TitleBg]          = ToImVec(t.Widget);
        colors[ImGuiCol_TitleBgActive]    = ToImVec(t.WidgetActive);
        colors[ImGuiCol_MenuBarBg]        = ToImVec(t.PanelBackground);
        colors[ImGuiCol_ScrollbarBg]      = ToImVec(t.PanelBackground);
        colors[ImGuiCol_ScrollbarGrab]    = ToImVec(t.Widget);
        colors[ImGuiCol_ScrollbarGrabHovered] = ToImVec(t.WidgetHovered);
        colors[ImGuiCol_ScrollbarGrabActive]  = ToImVec(t.WidgetActive);
        colors[ImGuiCol_CheckMark]        = ToImVec(t.Accent);
        colors[ImGuiCol_SliderGrab]       = ToImVec(t.Accent);
        colors[ImGuiCol_SliderGrabActive] = ToImVec(t.AccentActive);
        colors[ImGuiCol_Button]           = ToImVec(t.Widget);
        colors[ImGuiCol_ButtonHovered]    = ToImVec(t.WidgetHovered);
        colors[ImGuiCol_ButtonActive]     = ToImVec(t.WidgetActive);
        colors[ImGuiCol_Header]           = ToImVec(t.Selection);
        colors[ImGuiCol_HeaderHovered]    = ToImVec(t.SelectionHovered);
        colors[ImGuiCol_HeaderActive]     = ToImVec(t.Selection);
        colors[ImGuiCol_Separator]        = ToImVec(t.Border);
        colors[ImGuiCol_SeparatorHovered] = ToImVec(t.Accent);
        colors[ImGuiCol_SeparatorActive]  = ToImVec(t.Accent);
        colors[ImGuiCol_ResizeGrip]       = ToImVec(t.Widget);
        colors[ImGuiCol_ResizeGripHovered]= ToImVec(t.WidgetHovered);
        colors[ImGuiCol_ResizeGripActive] = ToImVec(t.WidgetActive);
        colors[ImGuiCol_Tab]              = ToImVec(t.Widget);
        colors[ImGuiCol_TabHovered]       = ToImVec(t.WidgetHovered);
        colors[ImGuiCol_TabSelected]        = ToImVec(t.WidgetActive);
        colors[ImGuiCol_TabDimmed]     = ToImVec(t.Widget);
        colors[ImGuiCol_TabDimmedSelected] = ToImVec(t.WidgetActive);
        colors[ImGuiCol_DockingPreview]   = ToImVec(t.Accent);
        colors[ImGuiCol_DockingEmptyBg]   = ToImVec(t.Background);
        colors[ImGuiCol_PlotLines]        = ToImVec(t.Accent);
        colors[ImGuiCol_PlotHistogram]    = ToImVec(t.Accent);
    }

    engine::math::Vec4 ThemeManager::GetColor(const std::string& key) const
    {
        const auto& t = GetActiveTheme();
        if (key == "background")        return t.Background;
        if (key == "panel")             return t.PanelBackground;
        if (key == "popup")             return t.PopupBackground;
        if (key == "border")            return t.Border;
        if (key == "text")              return t.Text;
        if (key == "text_disabled")     return t.TextDisabled;
        if (key == "widget")            return t.Widget;
        if (key == "widget_hovered")   return t.WidgetHovered;
        if (key == "widget_active")    return t.WidgetActive;
        if (key == "accent")            return t.Accent;
        if (key == "accent_hovered")   return t.AccentHovered;
        if (key == "accent_active")    return t.AccentActive;
        if (key == "selection")         return t.Selection;
        if (key == "success")           return t.Success;
        if (key == "warning")           return t.Warning;
        if (key == "error")             return t.Error;
        if (key == "info")              return t.Info;
        if (key == "entity")            return t.Entity;
        if (key == "component")         return t.Component;
        if (key == "scene")             return t.Scene;
        if (key == "axis_x")            return t.AxisX;
        if (key == "axis_y")            return t.AxisY;
        if (key == "axis_z")            return t.AxisZ;
        return t.Text;
    }

} // namespace editor
