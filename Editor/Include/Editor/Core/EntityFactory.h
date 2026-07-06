// ============================================================================
// File: Editor/Include/Editor/Core/EntityFactory.h
// Creates entities in the active scene.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

namespace editor {

    class EditorContext;

    /// @brief Factory for creating entities in the editor.  Provides
    ///        convenience methods for creating common entity types.
    class EntityFactory
    {
    public:
        EntityFactory() = default;
        ~EntityFactory() = default;

        /// @brief Creates an empty entity with a TagComponent and IDComponent.
        static engine::ecs::Entity CreateEmpty(EditorContext& context,
                                                const std::string& name = "Entity");

        /// @brief Creates a camera entity with a CameraComponent.
        static engine::ecs::Entity CreateCamera(EditorContext& context,
                                                  const std::string& name = "Camera");

        /// @brief Creates a directional light entity.
        static engine::ecs::Entity CreateDirectionalLight(EditorContext& context,
                                                            const std::string& name = "Directional Light");

        /// @brief Creates a point light entity.
        static engine::ecs::Entity CreatePointLight(EditorContext& context,
                                                     const std::string& name = "Point Light");

        /// @brief Creates a cube entity with a MeshComponent.
        static engine::ecs::Entity CreateCube(EditorContext& context,
                                               const std::string& name = "Cube");

        /// @brief Creates a plane entity with a MeshComponent.
        static engine::ecs::Entity CreatePlane(EditorContext& context,
                                                const std::string& name = "Plane");

        /// @brief Creates a sphere entity with a MeshComponent.
        static engine::ecs::Entity CreateSphere(EditorContext& context,
                                                 const std::string& name = "Sphere");

        /// @brief Deletes an entity and all its children.
        static void DeleteEntity(EditorContext& context, engine::ecs::Entity entity);
    };

} // namespace editor
