// ============================================================================
// File: Editor/Include/Editor/Commands/ShortcutManager.h
// Configurable keyboard shortcut system.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace editor {

    using engine::core::u32;

    class EditorCommandSystem;

    // ========================================================================
    // KeyModifiers — bitmask of keyboard modifiers.
    // ========================================================================

    enum class KeyModifiers : engine::core::u32
    {
        None    = 0,
        Ctrl    = (1u << 0),
        Shift   = (1u << 1),
        Alt     = (1u << 2),
        Super   = (1u << 3)
    };

    [[nodiscard]] constexpr KeyModifiers operator|(KeyModifiers a, KeyModifiers b) noexcept
    {
        return static_cast<KeyModifiers>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    [[nodiscard]] constexpr engine::core::u32 operator&(KeyModifiers a, KeyModifiers b) noexcept
    {
        return static_cast<u32>(a) & static_cast<u32>(b);
    }

    // ========================================================================
    // Shortcut — a key + modifier combination bound to a command.
    // ========================================================================

    struct Shortcut
    {
        std::string   CommandName;   // e.g. "file.save"
        engine::core::u32 Key;       // GLFW key code
        KeyModifiers  Mods{KeyModifiers::None};
        std::string   DisplayName;   // e.g. "Ctrl+S"

        [[nodiscard]] bool Matches(engine::core::u32 key, KeyModifiers mods) const noexcept
        {
            return Key == key && static_cast<engine::core::u32>(Mods) == static_cast<engine::core::u32>(mods);
        }
    };

    // ========================================================================
    // ShortcutManager — manages keyboard shortcuts.
    // ========================================================================

    /// @brief Manages keyboard shortcuts bound to commands.  Shortcuts
    ///        are configurable and can be rebound at runtime.
    class ShortcutManager
    {
    public:
        explicit ShortcutManager(EditorCommandSystem& commands);
        ~ShortcutManager() = default;

        /// @brief Binds a shortcut to a command.
        void Bind(const std::string& commandName,
                  engine::core::u32 key,
                  KeyModifiers mods = KeyModifiers::None,
                  const std::string& displayName = "");

        /// @brief Unbinds the shortcut for @p commandName.
        void Unbind(const std::string& commandName);

        /// @brief Processes a key press.  If a matching shortcut is
        ///        found, the bound command is executed.
        /// @return True if a shortcut was matched.
        bool ProcessKey(engine::core::u32 key, KeyModifiers mods);

        /// @brief Returns all registered shortcuts.
        [[nodiscard]] const std::vector<Shortcut>& GetAllShortcuts() const noexcept
        { return m_Shortcuts; }

        /// @brief Returns the shortcut bound to @p commandName, or nullptr.
        [[nodiscard]] const Shortcut* FindByCommand(const std::string& commandName) const;

        /// @brief Rebinds a command to a new key+modifier combination.
        void Rebind(const std::string& commandName,
                    engine::core::u32 newKey,
                    KeyModifiers newMods);

        /// @brief Formats a shortcut as a display string (e.g. "Ctrl+Shift+S").
        [[nodiscard]] static std::string FormatShortcut(engine::core::u32 key, KeyModifiers mods);

    private:
        EditorCommandSystem&    m_Commands;
        std::vector<Shortcut>   m_Shortcuts;
    };

} // namespace editor
