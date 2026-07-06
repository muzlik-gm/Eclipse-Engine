// ============================================================================
// File: Editor/Include/Editor/Core/DragDrop.h
// Drag-and-drop architecture using ImGui payload system.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>
#include <string_view>

namespace editor {

    // ========================================================================
    // DragDropPayload — data attached to a drag operation.
    // ========================================================================

    /// @brief Identifies what type of object is being dragged and its
    ///        associated data (e.g. file path, entity ID, asset UUID).
    struct DragDropPayload
    {
        /// Type of the dragged object.
        ///   "asset"       — a file system asset (path in Data field)
        ///   "entity"      — an ECS entity (entity ID in Data field as string)
        ///   "scene"       — a scene file (path in Data field)
        ///   "folder"      — a folder (path in Data field)
        ///   "material"    — a material asset (path in Data field)
        ///   "mesh"        — a mesh asset (path in Data field)
        ///   "prefab"      — a prefab file (path in Data field)
        std::string Type;

        /// Data payload (e.g. file path, entity ID, UUID).
        std::string Data;
    };

    // ========================================================================
    // DragDrop — static helpers for drag-and-drop operations.
    // ========================================================================

    /// @brief Static helpers for ImGui-based drag-and-drop.  All drag
    ///        operations use ImGui's payload system with a custom
    ///        DragDropPayload struct.
    class DragDrop
    {
    public:
        /// @brief Begins a drag source.  Call between BeginDragDropSource()
        ///        and EndDragDropSource() (handled internally).
        /// @param type   Payload type (e.g. "asset", "entity").
        /// @param data   Payload data (e.g. file path).
        static void SetPayload(const std::string& type, const std::string& data);

        /// @brief Accepts a drag payload of the given type.
        /// @param type Expected payload type.
        /// @return The payload if accepted, or nullptr.
        [[nodiscard]] static const DragDropPayload* AcceptPayload(const std::string& type);

        /// @brief Returns true if a drag of @p type is currently in progress.
        [[nodiscard]] static bool IsDragging(const std::string& type);

        /// @brief The ImGui payload type identifier string.
        static constexpr const char* kPayloadType = "EclipseDragDrop";
    };

} // namespace editor
