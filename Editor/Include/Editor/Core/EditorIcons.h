// ============================================================================
// File: Editor/Include/Editor/Core/EditorIcons.h
// Editor icon framework — supports icon packs.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include <string>
#include <unordered_map>

namespace editor {

    // ========================================================================
    // EditorIcons — manages editor icons for assets and entities.
    // ========================================================================

    /// @brief Provides icons for the editor UI.  Icons are identified
    ///        by name and can be swapped via icon packs.  Actual
    ///        texture loading is deferred to the rendering systems phase.
    class EditorIcons
    {
    public:
        EditorIcons();
        ~EditorIcons() = default;

        /// @brief Returns the icon name for an asset type.
        [[nodiscard]] std::string GetIconForAssetType(const std::string& assetType) const;

        /// @brief Returns the icon name for an entity type.
        [[nodiscard]] std::string GetIconForEntity(const std::string& tag) const;

        /// @brief Registers an icon for a type.
        void RegisterIcon(const std::string& typeName, const std::string& iconName);

        /// @brief Returns all registered icon mappings.
        [[nodiscard]] const std::unordered_map<std::string, std::string>& GetAllIcons() const noexcept
        { return m_Icons; }

    private:
        void RegisterDefaults();

        std::unordered_map<std::string, std::string> m_Icons;
    };

} // namespace editor
