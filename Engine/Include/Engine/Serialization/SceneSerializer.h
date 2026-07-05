// ============================================================================
// File: Engine/Include/Engine/Serialization/SceneSerializer.h
// JSON serialization / deserialization for Scene entities and components.
// ============================================================================
#pragma once

#include "Engine/Scene/Scene.h"

#include <nlohmann/json_fwd.hpp>
#include <memory>

namespace engine::serialization {

    // ========================================================================
    // SceneSerializer
    // ========================================================================

    /// @brief Static helper that serializes a Scene to JSON and
    ///        deserializes JSON back into a new Scene instance.
    ///
    /// Currently supports: TagComponent, IDComponent, TransformComponent,
    /// and HierarchyComponent.
    class SceneSerializer
    {
    public:
        // -- Serialization -----------------------------------------------------

        /// @brief Serializes the given scene's entities and supported
        ///        components into a JSON object.
        [[nodiscard]] static nlohmann::json Serialize(const scene::Scene& scene);

        // -- Deserialization ---------------------------------------------------

        /// @brief Deserializes a JSON object into a newly created Scene.
        ///
        /// @param data JSON object produced by Serialize().
        /// @return A unique_ptr to the newly created Scene.
        static std::unique_ptr<scene::Scene> Deserialize(const nlohmann::json& data);
    };

} // namespace engine::serialization