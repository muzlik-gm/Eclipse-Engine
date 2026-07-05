#!/usr/bin/env python3
"""Generate all missing Phase 5 files for Eclipse Engine."""
import os

BASE = "/home/z/my-project/Engine/Include/Engine"
SRC  = "/home/z/my-project/Engine/Source"

def write(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(content)
    print(f"  Created {path}")

# ============================================================================
# BATCH 8: Missing Components
# ============================================================================

write(f"{BASE}/Components/NameComponent.h", """\
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
""")

write(f"{BASE}/Components/MeshComponent.h", """\
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
""")

write(f"{BASE}/Components/CameraComponent.h", """\
// ============================================================================
// File: Engine/Include/Engine/Components/CameraComponent.h
// Runtime camera parameters for view/projection matrix generation.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::core::u32;
    using engine::math::Mat4;

    /// @brief Projection mode for the camera.
    enum class ProjectionMode : u32
    {
        Perspective = 0,
        Orthographic = 1
    };

    /// @brief Attaches camera parameters to an entity.
    struct CameraComponent
    {
        ProjectionMode Projection{ProjectionMode::Perspective};
        f32 FieldOfView{60.0f};
        f32 NearClip{0.1f};
        f32 FarClip{1000.0f};
        f32 OrthoSize{10.0f};
        bool Primary{false};
        f32 AspectRatio{0.0f};

        [[nodiscard]] Mat4 GetPerspectiveMatrix(f32 viewportAspect) const
        {
            f32 aspect = (AspectRatio > 0.0f) ? AspectRatio : viewportAspect;
            return math::Perspective(glm::radians(FieldOfView), aspect, NearClip, FarClip);
        }

        [[nodiscard]] Mat4 GetOrthographicMatrix(f32 viewportAspect) const
        {
            f32 aspect = (AspectRatio > 0.0f) ? AspectRatio : viewportAspect;
            f32 halfH = OrthoSize;
            f32 halfW = halfH * aspect;
            return math::Orthographic(-halfW, halfW, -halfH, halfH, NearClip, FarClip);
        }

        [[nodiscard]] Mat4 GetProjectionMatrix(f32 viewportAspect) const
        {
            return (Projection == ProjectionMode::Perspective)
                ? GetPerspectiveMatrix(viewportAspect)
                : GetOrthographicMatrix(viewportAspect);
        }
    };

} // namespace engine::components
""")

write(f"{BASE}/Components/LightComponent.h", """\
// ============================================================================
// File: Engine/Include/Engine/Components/LightComponent.h
// Runtime light parameters.  Data only, no lighting implementation.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::core::u32;
    using engine::core::i32;
    using engine::math::Vec3;

    /// @brief Type of light source.
    enum class LightType : u32
    {
        None        = 0,
        Directional = 1,
        Point       = 2,
        Spot        = 3
    };

    /// @brief Attaches light parameters to an entity.
    struct LightComponent
    {
        LightType Type{LightType::None};
        Vec3 Color{1.0f, 1.0f, 1.0f};
        f32  Intensity{1.0f};
        f32  Range{10.0f};
        f32  SpotInnerAngle{12.5f};
        f32  SpotOuterAngle{17.5f};
        bool CastShadows{false};
        i32  ShadowMapResolution{1024};
        f32  ShadowBias{0.005f};
    };

} // namespace engine::components
""")

write(f"{BASE}/Components/ScriptComponent.h", """\
// ============================================================================
// File: Engine/Include/Engine/Components/ScriptComponent.h
// Runtime representation of an entity script attachment.
// ============================================================================
#pragma once

#include <string>

namespace engine::components {

    /// @brief Attaches a script to an entity.
    struct ScriptComponent
    {
        std::string ScriptPath;
        bool        IsEnabled{true};
    };

} // namespace engine::components
""")

write(f"{BASE}/Components/AudioSource.h", """\
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
""")

write(f"{BASE}/Components/Collider.h", """\
// ============================================================================
// File: Engine/Include/Engine/Components/Collider.h
// Runtime representation of a collision shape.
// ============================================================================
#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;
    using engine::math::Vec3;

    /// @brief Shape of the collider.
    enum class ColliderType : int
    {
        None    = 0,
        Box     = 1,
        Sphere  = 2,
        Capsule = 3,
        Mesh    = 4
    };

    /// @brief Attaches collision shape parameters to an entity.
    struct Collider
    {
        ColliderType Type{ColliderType::None};
        Vec3 Size{1.0f, 1.0f, 1.0f};
        Vec3 Center{0.0f, 0.0f, 0.0f};
        bool IsTrigger{false};
        f32  Friction{0.5f};
        f32  Restitution{0.0f};
    };

} // namespace engine::components
""")

write(f"{BASE}/Components/RigidBody.h", """\
// ============================================================================
// File: Engine/Include/Engine/Components/RigidBody.h
// Runtime representation of a rigid body.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::components {

    using engine::core::f32;

    /// @brief Determines how a rigid body interacts with the physics world.
    enum class BodyType : int
    {
        Static    = 0,
        Dynamic   = 1,
        Kinematic = 2
    };

    /// @brief Attaches rigid body parameters to an entity.
    struct RigidBody
    {
        BodyType Type{BodyType::Dynamic};
        f32 Mass{1.0f};
        f32 LinearDrag{0.0f};
        f32 AngularDrag{0.05f};
        bool UseGravity{true};
        bool IsKinematic{false};
        float LinearVelocityX{0.0f};
        float LinearVelocityY{0.0f};
        float LinearVelocityZ{0.0f};
        float AngularVelocityX{0.0f};
        float AngularVelocityY{0.0f};
        float AngularVelocityZ{0.0f};
    };

} // namespace engine::components
""")

# ============================================================================
# BATCH 9: SceneLifecycle, SceneContext, EntityManager
# ============================================================================

write(f"{BASE}/Scene/SceneLifecycle.h", """\
// ============================================================================
// File: Engine/Include/Engine/Scene/SceneLifecycle.h
// Explicit lifecycle states for a Scene.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"

namespace engine::scene {

    using engine::core::u32;

    enum class SceneState : u32
    {
        Unloaded    = 0,
        Loading     = 1,
        Loaded      = 2,
        Activating  = 3,
        Active      = 4,
        Deactivating = 5,
        Inactive    = 6,
        Unloading   = 7,
        Destroyed   = 8
    };

    [[nodiscard]] constexpr const char* SceneStateToString(SceneState state) noexcept
    {
        switch (state)
        {
            case SceneState::Unloaded:     return "Unloaded";
            case SceneState::Loading:      return "Loading";
            case SceneState::Loaded:       return "Loaded";
            case SceneState::Activating:   return "Activating";
            case SceneState::Active:       return "Active";
            case SceneState::Deactivating: return "Deactivating";
            case SceneState::Inactive:     return "Inactive";
            case SceneState::Unloading:    return "Unloading";
            case SceneState::Destroyed:    return "Destroyed";
            default:                       return "Unknown";
        }
    }

} // namespace engine::scene
""")

write(f"{BASE}/Scene/SceneContext.h", """\
// ============================================================================
// File: Engine/Include/Engine/Scene/SceneContext.h
// Provides shared context that systems can access during scene updates.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"

#include <chrono>

namespace engine::scene {

    using engine::core::f64;
    using engine::core::u64;
    using engine::math::Vec2;

    /// @brief Immutable per-frame context passed to every system.
    class SceneContext
    {
    public:
        f64  DeltaTime{0.0};
        f64  FixedDeltaTime{1.0 / 60.0};
        f64  ElapsedTime{0.0};
        Vec2 ViewportSize{1280.0f, 720.0f};
        u64  FrameNumber{0};
        std::chrono::high_resolution_clock::time_point FrameStart;

        SceneContext() = default;
    };

} // namespace engine::scene
""")

write(f"{BASE}/Entities/EntityManager.h", """\
// ============================================================================
// File: Engine/Include/Engine/Entities/EntityManager.h
// Higher-level entity management facade on top of Registry.
// ============================================================================
#pragma once

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Core/UUID.h"

#include <string>
#include <vector>

namespace engine::entities {

    using engine::core::UUID;
    using engine::core::usize;

    /// @brief Convenience layer on top of ecs::Registry.
    class EntityManager
    {
    public:
        explicit EntityManager(ecs::Registry& registry) noexcept
            : m_registry(&registry) {}

        [[nodiscard]] ecs::Entity Create(const std::string& tag = "Entity");
        [[nodiscard]] ecs::Entity CreateNamed(const std::string& tag, const std::string& name);
        void Destroy(ecs::Entity entity);
        [[nodiscard]] bool IsValid(ecs::Entity entity) const;
        [[nodiscard]] ecs::Entity FindByTag(const std::string& tag) const;
        [[nodiscard]] ecs::Entity FindByUUID(const UUID& uuid) const;
        [[nodiscard]] std::vector<ecs::Entity> GetAll() const;

    private:
        ecs::Registry* m_registry;
    };

} // namespace engine::entities
""")

write(f"{SRC}/Entities/EntityManager.cpp", """\
// ============================================================================
// File: Engine/Source/Entities/EntityManager.cpp
// ============================================================================
#include "Engine/Entities/EntityManager.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/NameComponent.h"

namespace engine::entities {

    ecs::Entity EntityManager::Create(const std::string& tag)
    {
        ecs::Entity entity = m_registry->CreateEntity();
        m_registry->AddComponent<components::TagComponent>(entity, components::TagComponent{tag});
        m_registry->AddComponent<components::IDComponent>(entity, components::IDComponent{UUID{}});
        return entity;
    }

    ecs::Entity EntityManager::CreateNamed(const std::string& tag, const std::string& name)
    {
        ecs::Entity entity = Create(tag);
        m_registry->AddComponent<components::NameComponent>(entity, components::NameComponent{name});
        return entity;
    }

    void EntityManager::Destroy(ecs::Entity entity)
    {
        m_registry->DestroyEntity(entity);
    }

    bool EntityManager::IsValid(ecs::Entity entity) const
    {
        return m_registry->IsValid(entity);
    }

    ecs::Entity EntityManager::FindByTag(const std::string& tag) const
    {
        for (auto eh : m_registry->View<components::TagComponent>())
        {
            ecs::Entity entity{eh};
            auto& tc = m_registry->GetComponent<components::TagComponent>(entity);
            if (tc.Tag == tag)
                return entity;
        }
        return ecs::Invalid;
    }

    ecs::Entity EntityManager::FindByUUID(const UUID& uuid) const
    {
        for (auto eh : m_registry->View<components::IDComponent>())
        {
            ecs::Entity entity{eh};
            auto& idc = m_registry->GetComponent<components::IDComponent>(entity);
            if (idc.ID == uuid)
                return entity;
        }
        return ecs::Invalid;
    }

    std::vector<ecs::Entity> EntityManager::GetAll() const
    {
        std::vector<ecs::Entity> result;
        for (auto eh : m_registry->View<components::TagComponent>())
        {
            result.push_back(ecs::Entity{eh});
        }
        return result;
    }

} // namespace engine::entities
""")

# ============================================================================
# BATCH 10: Serialization Interfaces
# ============================================================================

write(f"{BASE}/Serialization/Interfaces/ISceneLoader.h", """\
// ============================================================================
// File: Engine/Include/Engine/Serialization/Interfaces/ISceneLoader.h
// Abstract interface for loading a scene from external storage.
// ============================================================================
#pragma once

#include <memory>
#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Abstract interface for scene loading backends.
    class ISceneLoader
    {
    public:
        virtual ~ISceneLoader() = default;

        /// @brief Loads a scene from the given file path.
        [[nodiscard]] virtual std::unique_ptr<Scene> Load(
            const std::string& filePath) = 0;

        /// @brief Returns true if this loader can handle the given extension.
        [[nodiscard]] virtual bool SupportsExtension(
            const std::string& extension) const = 0;
    };

} // namespace engine::scene
""")

write(f"{BASE}/Serialization/Interfaces/ISceneSaver.h", """\
// ============================================================================
// File: Engine/Include/Engine/Serialization/Interfaces/ISceneSaver.h
// Abstract interface for saving a scene to external storage.
// ============================================================================
#pragma once

#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Abstract interface for scene saving backends.
    class ISceneSaver
    {
    public:
        virtual ~ISceneSaver() = default;

        virtual bool Save(const Scene& scene, const std::string& filePath) = 0;

        [[nodiscard]] virtual std::string GetDefaultExtension() const = 0;
    };

} // namespace engine::scene
""")

# ============================================================================
# BATCH 11: Component Metadata & Reflection
# ============================================================================

write(f"{BASE}/ECS/ComponentRegistry.h", """\
// ============================================================================
// File: Engine/Include/Engine/ECS/ComponentRegistry.h
// Runtime type registry for components.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"

#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::ecs {

    using engine::core::usize;

    /// @brief Stores runtime metadata about a registered component type.
    struct ComponentMeta
    {
        std::string     Name;
        std::type_index TypeIndex;
        usize           Size;
        std::function<void(Registry&, Entity)> RemoveFn;
    };

    /// @brief Registry that stores metadata for every registered component type.
    class ComponentRegistry
    {
    public:
        template <typename T>
        void Register(const std::string& name)
        {
            ComponentMeta meta;
            meta.Name      = name;
            meta.TypeIndex = std::type_index(typeid(T));
            meta.Size      = sizeof(T);
            meta.RemoveFn   = [](Registry& reg, Entity ent) {
                if (reg.HasComponent<T>(ent))
                    reg.RemoveComponent<T>(ent);
            };
            m_Components[std::type_index(typeid(T))] = std::move(meta);
            m_OrderedNames.push_back(name);
        }

        [[nodiscard]] const ComponentMeta* GetMeta(std::type_index idx) const
        {
            auto it = m_Components.find(idx);
            return (it != m_Components.end()) ? &it->second : nullptr;
        }

        [[nodiscard]] const ComponentMeta* GetMetaByName(const std::string& name) const
        {
            for (const auto& [idx, meta] : m_Components)
            {
                if (meta.Name == name)
                    return &meta;
            }
            return nullptr;
        }

        [[nodiscard]] usize Count() const { return m_Components.size(); }

        [[nodiscard]] const std::vector<std::string>& GetNames() const
        {
            return m_OrderedNames;
        }

        void Clear()
        {
            m_Components.clear();
            m_OrderedNames.clear();
        }

    private:
        std::unordered_map<std::type_index, ComponentMeta> m_Components;
        std::vector<std::string>                           m_OrderedNames;
    };

} // namespace engine::ecs
""")

# ============================================================================
# BATCH 12: System Scheduler
# ============================================================================

write(f"{BASE}/Systems/SystemScheduler.h", """\
// ============================================================================
// File: Engine/Include/Engine/Systems/SystemScheduler.h
// Manages system execution order via groups, priorities, and dependencies.
// ============================================================================
#pragma once

#include "Engine/Systems/ISystem.h"
#include "Engine/Core/Types.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::systems {

    using engine::core::f64;
    using engine::core::i32;
    using engine::core::usize;

    /// @brief Bookkeeping for each registered system.
    struct SystemDescriptor
    {
        std::unique_ptr<ISystem>  System;
        std::string              Group{"Default"};
        i32                      Priority{0};
        std::unordered_set<std::string> Dependencies;
    };

    /// @brief Manages system registration, ordering, and execution.
    class SystemScheduler
    {
    public:
        SystemScheduler()  = default;
        ~SystemScheduler() = default;

        template <typename T, typename... Args>
        T& Add(ecs::Registry& registry, const std::string& group = "Default",
               i32 priority = 0, Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->OnAttach(registry);

            SystemDescriptor desc;
            desc.System   = std::move(system);
            desc.Group    = group;
            desc.Priority = priority;

            T& ref = static_cast<T&>(*desc.System);
            m_Systems.push_back(std::move(desc));
            m_SortDirty = true;
            return ref;
        }

        void Clear();

        void Update(f64 deltaTime);
        void FixedUpdate(f64 fixedDeltaTime);
        void LateUpdate(f64 deltaTime);

        [[nodiscard]] usize SystemCount() const { return m_Systems.size(); }
        [[nodiscard]] ISystem* GetSystem(usize index);
        [[nodiscard]] ISystem* GetSystemByName(std::string_view name);

    private:
        void SortSystems();

        std::vector<SystemDescriptor> m_Systems;
        bool m_SortDirty{true};
    };

} // namespace engine::systems
""")

write(f"{SRC}/Systems/SystemScheduler.cpp", """\
// ============================================================================
// File: Engine/Source/Systems/SystemScheduler.cpp
// ============================================================================
#include "Engine/Systems/SystemScheduler.h"
#include <algorithm>

namespace engine::systems {

    void SystemScheduler::Clear()
    {
        for (auto& desc : m_Systems)
        {
            if (desc.System)
                desc.System->OnDetach();
        }
        m_Systems.clear();
        m_SortDirty = true;
    }

    void SystemScheduler::Update(f64 deltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->Update(deltaTime);
        }
    }

    void SystemScheduler::FixedUpdate(f64 fixedDeltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->FixedUpdate(fixedDeltaTime);
        }
    }

    void SystemScheduler::LateUpdate(f64 deltaTime)
    {
        SortSystems();
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->IsEnabled())
                desc.System->LateUpdate(deltaTime);
        }
    }

    ISystem* SystemScheduler::GetSystem(usize index)
    {
        SortSystems();
        return (index < m_Systems.size()) ? m_Systems[index].System.get() : nullptr;
    }

    ISystem* SystemScheduler::GetSystemByName(std::string_view name)
    {
        for (auto& desc : m_Systems)
        {
            if (desc.System && desc.System->GetName() == name)
                return desc.System.get();
        }
        return nullptr;
    }

    void SystemScheduler::SortSystems()
    {
        if (!m_SortDirty)
            return;

        std::stable_sort(m_Systems.begin(), m_Systems.end(),
            [](const SystemDescriptor& a, const SystemDescriptor& b)
            {
                if (a.Group != b.Group)
                    return a.Group < b.Group;
                return a.Priority < b.Priority;
            });

        m_SortDirty = false;
    }

} // namespace engine::systems
""")

# ============================================================================
# BATCH 13: WorldManager & SceneManager
# ============================================================================

write(f"{BASE}/World/WorldManager.h", """\
// ============================================================================
// File: Engine/Include/Engine/World/WorldManager.h
// High-level facade that owns the World.
// ============================================================================
#pragma once

#include "Engine/World/World.h"
#include "Engine/Runtime/ISubsystem.h"

namespace engine::world {

    /// @brief Top-level manager that owns the World subsystem.
    ///
    /// Architecture: Application -> Engine -> WorldManager -> World -> Scene
    class WorldManager final : public runtime::ISubsystem
    {
    public:
        WorldManager() = default;
        ~WorldManager() override = default;

        WorldManager(const WorldManager&)            = delete;
        WorldManager& operator=(const WorldManager&) = delete;
        WorldManager(WorldManager&&)                 = delete;
        WorldManager& operator=(WorldManager&&)      = delete;

        [[nodiscard]] std::string_view GetName() const noexcept override;
        [[nodiscard]] std::vector<std::string> GetDependencies() const override;

        bool Initialize() override;
        void Shutdown() override;

        void Update(f64 deltaTime) override;
        void FixedUpdate(f64 fixedDeltaTime) override;
        void LateUpdate(f64 deltaTime) override;

        [[nodiscard]] World& GetWorld() { return m_world; }
        [[nodiscard]] const World& GetWorld() const { return m_world; }

    private:
        World m_world;
    };

} // namespace engine::world
""")

write(f"{SRC}/World/WorldManager.cpp", """\
// ============================================================================
// File: Engine/Source/World/WorldManager.cpp
// ============================================================================
#include "Engine/World/WorldManager.h"

namespace engine::world {

    std::string_view WorldManager::GetName() const noexcept { return "WorldManager"; }
    std::vector<std::string> WorldManager::GetDependencies() const { return {}; }

    bool WorldManager::Initialize() { return m_world.Initialize(); }
    void WorldManager::Shutdown() { m_world.Shutdown(); }
    void WorldManager::Update(f64 deltaTime) { m_world.Update(deltaTime); }
    void WorldManager::FixedUpdate(f64 fixedDeltaTime) { m_world.FixedUpdate(fixedDeltaTime); }
    void WorldManager::LateUpdate(f64 deltaTime) { m_world.LateUpdate(deltaTime); }

} // namespace engine::world
""")

write(f"{BASE}/Scene/SceneManager.h", """\
// ============================================================================
// File: Engine/Include/Engine/Scene/SceneManager.h
// Utility class for scene loading, saving, and lifecycle transitions.
// ============================================================================
#pragma once

#include "Engine/Scene/SceneLifecycle.h"
#include "Engine/Core/UUID.h"

#include <memory>
#include <string>

namespace engine::scene {

    class Scene;

    /// @brief Utility for scene creation and lifecycle management.
    class SceneManager
    {
    public:
        SceneManager()  = default;
        ~SceneManager() = default;

        [[nodiscard]] std::unique_ptr<Scene> CreateEmpty(const std::string& name) const;
        void TransitionTo(Scene& scene, SceneState target) const;
        [[nodiscard]] SceneState GetState(const Scene& scene) const;

        [[nodiscard]] static bool CanUpdate(SceneState state)
        {
            return state == SceneState::Active;
        }
    };

} // namespace engine::scene
""")

write(f"{SRC}/Scene/SceneManager.cpp", """\
// ============================================================================
// File: Engine/Source/Scene/SceneManager.cpp
// ============================================================================
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"

namespace engine::scene {

    std::unique_ptr<Scene> SceneManager::CreateEmpty(const std::string& name) const
    {
        return std::make_unique<Scene>(name);
    }

    void SceneManager::TransitionTo(Scene& scene, SceneState target) const
    {
        SceneState current = GetState(scene);

        switch (target)
        {
            case SceneState::Loaded:
                break;
            case SceneState::Active:
                if (current == SceneState::Loaded || current == SceneState::Inactive)
                    scene.SetActive(true);
                break;
            case SceneState::Inactive:
                if (current == SceneState::Active || current == SceneState::Deactivating)
                    scene.SetActive(false);
                break;
            default:
                break;
        }
    }

    SceneState SceneManager::GetState(const Scene& scene) const
    {
        return scene.IsActive() ? SceneState::Active : SceneState::Loaded;
    }

} // namespace engine::scene
""")

# ============================================================================
# BATCH 14: IRenderInterface stub
# ============================================================================

os.makedirs(f"{BASE}/Rendering", exist_ok=True)

write(f"{BASE}/Rendering/IRenderInterface.h", """\
// ============================================================================
// File: Engine/Include/Engine/Rendering/IRenderInterface.h
// Abstract interface for submitting renderable data from World to Renderer.
// ============================================================================
#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/Core/Types.h"

namespace engine::rendering {

    using engine::ecs::Entity;

    /// @brief Minimal renderable data packet extracted from World entities.
    struct RenderSubmitInfo
    {
        Entity Entity;
        void* MeshData{nullptr};
        void* TransformData{nullptr};
        void* CameraData{nullptr};
        bool  IsVisible{true};
    };

    /// @brief Abstract interface for the renderer to receive renderable data.
    class IRenderInterface
    {
    public:
        virtual ~IRenderInterface() = default;

        virtual void SubmitRenderable(const RenderSubmitInfo& info) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void SetActiveCamera(const RenderSubmitInfo& cameraInfo) = 0;
    };

} // namespace engine::rendering
""")

print("\nAll Phase 5 missing files generated successfully!")