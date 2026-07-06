// ============================================================================
// File: Editor/Include/Editor/Selection/EditorSelection.h
// Global editor selection system.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Core/UUID.h"

#include <functional>
#include <unordered_set>
#include <vector>

namespace editor {

    // ========================================================================
    // SelectionType — what kind of object is selected.
    // ========================================================================

    enum class SelectionType : engine::core::u32
    {
        None    = 0,
        Entity  = 1,
        Asset   = 2
    };

    // ========================================================================
    // SelectionEntry — a single selected item.
    // ========================================================================

    struct SelectionEntry
    {
        SelectionType         Type{SelectionType::None};
        engine::ecs::Entity   Entity{engine::ecs::Invalid};
        engine::core::UUID    AssetUUID{};
    };

    // ========================================================================
    // EditorSelection — global selection manager.
    // ========================================================================

    /// @brief The single source of truth for what is selected in the
    ///        editor.  Panels (Hierarchy, Inspector, Content Browser)
    ///        all read from and write to this manager.
    class EditorSelection
    {
    public:
        EditorSelection() = default;
        ~EditorSelection() = default;

        /// @brief Selects a single entity, replacing the current selection.
        void SelectEntity(engine::ecs::Entity entity);

        /// @brief Adds an entity to the selection (multi-select).
        void AddEntityToSelection(engine::ecs::Entity entity);

        /// @brief Deselects a specific entity.
        void DeselectEntity(engine::ecs::Entity entity);

        /// @brief Clears the entire selection.
        void Clear();

        /// @brief Returns the primary (first) selected entity, or Invalid.
        [[nodiscard]] engine::ecs::Entity GetPrimaryEntity() const noexcept;

        /// @brief Returns all selected entities.
        [[nodiscard]] const std::vector<engine::ecs::Entity>& GetSelectedEntities() const noexcept
        { return m_SelectedEntities; }

        /// @brief Returns true if @p entity is in the selection.
        [[nodiscard]] bool IsSelected(engine::ecs::Entity entity) const noexcept;

        /// @brief Returns the number of selected items.
        [[nodiscard]] engine::core::u32 GetCount() const noexcept
        { return static_cast<engine::core::u32>(m_SelectedEntities.size()); }

        /// @brief Returns true if nothing is selected.
        [[nodiscard]] bool IsEmpty() const noexcept { return m_SelectedEntities.empty(); }

        // -- Callbacks -----------------------------------------------------

        using SelectionChangedCallback = std::function<void(const EditorSelection&)>;

        /// @brief Registers a callback invoked whenever the selection changes.
        /// @return An opaque subscription id.
        engine::core::u32 Subscribe(SelectionChangedCallback callback);

        /// @brief Removes a registered callback.
        void Unsubscribe(engine::core::u32 subscriptionId);

    private:
        void NotifyChanged();

        std::vector<engine::ecs::Entity>              m_SelectedEntities;
        std::unordered_set<engine::core::u32>          m_SubscriptionIds;
        std::unordered_map<engine::core::u32,
            SelectionChangedCallback>                  m_Callbacks;
        engine::core::u32                              m_NextId{1};
    };

} // namespace editor
