// ============================================================================
// File: Engine/Include/Engine/Components/AudioSource.h
// Runtime representation of an audio source.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;

    /// @brief Attaches audio playback parameters to an entity.
    struct AudioSource
    {
        std::string ClipPath;
        f32  Volume{1.0f};
        f32  Pitch{1.0f};
        bool Loop{false};
        bool PlayOnAwake{false};
        bool Spatial{false};
        f32  MinDistance{1.0f};
        f32  MaxDistance{50.0f};
        f32  RolloffFactor{1.0f};
    };

} // namespace engine::components
