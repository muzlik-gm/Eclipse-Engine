// ============================================================================
// File: Editor/Source/History/CommandHistory.cpp
// ============================================================================
#include "Editor/History/CommandHistory.h"
#include "Engine/Core/Log.h"

namespace editor {

    CommandHistory::CommandHistory() = default;

    void CommandHistory::Execute(std::unique_ptr<IUndoCommand> command)
    {
        if (!command)
            return;

        command->Execute();
        m_UndoStack.push_back(std::move(command));

        // Clear the redo stack — new action invalidates redo history.
        m_RedoStack.clear();

        // Trim if over max depth.
        while (m_UndoStack.size() > m_MaxDepth)
            m_UndoStack.erase(m_UndoStack.begin());
    }

    void CommandHistory::Undo()
    {
        if (m_UndoStack.empty())
            return;

        auto& cmd = m_UndoStack.back();
        ENGINE_LOG_DEBUG("CommandHistory — undo: {}", cmd->GetDescription());
        cmd->Undo();
        m_RedoStack.push_back(std::move(cmd));
        m_UndoStack.pop_back();
    }

    void CommandHistory::Redo()
    {
        if (m_RedoStack.empty())
            return;

        auto& cmd = m_RedoStack.back();
        ENGINE_LOG_DEBUG("CommandHistory — redo: {}", cmd->GetDescription());
        cmd->Redo();
        m_UndoStack.push_back(std::move(cmd));
        m_RedoStack.pop_back();
    }

    std::string CommandHistory::GetUndoDescription() const
    {
        if (m_UndoStack.empty())
            return "";
        return m_UndoStack.back()->GetDescription();
    }

    std::string CommandHistory::GetRedoDescription() const
    {
        if (m_RedoStack.empty())
            return "";
        return m_RedoStack.back()->GetDescription();
    }

    void CommandHistory::Clear()
    {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }

} // namespace editor
