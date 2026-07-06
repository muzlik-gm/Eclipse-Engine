// ============================================================================
// File: Editor/Source/Core/DragDrop.cpp
// ============================================================================
#include "Editor/Core/DragDrop.h"

#include <imgui.h>
#include <cstring>
#include <string>

namespace editor {

    // Static storage for the last accepted payload.
    static DragDropPayload s_AcceptedPayload;
    static bool s_HasAccepted = false;

    void DragDrop::SetPayload(const std::string& type, const std::string& data)
    {
        DragDropPayload payload;
        payload.Type = type;
        payload.Data = data;

        ImGui::SetDragDropPayload(kPayloadType, &payload, sizeof(payload));
    }

    const DragDropPayload* DragDrop::AcceptPayload(const std::string& type)
    {
        auto* imguiPayload = ImGui::AcceptDragDropPayload(kPayloadType);
        if (!imguiPayload)
            return nullptr;

        if (imguiPayload->DataSize < sizeof(DragDropPayload))
            return nullptr;

        auto* dp = static_cast<const DragDropPayload*>(imguiPayload->Data);
        if (dp->Type != type)
            return nullptr;

        s_AcceptedPayload = *dp;
        s_HasAccepted = true;
        return &s_AcceptedPayload;
    }

    bool DragDrop::IsDragging(const std::string& type)
    {
        auto* imguiPayload = ImGui::GetDragDropPayload();
        if (!imguiPayload)
            return false;
        if (imguiPayload->DataSize < sizeof(DragDropPayload))
            return false;
        auto* dp = static_cast<const DragDropPayload*>(imguiPayload->Data);
        return dp->Type == type;
    }

} // namespace editor
