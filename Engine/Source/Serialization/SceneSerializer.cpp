// ============================================================================
// File: Engine/Source/Serialization/SceneSerializer.cpp
// SceneSerializer implementation — JSON serialize / deserialize with
// UUID-based hierarchy references and full component coverage.
// ============================================================================

#include "Engine/Serialization/SceneSerializer.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <limits>
#include <unordered_map>

#include "Engine/Core/Log.h"
#include "Engine/Core/UUID.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Components/VisibilityComponent.h"
#include "Engine/Components/EnabledComponent.h"
#include "Engine/Components/StaticComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/ScriptComponent.h"
#include "Engine/Components/AudioSource.h"
#include "Engine/Components/Collider.h"
#include "Engine/Components/RigidBody.h"
#include "Engine/Hierarchy/HierarchyUtils.h"

namespace engine::serialization {

    using engine::core::f32;
    using engine::core::u32;
    using engine::core::UUID;
    using engine::ecs::Entity;
    using engine::ecs::Registry;
    using engine::components::TagComponent;
    using engine::components::IDComponent;
    using engine::components::NameComponent;
    using engine::components::TransformComponent;
    using engine::components::HierarchyComponent;
    using engine::components::VisibilityComponent;
    using engine::components::EnabledComponent;
    using engine::components::StaticComponent;
    using engine::components::MeshComponent;
    using engine::components::CameraComponent;
    using engine::components::LightComponent;
    using engine::components::ScriptComponent;
    using engine::components::AudioSource;
    using engine::components::Collider;
    using engine::components::RigidBody;
    using nlohmann::json;

    // ========================================================================
    // Helper: UUID serialization
    // ========================================================================

    static json SerializeUUID(const UUID& uuid)
    {
        if (uuid.IsValid())
            return uuid.ToString();
        return nullptr;
    }

    static UUID DeserializeUUID(const json& j)
    {
        if (j.is_null())
            return UUID{};
        if (j.is_string())
            return UUID::FromString(j.get<std::string>());
        return UUID{};
    }

    // ========================================================================
    // Helper: Vec3 / Quat serialization
    // ========================================================================

    static json SerializeVec3(const math::Vec3& v)
    {
        return json::array({v.x, v.y, v.z});
    }

    static math::Vec3 DeserializeVec3(const json& j, const math::Vec3& fallback = math::Vec3{0.0f, 0.0f, 0.0f})
    {
        if (j.is_array() && j.size() == 3)
            return math::Vec3(j[0].get<f32>(), j[1].get<f32>(), j[2].get<f32>());
        return fallback;
    }

    static json SerializeQuat(const math::Quat& q)
    {
        // glm::quat constructor order is (w, x, y, z) — match it so the
        // round-trip through DeserializeQuat is lossless.
        return json::array({q.w, q.x, q.y, q.z});
    }

    static math::Quat DeserializeQuat(const json& j)
    {
        if (j.is_array() && j.size() == 4)
            return math::Quat(j[0].get<f32>(), j[1].get<f32>(),
                              j[2].get<f32>(), j[3].get<f32>());
        return math::Quat{1.0f, 0.0f, 0.0f, 0.0f};
    }

    // ========================================================================
    // Serialize — per-component
    // ========================================================================

    static void SerializeEntityComponents(json& entityJson,
                                          const Registry& registry,
                                          Entity entity)
    {
        // -- TagComponent ---------------------------------------------------
        if (registry.HasComponent<TagComponent>(entity))
        {
            const auto& c = registry.GetComponent<TagComponent>(entity);
            entityJson["tag"] = c.Tag;
        }

        // -- IDComponent ----------------------------------------------------
        if (registry.HasComponent<IDComponent>(entity))
        {
            const auto& c = registry.GetComponent<IDComponent>(entity);
            entityJson["uuid"] = c.ID.ToString();
        }

        // -- NameComponent --------------------------------------------------
        if (registry.HasComponent<NameComponent>(entity))
        {
            const auto& c = registry.GetComponent<NameComponent>(entity);
            entityJson["name"] = c.Name;
        }

        // -- TransformComponent ---------------------------------------------
        if (registry.HasComponent<TransformComponent>(entity))
        {
            const auto& tc = registry.GetComponent<TransformComponent>(entity);
            json t;
            t["translation"] = SerializeVec3(tc.Translation);
            t["rotation"]    = SerializeQuat(tc.Rotation);
            t["scale"]       = SerializeVec3(tc.Scale);
            entityJson["transform"] = t;
        }

        // -- HierarchyComponent (Parent UUID only) --------------------------
        if (registry.HasComponent<HierarchyComponent>(entity))
        {
            const auto& hc = registry.GetComponent<HierarchyComponent>(entity);

            // Serialize the parent's UUID so hierarchy links survive
            // entity-index recycling across save / load.
            std::string parentUUIDStr;
            if (hc.Parent != entt::null
                && registry.HasComponent<IDComponent>(hc.Parent))
            {
                parentUUIDStr = registry.GetComponent<IDComponent>(hc.Parent).ID.ToString();
            }

            json h;
            h["parent_uuid"] = parentUUIDStr;
            h["child_count"] = hc.ChildCount;
            entityJson["hierarchy"] = h;
        }

        // -- VisibilityComponent --------------------------------------------
        if (registry.HasComponent<VisibilityComponent>(entity))
        {
            const auto& c = registry.GetComponent<VisibilityComponent>(entity);
            entityJson["visibility"] = c.IsVisible;
        }

        // -- EnabledComponent -----------------------------------------------
        if (registry.HasComponent<EnabledComponent>(entity))
        {
            const auto& c = registry.GetComponent<EnabledComponent>(entity);
            entityJson["enabled"] = c.IsEnabled;
        }

        // -- StaticComponent ------------------------------------------------
        if (registry.HasComponent<StaticComponent>(entity))
        {
            const auto& c = registry.GetComponent<StaticComponent>(entity);
            entityJson["static"] = c.IsStatic;
        }

        // -- MeshComponent --------------------------------------------------
        if (registry.HasComponent<MeshComponent>(entity))
        {
            const auto& c = registry.GetComponent<MeshComponent>(entity);
            json m;
            m["type"]           = static_cast<u32>(c.Type);
            m["mesh_asset_id"]  = c.MeshAssetID.ToString();
            m["sub_mesh_index"] = c.SubMeshIndex;
            m["cast_shadows"]   = c.CastShadows;
            entityJson["mesh"] = m;
        }

        // -- CameraComponent ------------------------------------------------
        if (registry.HasComponent<CameraComponent>(entity))
        {
            const auto& c = registry.GetComponent<CameraComponent>(entity);
            json cam;
            cam["projection"]  = static_cast<u32>(c.Projection);
            cam["fov"]         = c.FieldOfView;
            cam["near_clip"]   = c.NearClip;
            cam["far_clip"]    = c.FarClip;
            cam["ortho_size"]  = c.OrthoSize;
            cam["primary"]     = c.Primary;
            cam["aspect"]      = c.AspectRatio;
            entityJson["camera"] = cam;
        }

        // -- LightComponent -------------------------------------------------
        if (registry.HasComponent<LightComponent>(entity))
        {
            const auto& c = registry.GetComponent<LightComponent>(entity);
            json l;
            l["type"]              = static_cast<u32>(c.Type);
            l["color"]             = SerializeVec3(c.Color);
            l["intensity"]         = c.Intensity;
            l["range"]             = c.Range;
            l["spot_inner"]        = c.SpotInnerAngle;
            l["spot_outer"]        = c.SpotOuterAngle;
            l["cast_shadows"]      = c.CastShadows;
            l["shadow_resolution"] = c.ShadowMapResolution;
            l["shadow_bias"]       = c.ShadowBias;
            entityJson["light"] = l;
        }

        // -- ScriptComponent ------------------------------------------------
        if (registry.HasComponent<ScriptComponent>(entity))
        {
            const auto& c = registry.GetComponent<ScriptComponent>(entity);
            json s;
            s["path"]     = c.ScriptPath;
            s["enabled"]  = c.IsEnabled;
            entityJson["script"] = s;
        }

        // -- AudioSource ----------------------------------------------------
        if (registry.HasComponent<AudioSource>(entity))
        {
            const auto& c = registry.GetComponent<AudioSource>(entity);
            json a;
            a["clip_path"]    = c.ClipPath;
            a["volume"]       = c.Volume;
            a["pitch"]        = c.Pitch;
            a["loop"]         = c.Loop;
            a["play_on_awake"] = c.PlayOnAwake;
            a["spatial"]      = c.Spatial;
            a["min_distance"] = c.MinDistance;
            a["max_distance"] = c.MaxDistance;
            a["rolloff"]      = c.RolloffFactor;
            entityJson["audio"] = a;
        }

        // -- Collider -------------------------------------------------------
        if (registry.HasComponent<Collider>(entity))
        {
            const auto& c = registry.GetComponent<Collider>(entity);
            json col;
            col["type"]        = static_cast<u32>(c.Type);
            col["size"]        = SerializeVec3(c.Size);
            col["center"]      = SerializeVec3(c.Center);
            col["is_trigger"]  = c.IsTrigger;
            col["friction"]    = c.Friction;
            col["restitution"] = c.Restitution;
            entityJson["collider"] = col;
        }

        // -- RigidBody ------------------------------------------------------
        if (registry.HasComponent<RigidBody>(entity))
        {
            const auto& c = registry.GetComponent<RigidBody>(entity);
            json rb;
            rb["type"]           = static_cast<u32>(c.Type);
            rb["mass"]           = c.Mass;
            rb["linear_drag"]    = c.LinearDrag;
            rb["angular_drag"]   = c.AngularDrag;
            rb["use_gravity"]    = c.UseGravity;
            rb["is_kinematic"]   = c.IsKinematic;
            rb["linear_vel"]     = SerializeVec3(c.LinearVelocity);
            rb["angular_vel"]    = SerializeVec3(c.AngularVelocity);
            entityJson["rigid_body"] = rb;
        }
    }

    // ========================================================================
    // Serialize — full scene
    // ========================================================================

    json SceneSerializer::Serialize(const scene::Scene& scene)
    {
        json root;
        root["name"] = scene.GetName();
        root["uuid"] = scene.GetUUID().ToString();

        auto& registry = const_cast<scene::Scene&>(scene).GetRegistry();

        // Iterate over all entities that have a TagComponent (every entity
        // created via Scene::CreateEntity gets one).
        auto view = registry.View<TagComponent>();

        json entitiesJson = json::array();
        for (auto entityHandle : view)
        {
            Entity entity{entityHandle};
            json entityJson;
            entityJson["index"] = ecs::GetIndex(entity);

            SerializeEntityComponents(entityJson, registry, entity);

            entitiesJson.push_back(entityJson);
        }

        root["entities"]    = entitiesJson;
        root["entity_count"] = entitiesJson.size();

        return root;
    }

    // ========================================================================
    // Deserialize — full scene
    // ========================================================================

    std::unique_ptr<scene::Scene> SceneSerializer::Deserialize(const json& data)
    {
        std::string name = data.value("name", "Untitled Scene");
        auto scene = std::make_unique<scene::Scene>(name);

        // Restore scene UUID if present.
        if (data.contains("uuid"))
        {
            // Scene UUID is set at construction; we leave it as-is since
            // the UUID field is const.  In a production engine we would
            // add a SetUUID method, but for now the deserialized scene
            // gets a fresh UUID.
        }

        auto& registry = scene->GetRegistry();

        if (!data.contains("entities") || !data["entities"].is_array())
            return scene;

        // ----------------------------------------------------------------
        // Pass 1: Create all entities, restore Tag / ID / Name.
        // Build UUID → Entity map for hierarchy reconstruction.
        // ----------------------------------------------------------------

        std::unordered_map<std::string, Entity> uuidToEntity;

        for (const auto& ej : data["entities"])
        {
            std::string tag = ej.value("tag", "Entity");
            auto handle = scene->CreateEntity(tag);
            Entity entity = handle.GetId();

            // Restore IDComponent UUID if present.
            if (ej.contains("uuid"))
            {
                auto uuid = UUID::FromString(ej["uuid"].get<std::string>());
                registry.GetComponent<IDComponent>(entity).ID = uuid;
                uuidToEntity[uuid.ToString()] = entity;
            }

            // Restore NameComponent.
            if (ej.contains("name"))
            {
                if (!registry.HasComponent<NameComponent>(entity))
                    registry.AddComponent<NameComponent>(entity);
                registry.GetComponent<NameComponent>(entity).Name =
                    ej["name"].get<std::string>();
            }
        }

        // ----------------------------------------------------------------
        // Pass 2: Restore all remaining components and rebuild hierarchy.
        // ----------------------------------------------------------------

        for (const auto& ej : data["entities"])
        {
            // Find this entity by its UUID.
            Entity entity = entt::null;
            if (ej.contains("uuid"))
            {
                auto it = uuidToEntity.find(ej["uuid"].get<std::string>());
                if (it != uuidToEntity.end())
                    entity = it->second;
            }
            if (entity == entt::null)
                continue;

            // -- TransformComponent -----------------------------------------
            if (ej.contains("transform"))
            {
                const auto& t = ej["transform"];
                TransformComponent tc;
                if (t.contains("translation"))
                    tc.Translation = DeserializeVec3(t["translation"]);
                if (t.contains("rotation"))
                    tc.Rotation = DeserializeQuat(t["rotation"]);
                if (t.contains("scale"))
                    tc.Scale = DeserializeVec3(t["scale"], math::Vec3{1.0f, 1.0f, 1.0f});
                tc.WorldDirty = true;
                registry.AddComponent<TransformComponent>(entity, tc);
            }

            // -- HierarchyComponent (rebuild via SetParent) -----------------
            if (ej.contains("hierarchy"))
            {
                const auto& h = ej["hierarchy"];
                std::string parentUUIDStr = h.value("parent_uuid", std::string{});

                if (!parentUUIDStr.empty())
                {
                    auto it = uuidToEntity.find(parentUUIDStr);
                    if (it != uuidToEntity.end())
                    {
                        // Ensure both entities have HierarchyComponent.
                        if (!registry.HasComponent<HierarchyComponent>(entity))
                            registry.AddComponent<HierarchyComponent>(entity);
                        hierarchy::SetParent(registry, entity, it->second);
                    }
                }
                else
                {
                    // Root entity — just add the HierarchyComponent.
                    if (!registry.HasComponent<HierarchyComponent>(entity))
                        registry.AddComponent<HierarchyComponent>(entity);
                }
            }

            // -- VisibilityComponent ----------------------------------------
            if (ej.contains("visibility"))
            {
                VisibilityComponent vc;
                vc.IsVisible = ej["visibility"].get<bool>();
                registry.AddComponent<VisibilityComponent>(entity, vc);
            }

            // -- EnabledComponent -------------------------------------------
            if (ej.contains("enabled"))
            {
                EnabledComponent ec;
                ec.IsEnabled = ej["enabled"].get<bool>();
                registry.AddComponent<EnabledComponent>(entity, ec);
            }

            // -- StaticComponent --------------------------------------------
            if (ej.contains("static"))
            {
                StaticComponent sc;
                sc.IsStatic = ej["static"].get<bool>();
                registry.AddComponent<StaticComponent>(entity, sc);
            }

            // -- MeshComponent ----------------------------------------------
            if (ej.contains("mesh"))
            {
                const auto& m = ej["mesh"];
                MeshComponent mc;
                mc.Type          = static_cast<components::MeshType>(
                    m.value("type", 0u));
                mc.MeshAssetID   = m.contains("mesh_asset_id")
                    ? UUID::FromString(m["mesh_asset_id"].get<std::string>())
                    : UUID{};
                mc.SubMeshIndex  = m.value("sub_mesh_index", 0u);
                mc.CastShadows   = m.value("cast_shadows", true);
                registry.AddComponent<MeshComponent>(entity, mc);
            }

            // -- CameraComponent --------------------------------------------
            if (ej.contains("camera"))
            {
                const auto& c = ej["camera"];
                CameraComponent cc;
                cc.Projection  = static_cast<components::ProjectionMode>(
                    c.value("projection", 0u));
                cc.FieldOfView = c.value("fov", 60.0f);
                cc.NearClip    = c.value("near_clip", 0.1f);
                cc.FarClip     = c.value("far_clip", 1000.0f);
                cc.OrthoSize   = c.value("ortho_size", 10.0f);
                cc.Primary     = c.value("primary", false);
                cc.AspectRatio = c.value("aspect", 0.0f);
                registry.AddComponent<CameraComponent>(entity, cc);
            }

            // -- LightComponent ---------------------------------------------
            if (ej.contains("light"))
            {
                const auto& l = ej["light"];
                LightComponent lc;
                lc.Type                = static_cast<components::LightType>(
                    l.value("type", 0u));
                lc.Color               = l.contains("color")
                    ? DeserializeVec3(l["color"], math::Vec3{1.0f, 1.0f, 1.0f})
                    : math::Vec3{1.0f, 1.0f, 1.0f};
                lc.Intensity           = l.value("intensity", 1.0f);
                lc.Range               = l.value("range", 10.0f);
                lc.SpotInnerAngle      = l.value("spot_inner", 12.5f);
                lc.SpotOuterAngle      = l.value("spot_outer", 17.5f);
                lc.CastShadows         = l.value("cast_shadows", false);
                lc.ShadowMapResolution = l.value("shadow_resolution", 1024);
                lc.ShadowBias          = l.value("shadow_bias", 0.005f);
                registry.AddComponent<LightComponent>(entity, lc);
            }

            // -- ScriptComponent --------------------------------------------
            if (ej.contains("script"))
            {
                const auto& s = ej["script"];
                ScriptComponent sc;
                sc.ScriptPath = s.value("path", std::string{});
                sc.IsEnabled  = s.value("enabled", true);
                registry.AddComponent<ScriptComponent>(entity, sc);
            }

            // -- AudioSource ------------------------------------------------
            if (ej.contains("audio"))
            {
                const auto& a = ej["audio"];
                AudioSource as;
                as.ClipPath     = a.value("clip_path", std::string{});
                as.Volume       = a.value("volume", 1.0f);
                as.Pitch        = a.value("pitch", 1.0f);
                as.Loop         = a.value("loop", false);
                as.PlayOnAwake  = a.value("play_on_awake", false);
                as.Spatial      = a.value("spatial", false);
                as.MinDistance  = a.value("min_distance", 1.0f);
                as.MaxDistance  = a.value("max_distance", 50.0f);
                as.RolloffFactor = a.value("rolloff", 1.0f);
                registry.AddComponent<AudioSource>(entity, as);
            }

            // -- Collider ---------------------------------------------------
            if (ej.contains("collider"))
            {
                const auto& c = ej["collider"];
                Collider col;
                col.Type        = static_cast<components::ColliderType>(
                    c.value("type", 0));
                col.Size        = c.contains("size")
                    ? DeserializeVec3(c["size"], math::Vec3{1.0f, 1.0f, 1.0f})
                    : math::Vec3{1.0f, 1.0f, 1.0f};
                col.Center      = c.contains("center")
                    ? DeserializeVec3(c["center"])
                    : math::Vec3{0.0f, 0.0f, 0.0f};
                col.IsTrigger   = c.value("is_trigger", false);
                col.Friction    = c.value("friction", 0.5f);
                col.Restitution = c.value("restitution", 0.0f);
                registry.AddComponent<Collider>(entity, col);
            }

            // -- RigidBody --------------------------------------------------
            if (ej.contains("rigid_body"))
            {
                const auto& r = ej["rigid_body"];
                RigidBody rb;
                rb.Type           = static_cast<components::BodyType>(
                    r.value("type", 0));
                rb.Mass           = r.value("mass", 1.0f);
                rb.LinearDrag     = r.value("linear_drag", 0.0f);
                rb.AngularDrag    = r.value("angular_drag", 0.05f);
                rb.UseGravity     = r.value("use_gravity", true);
                rb.IsKinematic    = r.value("is_kinematic", false);
                if (r.contains("linear_vel"))
                    rb.LinearVelocity = DeserializeVec3(r["linear_vel"]);
                if (r.contains("angular_vel"))
                    rb.AngularVelocity = DeserializeVec3(r["angular_vel"]);
                registry.AddComponent<RigidBody>(entity, rb);
            }
        }

        return scene;
    }

    // ========================================================================
    // File I/O
    // ========================================================================

    bool SceneSerializer::SaveToFile(const scene::Scene& scene,
                                     const std::string& filePath)
    {
        try
        {
            json data = Serialize(scene);
            std::ofstream ofs(filePath);
            if (!ofs.is_open())
            {
                ENGINE_LOG_ERROR("SceneSerializer — failed to open '{}' for writing", filePath);
                return false;
            }
            ofs << std::setw(4) << data << std::endl;
            ENGINE_LOG_INFO("SceneSerializer — scene '{}' saved to '{}'",
                            scene.GetName(), filePath);
            return true;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("SceneSerializer — save failed: {}", e.what());
            return false;
        }
    }

    std::unique_ptr<scene::Scene> SceneSerializer::LoadFromFile(const std::string& filePath)
    {
        try
        {
            std::ifstream ifs(filePath);
            if (!ifs.is_open())
            {
                ENGINE_LOG_ERROR("SceneSerializer — failed to open '{}' for reading", filePath);
                return nullptr;
            }

            json data;
            ifs >> data;

            auto scene = Deserialize(data);
            ENGINE_LOG_INFO("SceneSerializer — scene '{}' loaded from '{}'",
                            scene ? scene->GetName() : "null", filePath);
            return scene;
        }
        catch (const std::exception& e)
        {
            ENGINE_LOG_ERROR("SceneSerializer — load failed: {}", e.what());
            return nullptr;
        }
    }

    // ========================================================================
    // JsonSceneLoader
    // ========================================================================

    std::unique_ptr<scene::Scene> JsonSceneLoader::Load(const std::string& filePath)
    {
        return SceneSerializer::LoadFromFile(filePath);
    }

    bool JsonSceneLoader::SupportsExtension(const std::string& extension) const
    {
        return extension == ".scene" || extension == ".json"
            || extension == "scene"  || extension == "json";
    }

    // ========================================================================
    // JsonSceneSaver
    // ========================================================================

    bool JsonSceneSaver::Save(const scene::Scene& scene, const std::string& filePath)
    {
        return SceneSerializer::SaveToFile(scene, filePath);
    }

    std::string JsonSceneSaver::GetDefaultExtension() const
    {
        return ".scene";
    }

} // namespace engine::serialization
