// ============================================================================
// File: Editor/Source/Commands/ShortcutManager.cpp
// ============================================================================
#include "Editor/Commands/ShortcutManager.h"
#include "Editor/Commands/EditorCommandSystem.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <sstream>

namespace editor {

    ShortcutManager::ShortcutManager(EditorCommandSystem& commands)
        : m_Commands(commands)
    {
    }

    void ShortcutManager::Bind(const std::string& commandName, u32 key,
                                KeyModifiers mods, const std::string& displayName)
    {
        // Remove existing binding for this command.
        Unbind(commandName);

        Shortcut s;
        s.CommandName = commandName;
        s.Key = key;
        s.Mods = mods;
        s.DisplayName = displayName.empty() ? FormatShortcut(key, mods) : displayName;
        m_Shortcuts.push_back(s);
    }

    void ShortcutManager::Unbind(const std::string& commandName)
    {
        m_Shortcuts.erase(
            std::remove_if(m_Shortcuts.begin(), m_Shortcuts.end(),
                [&](const Shortcut& s) { return s.CommandName == commandName; }),
            m_Shortcuts.end());
    }

    bool ShortcutManager::ProcessKey(u32 key, KeyModifiers mods)
    {
        for (const auto& s : m_Shortcuts)
        {
            if (s.Matches(key, mods))
            {
                m_Commands.Execute(s.CommandName);
                return true;
            }
        }
        return false;
    }

    const Shortcut* ShortcutManager::FindByCommand(const std::string& commandName) const
    {
        for (const auto& s : m_Shortcuts)
        {
            if (s.CommandName == commandName)
                return &s;
        }
        return nullptr;
    }

    void ShortcutManager::Rebind(const std::string& commandName, u32 newKey, KeyModifiers newMods)
    {
        for (auto& s : m_Shortcuts)
        {
            if (s.CommandName == commandName)
            {
                s.Key = newKey;
                s.Mods = newMods;
                s.DisplayName = FormatShortcut(newKey, newMods);
                return;
            }
        }
        // Not found — create a new binding.
        Bind(commandName, newKey, newMods);
    }

    std::string ShortcutManager::FormatShortcut(u32 key, KeyModifiers mods)
    {
        std::ostringstream ss;

        if ((mods & KeyModifiers::Ctrl) != 0)
            ss << "Ctrl+";
        if ((mods & KeyModifiers::Shift) != 0)
            ss << "Shift+";
        if ((mods & KeyModifiers::Alt) != 0)
            ss << "Alt+";
        if ((mods & KeyModifiers::Super) != 0)
            ss << "Super+";

        // Map common GLFW keys to display names.
        switch (key)
        {
            case GLFW_KEY_S: ss << "S"; break;
            case GLFW_KEY_O: ss << "O"; break;
            case GLFW_KEY_N: ss << "N"; break;
            case GLFW_KEY_F: ss << "F"; break;
            case GLFW_KEY_W: ss << "W"; break;
            case GLFW_KEY_E: ss << "E"; break;
            case GLFW_KEY_R: ss << "R"; break;
            case GLFW_KEY_T: ss << "T"; break;
            case GLFW_KEY_SPACE: ss << "Space"; break;
            case GLFW_KEY_DELETE: ss << "Delete"; break;
            case GLFW_KEY_ENTER: ss << "Enter"; break;
            case GLFW_KEY_ESCAPE: ss << "Esc"; break;
            case GLFW_KEY_TAB: ss << "Tab"; break;
            case GLFW_KEY_F5: ss << "F5"; break;
            default:
                if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
                    ss << static_cast<char>('A' + (key - GLFW_KEY_A));
                else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
                    ss << static_cast<char>('0' + (key - GLFW_KEY_0));
                else
                    ss << "Key" << key;
                break;
        }

        return ss.str();
    }

} // namespace editor
