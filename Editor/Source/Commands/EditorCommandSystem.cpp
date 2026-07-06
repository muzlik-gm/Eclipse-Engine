// ============================================================================
// File: Editor/Source/Commands/EditorCommandSystem.cpp
// ============================================================================
#include "Editor/Commands/EditorCommandSystem.h"
#include "Engine/Core/Log.h"

namespace editor {

    EditorCommandSystem::EditorCommandSystem() = default;

    CommandId EditorCommandSystem::Register(const Command& command)
    {
        if (command.Name.empty())
        {
            ENGINE_LOG_ERROR("EditorCommandSystem — cannot register command with empty name");
            return 0;
        }

        if (m_Commands.count(command.Name) > 0)
        {
            ENGINE_LOG_WARN("EditorCommandSystem — command '{}' already registered", command.Name);
            return 0;
        }

        m_Commands[command.Name] = command;
        return m_NextId++;
    }

    void EditorCommandSystem::Unregister(const std::string& name)
    {
        m_Commands.erase(name);
    }

    bool EditorCommandSystem::Execute(const std::string& name)
    {
        auto it = m_Commands.find(name);
        if (it == m_Commands.end())
        {
            ENGINE_LOG_WARN("EditorCommandSystem — command '{}' not found", name);
            return false;
        }

        if (it->second.CanExecute && !it->second.CanExecute())
            return false;

        if (it->second.Execute)
        {
            it->second.Execute();
            return true;
        }
        return false;
    }

    bool EditorCommandSystem::CanExecute(const std::string& name) const
    {
        auto it = m_Commands.find(name);
        if (it == m_Commands.end())
            return false;
        if (it->second.CanExecute)
            return it->second.CanExecute();
        return true;
    }

    const Command* EditorCommandSystem::Find(const std::string& name) const
    {
        auto it = m_Commands.find(name);
        return (it != m_Commands.end()) ? &it->second : nullptr;
    }

    std::vector<const Command*> EditorCommandSystem::FindByCategory(
        const std::string& category) const
    {
        std::vector<const Command*> result;
        for (const auto& [_, cmd] : m_Commands)
        {
            if (cmd.Category == category)
                result.push_back(&cmd);
        }
        return result;
    }

    std::vector<const Command*> EditorCommandSystem::GetAll() const
    {
        std::vector<const Command*> result;
        result.reserve(m_Commands.size());
        for (const auto& [_, cmd] : m_Commands)
            result.push_back(&cmd);
        return result;
    }

    u32 EditorCommandSystem::GetCount() const noexcept
    {
        return static_cast<u32>(m_Commands.size());
    }

    void EditorCommandSystem::SetChecked(const std::string& name, bool checked)
    {
        auto it = m_Commands.find(name);
        if (it != m_Commands.end())
            it->second.Checked = checked;
    }

} // namespace editor
