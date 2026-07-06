// ============================================================================
// File: Engine/Include/Engine/Components/IDComponent.h
// Unique identifier attached to every entity.
// ============================================================================

#pragma once

#include "Engine/Core/UUID.h"

namespace engine::components
{

    // ========================================================================
    // IDComponent
    // ========================================================================

    /// Assigns a globally unique 128-bit identifier to an entity.
    ///
    /// Every entity in the ECS world carries an IDComponent so that it can
    /// be unambiguously referenced across serialization boundaries, network
    /// replication, and editor panels.
    struct IDComponent
    {
        /// Randomly generated UUID (version 4) for this entity.
        core::UUID ID{};
    };

} // namespace engine::components