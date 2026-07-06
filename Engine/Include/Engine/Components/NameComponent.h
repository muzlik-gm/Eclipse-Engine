// ============================================================================
// File: Engine/Include/Engine/Components/NameComponent.h
// Human-readable name distinct from the tag.
// ============================================================================
#pragma once

#include <string>

namespace engine::components {

    /// @brief A dedicated name component for entities.
    ///
    /// While TagComponent provides a short label, NameComponent stores
    /// a longer, user-facing display name suitable for editor panels
    /// and runtime lookups.
    struct NameComponent
    {
        std::string Name;
    };

} // namespace engine::components
