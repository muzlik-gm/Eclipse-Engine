// ============================================================================
// File: Engine/Include/Engine/Serialization/SceneSerializer.h
// JSON serialization / deserialization for Scene entities and components.
// ============================================================================
#pragma once

#include "Engine/Scene/Scene.h"
#include "Engine/Serialization/Interfaces/ISceneLoader.h"
#include "Engine/Serialization/Interfaces/ISceneSaver.h"

#include <nlohmann/json_fwd.hpp>
#include <memory>
#include <string>

namespace engine::serialization {

    // ========================================================================
    // SceneSerializer
    // ========================================================================

    /// @brief Serializes a Scene to JSON and deserializes JSON back into
    ///        a new Scene instance.
    ///
    /// Hierarchy references (Parent, FirstChild, NextSibling, PrevSibling)
    /// are serialized as UUID strings rather than entity indices.  EnTT
    /// recycles entity indices after destruction, so using UUIDs guarantees
    /// that hierarchy links survive a save / load round-trip.
    ///
    /// Supported components:
    ///   TagComponent, IDComponent, NameComponent,
    ///   TransformComponent, HierarchyComponent,
    ///   VisibilityComponent, EnabledComponent, StaticComponent,
    ///   MeshComponent, CameraComponent, LightComponent,
    ///   ScriptComponent, AudioSource, Collider, RigidBody.
    class SceneSerializer
    {
    public:
        // -- In-memory serialization -------------------------------------------

        /// @brief Serializes the given scene's entities and supported
        ///        components into a JSON object.
        [[nodiscard]] static nlohmann::json Serialize(const scene::Scene& scene);

        /// @brief Deserializes a JSON object into a newly created Scene.
        static std::unique_ptr<scene::Scene> Deserialize(const nlohmann::json& data);

        // -- File I/O ----------------------------------------------------------

        /// @brief Serializes the scene and writes the JSON to @p filePath.
        /// @return true on success, false on I/O error.
        static bool SaveToFile(const scene::Scene& scene,
                               const std::string& filePath);

        /// @brief Reads JSON from @p filePath and deserializes a new Scene.
        /// @return A unique_ptr to the new Scene, or nullptr on error.
        static std::unique_ptr<scene::Scene> LoadFromFile(const std::string& filePath);
    };

    // ========================================================================
    // JsonSceneLoader — concrete ISceneLoader backed by SceneSerializer.
    // ========================================================================

    class JsonSceneLoader final : public scene::ISceneLoader
    {
    public:
        [[nodiscard]] std::unique_ptr<scene::Scene> Load(
            const std::string& filePath) override;

        [[nodiscard]] bool SupportsExtension(
            const std::string& extension) const override;
    };

    // ========================================================================
    // JsonSceneSaver — concrete ISceneSaver backed by SceneSerializer.
    // ========================================================================

    class JsonSceneSaver final : public scene::ISceneSaver
    {
    public:
        bool Save(const scene::Scene& scene,
                  const std::string& filePath) override;

        [[nodiscard]] std::string GetDefaultExtension() const override;
    };

} // namespace engine::serialization
