// ============================================================================
// File: Editor/Source/Core/EditorIcons.cpp
// ============================================================================
#include "Editor/Core/EditorIcons.h"

namespace editor {

    EditorIcons::EditorIcons()
    {
        RegisterDefaults();
    }

    void EditorIcons::RegisterDefaults()
    {
        // Entity icons
        m_Icons["entity"]        = "entity";
        m_Icons["entity_camera"] = "camera";
        m_Icons["entity_light"]  = "light";
        m_Icons["entity_mesh"]   = "mesh";

        // Asset icons
        m_Icons["texture"]  = "texture";
        m_Icons["shader"]   = "shader";
        m_Icons["material"] = "material";
        m_Icons["mesh"]     = "mesh";
        m_Icons["scene"]    = "scene";
        m_Icons["prefab"]   = "prefab";
        m_Icons["audio"]    = "audio";
        m_Icons["font"]     = "font";
        m_Icons["script"]   = "script";
        m_Icons["folder"]   = "folder";
        m_Icons["file"]     = "file";
    }

    std::string EditorIcons::GetIconForAssetType(const std::string& assetType) const
    {
        auto it = m_Icons.find(assetType);
        return (it != m_Icons.end()) ? it->second : "file";
    }

    std::string EditorIcons::GetIconForEntity(const std::string& tag) const
    {
        auto it = m_Icons.find("entity_" + tag);
        if (it != m_Icons.end())
            return it->second;

        if (tag.find("Camera") != std::string::npos)
            return "camera";
        if (tag.find("Light") != std::string::npos)
            return "light";
        if (tag.find("Mesh") != std::string::npos)
            return "mesh";

        return "entity";
    }

    void EditorIcons::RegisterIcon(const std::string& typeName, const std::string& iconName)
    {
        m_Icons[typeName] = iconName;
    }

} // namespace editor
