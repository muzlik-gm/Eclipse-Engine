// ============================================================================
// File: Engine/Include/Engine/Assets/Core/AssetTypes.h
// Core asset type definitions — UUIDs, handles, types, states.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/UUID.h"

#include <atomic>
#include <compare>
#include <functional>
#include <string>
#include <string_view>

namespace engine::assets {

    using engine::core::u32;
    using engine::core::u64;
    using engine::core::usize;

    // ========================================================================
    // AssetUUID — stable identifier for an asset.
    // ========================================================================

    /// @brief A stable UUID that uniquely identifies an asset across
    ///        sessions, platforms, and network replication.
    using AssetUUID = core::UUID;

    // ========================================================================
    // AssetType — enumerates the built-in asset types.
    // ========================================================================

    /// @brief Built-in asset type identifiers.  Custom asset types use
    ///        AssetType::Custom with a type name registered through
    ///        AssetTypeRegistry.
    enum class AssetType : u32
    {
        Unknown    = 0,
        Texture    = 1,
        Shader     = 2,
        Material   = 3,
        Mesh       = 4,
        Scene      = 5,
        Prefab     = 6,
        Audio      = 7,
        Animation  = 8,
        Font       = 9,
        Script     = 10,
        Binary     = 11,
        Text       = 12,
        Config     = 13,
        Custom     = 0xFFFFFFFF
    };

    [[nodiscard]] constexpr const char* AssetTypeToString(AssetType type) noexcept
    {
        switch (type)
        {
            case AssetType::Texture:   return "Texture";
            case AssetType::Shader:    return "Shader";
            case AssetType::Material:  return "Material";
            case AssetType::Mesh:      return "Mesh";
            case AssetType::Scene:     return "Scene";
            case AssetType::Prefab:    return "Prefab";
            case AssetType::Audio:     return "Audio";
            case AssetType::Animation: return "Animation";
            case AssetType::Font:      return "Font";
            case AssetType::Script:    return "Script";
            case AssetType::Binary:    return "Binary";
            case AssetType::Text:      return "Text";
            case AssetType::Config:    return "Config";
            case AssetType::Custom:    return "Custom";
            case AssetType::Unknown:   break;
        }
        return "Unknown";
    }

    // ========================================================================
    // AssetState — lifecycle state of an asset.
    // ========================================================================

    enum class AssetState : u32
    {
        Unknown      = 0,
        Registered   = 1,  // metadata exists, data not loaded
        Importing    = 2,  // importer is running
        Imported     = 3,  // imported but not loaded into GPU/runtime
        Loading      = 4,  // runtime data is being loaded
        Loaded       = 5,  // fully loaded and ready
        Unloading    = 6,  // being unloaded
        Unloaded     = 7,  // runtime data released, metadata kept
        Error        = 8,  // load/import failed
        Missing      = 9   // file not found / deleted
    };

    [[nodiscard]] constexpr const char* AssetStateToString(AssetState state) noexcept
    {
        switch (state)
        {
            case AssetState::Registered: return "Registered";
            case AssetState::Importing:  return "Importing";
            case AssetState::Imported:   return "Imported";
            case AssetState::Loading:    return "Loading";
            case AssetState::Loaded:     return "Loaded";
            case AssetState::Unloading:  return "Unloading";
            case AssetState::Unloaded:   return "Unloaded";
            case AssetState::Error:      return "Error";
            case AssetState::Missing:    return "Missing";
            case AssetState::Unknown:    break;
        }
        return "Unknown";
    }

    // ========================================================================
    // AssetHandle — opaque handle to an asset.
    // ========================================================================

    /// @brief An opaque, lightweight handle to an asset.  Handles are
    ///        cheap to copy and compare.  They do not own the asset —
    ///        use AssetRef (strong) or AssetWeakRef (weak) for ownership.
    struct AssetHandle
    {
        AssetUUID UUID{};
        u32       Generation{0};  // incremented on reload

        [[nodiscard]] bool IsValid() const noexcept { return UUID.IsValid(); }
        [[nodiscard]] bool operator==(const AssetHandle&) const noexcept = default;
        [[nodiscard]] bool operator!=(const AssetHandle&) const noexcept = default;

        [[nodiscard]] usize Hash() const noexcept
        {
            return UUID.Hash() ^ (static_cast<usize>(Generation) << 32);
        }
    };

    // ========================================================================
    // AssetPath — virtual path to an asset.
    // ========================================================================

    /// @brief A virtual path that identifies an asset within the project.
    ///        Example: "assets://textures/brick.png"
    ///        The scheme prefix determines the root:
    ///          assets://  — project assets directory
    ///          engine://  — engine built-in assets
    ///          package:// — packaged asset bundle
    struct AssetPath
    {
        std::string Path;

        AssetPath() = default;
        explicit AssetPath(std::string path) : Path(std::move(path)) {}
        AssetPath(std::string_view scheme, std::string_view relative)
            : Path(std::string(scheme) + "://" + std::string(relative)) {}

        [[nodiscard]] bool IsValid() const noexcept { return !Path.empty(); }

        [[nodiscard]] std::string GetScheme() const
        {
            auto pos = Path.find("://");
            return (pos != std::string::npos) ? Path.substr(0, pos) : "";
        }

        [[nodiscard]] std::string GetRelativePath() const
        {
            auto pos = Path.find("://");
            return (pos != std::string::npos) ? Path.substr(pos + 3) : Path;
        }

        [[nodiscard]] std::string GetExtension() const
        {
            auto rel = GetRelativePath();
            auto pos = rel.find_last_of('.');
            return (pos != std::string::npos) ? rel.substr(pos) : "";
        }

        [[nodiscard]] std::string GetFileName() const
        {
            auto rel = GetRelativePath();
            auto pos = rel.find_last_of('/');
            return (pos != std::string::npos) ? rel.substr(pos + 1) : rel;
        }

        [[nodiscard]] std::string GetFileNameWithoutExtension() const
        {
            auto name = GetFileName();
            auto pos = name.find_last_of('.');
            return (pos != std::string::npos) ? name.substr(0, pos) : name;
        }

        bool operator==(const AssetPath&) const = default;
    };

} // namespace engine::assets

// ===========================================================================
// std::hash specializations for use in unordered_map / unordered_set
// ===========================================================================
namespace std {

    template <>
    struct hash<engine::assets::AssetHandle>
    {
        [[nodiscard]] std::size_t operator()(const engine::assets::AssetHandle& h) const noexcept
        {
            return h.Hash();
        }
    };

    template <>
    struct hash<engine::assets::AssetPath>
    {
        [[nodiscard]] std::size_t operator()(const engine::assets::AssetPath& p) const noexcept
        {
            return std::hash<std::string>{}(p.Path);
        }
    };

} // namespace std
