// ============================================================================
// File: Editor/Include/Editor/History/CommandHistory.h
// Undo/Redo command history architecture.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace editor {

    // ========================================================================
    // IUndoCommand — interface for an undoable command.
    // ========================================================================

    /// @brief Interface for a command that can be undone and redone.
    ///        Concrete commands (RenameEntity, CreateEntity, TransformChange,
    ///        etc.) implement this interface and are pushed onto the
    ///        CommandHistory.
    class IUndoCommand
    {
    public:
        virtual ~IUndoCommand() = default;

        /// @brief Returns a human-readable description (e.g. "Rename Entity").
        [[nodiscard]] virtual std::string GetDescription() const = 0;

        /// @brief Executes the command (called on first push).
        virtual void Execute() = 0;

        /// @brief Undoes the command.
        virtual void Undo() = 0;

        /// @brief Redoes the command (re-executes after undo).
        virtual void Redo() { Execute(); }
    };

    // ========================================================================
    // LambdaUndoCommand — convenience implementation using lambdas.
    // ========================================================================

    /// @brief Convenience IUndoCommand implementation that takes execute
    ///        and undo lambdas.
    class LambdaUndoCommand final : public IUndoCommand
    {
    public:
        LambdaUndoCommand(std::string description,
                          std::function<void()> execute,
                          std::function<void()> undo)
            : m_Description(std::move(description))
            , m_Execute(std::move(execute))
            , m_Undo(std::move(undo))
        {}

        [[nodiscard]] std::string GetDescription() const override { return m_Description; }
        void Execute() override { if (m_Execute) m_Execute(); }
        void Undo() override { if (m_Undo) m_Undo(); }

    private:
        std::string         m_Description;
        std::function<void()> m_Execute;
        std::function<void()> m_Undo;
    };

    // ========================================================================
    // CommandHistory — manages the undo/redo stack.
    // ========================================================================

    /// @brief Manages a stack of undoable commands.  Supports undo,
    ///        redo, and limiting the stack depth.
    class CommandHistory
    {
    public:
        CommandHistory();
        ~CommandHistory() = default;

        /// @brief Pushes a command onto the undo stack and executes it.
        void Execute(std::unique_ptr<IUndoCommand> command);

        /// @brief Undoes the last command.
        void Undo();

        /// @brief Redoes the last undone command.
        void Redo();

        /// @brief Returns true if there are commands to undo.
        [[nodiscard]] bool CanUndo() const noexcept { return !m_UndoStack.empty(); }

        /// @brief Returns true if there are commands to redo.
        [[nodiscard]] bool CanRedo() const noexcept { return !m_RedoStack.empty(); }

        /// @brief Returns the description of the next undo command.
        [[nodiscard]] std::string GetUndoDescription() const;

        /// @brief Returns the description of the next redo command.
        [[nodiscard]] std::string GetRedoDescription() const;

        /// @brief Clears the entire history.
        void Clear();

        /// @brief Sets the maximum stack depth (default 100).
        void SetMaxDepth(engine::core::u32 depth) noexcept { m_MaxDepth = depth; }

        /// @brief Returns the number of commands on the undo stack.
        [[nodiscard]] engine::core::u32 GetUndoCount() const noexcept
        { return static_cast<engine::core::u32>(m_UndoStack.size()); }

        /// @brief Returns the number of commands on the redo stack.
        [[nodiscard]] engine::core::u32 GetRedoCount() const noexcept
        { return static_cast<engine::core::u32>(m_RedoStack.size()); }

    private:
        std::vector<std::unique_ptr<IUndoCommand>> m_UndoStack;
        std::vector<std::unique_ptr<IUndoCommand>> m_RedoStack;
        engine::core::u32                          m_MaxDepth{100};
    };

} // namespace editor
