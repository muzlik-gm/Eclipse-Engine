// ============================================================================
// File: Editor/Include/Editor/Commands/EditorCommandSystem.h
// Centralized command architecture for menus, toolbar, and shortcuts.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace editor {

    using engine::core::u32;

    // ========================================================================
    // CommandId — unique identifier for a command.
    // ========================================================================

    /// @brief Opaque command identifier.  Commands are registered with
    ///        a unique string name (e.g. "file.save") and invoked by
    ///        that name from menus, toolbar, or shortcuts.
    using CommandId = u32;

    // ========================================================================
    // Command — a single registered command.
    // ========================================================================

    struct Command
    {
        std::string             Name;        // e.g. "file.save"
        std::string             DisplayName; // e.g. "Save"
        std::string             Category;    // e.g. "File"
        std::string             Description;
        std::function<void()>   Execute;
        std::function<bool()>   CanExecute;  // returns false to disable
        bool                    Checked{false};
    };

    // ========================================================================
    // EditorCommandSystem — registers and dispatches commands.
    // ========================================================================

    /// @brief Central command registry.  Menus, toolbar buttons, and
    ///        keyboard shortcuts all invoke commands by name through
    ///        this system rather than using direct callbacks.
    class EditorCommandSystem
    {
    public:
        EditorCommandSystem();
        ~EditorCommandSystem() = default;

        /// @brief Registers a command.  Returns its ID, or 0 on failure.
        CommandId Register(const Command& command);

        /// @brief Unregisters a command by name.
        void Unregister(const std::string& name);

        /// @brief Executes the command with @p name.
        /// @return True if the command was found and executed.
        bool Execute(const std::string& name);

        /// @brief Returns true if @p name is registered and can execute.
        [[nodiscard]] bool CanExecute(const std::string& name) const;

        /// @brief Returns the command with @p name, or nullptr.
        [[nodiscard]] const Command* Find(const std::string& name) const;

        /// @brief Returns all commands in @p category.
        [[nodiscard]] std::vector<const Command*> FindByCategory(
            const std::string& category) const;

        /// @brief Returns all registered commands.
        [[nodiscard]] std::vector<const Command*> GetAll() const;

        /// @brief Returns the number of registered commands.
        [[nodiscard]] u32 GetCount() const noexcept;

        /// @brief Sets the checked state of a command (for toggle items).
        void SetChecked(const std::string& name, bool checked);

    private:
        std::unordered_map<std::string, Command> m_Commands;
        CommandId                                m_NextId{1};
    };

} // namespace editor
