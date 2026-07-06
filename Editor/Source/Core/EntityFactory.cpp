// ============================================================================
// File: Editor/Source/Core/EntityFactory.cpp
// ============================================================================
#include "Editor/Core/EntityFactory.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Selection/EditorSelection.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Hierarchy/HierarchyManager.h"
#include "Engine/Core/Log.h"

namespace editor {

    using namespace engine::components;

    engine::ecs::Entity EntityFactory::CreateEmpty(EditorContext& context,
                                                     const std::string& name)
    {
        auto* scene = context.GetActiveScene();
        if (!scene)
            return engine::ecs::Invalid;

        auto handle = scene->CreateEntity(name);
        handle.AddComponent<TransformComponent>();
        ENGINE_LOG_INFO("EntityFactory — created empty entity '{}'", name);
        return handle.GetId();
    }

    engine::ecs::Entity EntityFactory::CreateCamera(EditorContext& context,
                                                      const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        CameraComponent cam;
        cam.Projection = ProjectionMode::Perspective;
        cam.FieldOfView = 60.0f;
        cam.NearClip = 0.1f;
        cam.FarClip = 1000.0f;
        cam.Primary = true;
        registry.AddComponent<CameraComponent>(entity, cam);

        ENGINE_LOG_INFO("EntityFactory — created camera '{}'", name);
        return entity;
    }

    engine::ecs::Entity EntityFactory::CreateDirectionalLight(EditorContext& context,
                                                                const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        LightComponent light;
        light.Type = LightType::Directional;
        light.Color = engine::math::Vec3(1.0f, 1.0f, 0.9f);
        light.Intensity = 1.0f;
        registry.AddComponent<LightComponent>(entity, light);

        ENGINE_LOG_INFO("EntityFactory — created directional light '{}'", name);
        return entity;
    }

    engine::ecs::Entity EntityFactory::CreatePointLight(EditorContext& context,
                                                          const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        LightComponent light;
        light.Type = LightType::Point;
        light.Color = engine::math::Vec3(1.0f, 1.0f, 1.0f);
        light.Intensity = 1.0f;
        light.Range = 10.0f;
        registry.AddComponent<LightComponent>(entity, light);

        ENGINE_LOG_INFO("EntityFactory — created point light '{}'", name);
        return entity;
    }

    engine::ecs::Entity EntityFactory::CreateCube(EditorContext& context,
                                                    const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        MeshComponent mesh;
        mesh.Type = MeshType::Cube;
        mesh.CastShadows = true;
        registry.AddComponent<MeshComponent>(entity, mesh);

        ENGINE_LOG_INFO("EntityFactory — created cube '{}'", name);
        return entity;
    }

    engine::ecs::Entity EntityFactory::CreatePlane(EditorContext& context,
                                                     const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        MeshComponent mesh;
        mesh.Type = MeshType::Plane;
        mesh.CastShadows = true;
        registry.AddComponent<MeshComponent>(entity, mesh);

        ENGINE_LOG_INFO("EntityFactory — created plane '{}'", name);
        return entity;
    }

    engine::ecs::Entity EntityFactory::CreateSphere(EditorContext& context,
                                                      const std::string& name)
    {
        auto entity = CreateEmpty(context, name);
        if (entity == engine::ecs::Invalid)
            return entity;

        auto* scene = context.GetActiveScene();
        auto& registry = scene->GetRegistry();

        MeshComponent mesh;
        mesh.Type = MeshType::Sphere;
        mesh.CastShadows = true;
        registry.AddComponent<MeshComponent>(entity, mesh);

        ENGINE_LOG_INFO("EntityFactory — created sphere '{}'", name);
        return entity;
    }

    void EntityFactory::DeleteEntity(EditorContext& context, engine::ecs::Entity entity)
    {
        if (entity == engine::ecs::Invalid)
            return;

        auto* scene = context.GetActiveScene();
        if (!scene)
            return;

        auto& registry = scene->GetRegistry();

        // Deselect if the entity is currently selected.
        context.GetSelection().DeselectEntity(entity);

        // Destroy with descendants.
        engine::hierarchy::HierarchyManager::DestroyWithDescendants(registry, entity);

        ENGINE_LOG_INFO("EntityFactory — deleted entity");
    }

} // namespace editor
