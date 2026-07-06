// ============================================================================
// File: Engine/Include/Engine/Assets/Reflection/IAssetReflection.h
// Reflection interface for asset types.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <string>
#include <vector>

namespace engine::assets {

    class IAsset;

    // ========================================================================
    // AssetFieldInfo — describes a field of an asset.
    // ========================================================================

    struct AssetFieldInfo
    {
        std::string  Name;
        std::string  TypeName;
        core::usize  Offset{0};
        core::usize  Size{0};
        bool         IsEditable{true};
        bool         IsArray{false};
        std::string  Description;
        std::string  Category{"Default"};
    };

    // ========================================================================
    // IAssetReflection — reflection for an asset type.
    // ========================================================================

    /// @brief Provides runtime reflection for a specific asset type.
    ///        Used by the editor (future phase) to inspect and edit
    ///        asset fields without knowing the concrete C++ type.
    class IAssetReflection
    {
    public:
        virtual ~IAssetReflection() = default;

        [[nodiscard]] virtual const std::string& GetTypeName() const noexcept = 0;
        [[nodiscard]] virtual AssetType GetAssetType() const noexcept = 0;
        [[nodiscard]] virtual const std::vector<AssetFieldInfo>& GetFields() const noexcept = 0;
        [[nodiscard]] virtual const AssetFieldInfo* FindField(const std::string& name) const = 0;
    };

} // namespace engine::assets
