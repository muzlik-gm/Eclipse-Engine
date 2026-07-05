// ============================================================================
// File: Engine/Include/Engine/ECS/Entity.h
// Entity identifier — type alias for entt::entity.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

#include <entt/entt.hpp>

namespace engine::ecs {

    using engine::core::u32;

    // ========================================================================
    // Entity — type alias for the underlying entt::entity identifier.
    // ========================================================================

    /// @brief Entity identifier, backed by entt::entity.
    ///
    /// Use Entity::Invalid (alias for entt::null) to represent a null entity.
    /// Use ecs::Registry methods for creation, destruction, and component
    /// management rather than manipulating raw entt handles directly.
    using Entity = entt::entity;

    /// @brief Sentinel value representing an invalid / null entity.
    inline constexpr Entity Invalid = entt::null;

    // ========================================================================
    // Free functions for common entity operations.
    // ========================================================================

    /// @brief Returns true if the entity handle is not null.
    [[nodiscard]] inline bool IsValid(Entity entity) noexcept
    {
        return entity != entt::null;
    }

    /// @brief Returns the underlying integral index of the entity.
    [[nodiscard]] inline u32 GetIndex(Entity entity) noexcept
    {
        return static_cast<u32>(entt::to_integral(entity));
    }

} // namespace engine::ecs