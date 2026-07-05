// ============================================================================
// File: Engine/Include/Engine/Components/TagComponent.h
// ============================================================================
#pragma once

// Human-readable entity tag/name.

#include <string>

namespace engine::components {

    // ========================================================================
    // TagComponent — optional human-readable label attached to an entity.
    // ========================================================================

    /// @brief Data-only component that stores a human-readable tag / name.
    ///
    /// Attach a TagComponent to any entity that should carry a display name
    /// (e.g. in an editor hierarchy panel or debug output).
    struct TagComponent
    {
        std::string Tag;
    };

} // namespace engine::components