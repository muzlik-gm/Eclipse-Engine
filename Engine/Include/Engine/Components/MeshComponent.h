// ============================================================================
// File: Engine/Include/Engine/Components/MeshComponent.h
// Runtime representation of a mesh asset reference.
// ============================================================================
#pragma once

#include "Engine/Core/UUID.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::u32;
    using engine::core::f32;

    /// @brief Indicates which type of mesh proxy to render.
    enum class MeshType : u32
    {
        None     = 0,
        Custom   = 1,
        Cube     = 2,
        Sphere   = 3,
        Plane    = 4,
        Quad     = 5
    };

    /// @brief Attaches renderable mesh data to an entity.
    struct MeshComponent
    {
        MeshType  Type{MeshType::None};
        core::UUID MeshAssetID{};
        u32       SubMeshIndex{0};
        bool      CastShadows{true};
    };

} // namespace engine::components
