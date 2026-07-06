// ============================================================================
// File: Editor/Source/Selection/EditorSelection.cpp
// ============================================================================
#include "Editor/Selection/EditorSelection.h"

namespace editor {

    using engine::core::u32;

    using engine::ecs::Entity;

    void EditorSelection::SelectEntity(Entity entity)
    {
        m_SelectedEntities.clear();
        if (entity != engine::ecs::Invalid)
            m_SelectedEntities.push_back(entity);
        NotifyChanged();
    }

    void EditorSelection::AddEntityToSelection(Entity entity)
    {
        if (entity == engine::ecs::Invalid)
            return;
        if (IsSelected(entity))
            return;
        m_SelectedEntities.push_back(entity);
        NotifyChanged();
    }

    void EditorSelection::DeselectEntity(Entity entity)
    {
        m_SelectedEntities.erase(
            std::remove(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity),
            m_SelectedEntities.end());
        NotifyChanged();
    }

    void EditorSelection::Clear()
    {
        if (!m_SelectedEntities.empty())
        {
            m_SelectedEntities.clear();
            NotifyChanged();
        }
    }

    Entity EditorSelection::GetPrimaryEntity() const noexcept
    {
        if (m_SelectedEntities.empty())
            return engine::ecs::Invalid;
        return m_SelectedEntities.front();
    }

    bool EditorSelection::IsSelected(Entity entity) const noexcept
    {
        for (auto e : m_SelectedEntities)
        {
            if (e == entity)
                return true;
        }
        return false;
    }

    engine::core::u32 EditorSelection::Subscribe(SelectionChangedCallback callback)
    {
        u32 id = m_NextId++;
        m_Callbacks[id] = std::move(callback);
        return id;
    }

    void EditorSelection::Unsubscribe(engine::core::u32 subscriptionId)
    {
        m_Callbacks.erase(subscriptionId);
    }

    void EditorSelection::NotifyChanged()
    {
        for (const auto& [_, cb] : m_Callbacks)
        {
            if (cb)
                cb(*this);
        }
    }

} // namespace editor
