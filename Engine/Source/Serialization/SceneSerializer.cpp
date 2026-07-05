// ============================================================================
// File: Engine/Source/Serialization/SceneSerializer.cpp
// SceneSerializer implementation — JSON serialize / deserialize.
// ============================================================================

#include "Engine/Serialization/SceneSerializer.h"

#include <nlohmann/json.hpp>

#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"

namespace engine::serialization {

    using engine::core::f32;
    using engine::core::u32;

    // ========================================================================
    // Helper — entity handle to u32 index for serialization
    // ========================================================================

    static u32 EntityToIndex(ecs::Entity entity)
    {
        return ecs::IsValid(entity) ? ecs::GetIndex(entity) : std::numeric_limits<u32>::max();
    }

    static ecs::Entity IndexToEntity(u32 index)
    {
        return (index != std::numeric_limits<u32>::max())
            ? ecs::Entity{static_cast<entt::entity>(index)}
            : entt::null;
    }

    // ========================================================================
    // Serialize
    // ========================================================================

    nlohmann::json SceneSerializer::Serialize(const scene::Scene& scene)
    {
        nlohmann::json root;
        root["name"] = scene.GetName();
        root["uuid"] = scene.GetUUID().ToString();

        auto& registry = const_cast<scene::Scene&>(scene).GetRegistry();
        auto allView = registry.GetInner().view<components::TagComponent>();

        nlohmann::json entitiesJson = nlohmann::json::array();

        for (auto entityHandle : allView)
        {
            ecs::Entity entity{entityHandle};
            nlohmann::json entityJson;

            // Entity index (used as a stable reference during serialization).
            entityJson["index"] = ecs::GetIndex(entity);

            // -- TagComponent ---------------------------------------------------
            if (registry.HasComponent<components::TagComponent>(entity))
            {
                const auto& tag = registry.GetComponent<components::TagComponent>(entity);
                entityJson["tag"] = tag.Tag;
            }

            // -- IDComponent ----------------------------------------------------
            if (registry.HasComponent<components::IDComponent>(entity))
            {
                const auto& id = registry.GetComponent<components::IDComponent>(entity);
                entityJson["uuid"] = id.ID.ToString();
            }

            // -- TransformComponent ---------------------------------------------
            if (registry.HasComponent<components::TransformComponent>(entity))
            {
                const auto& tc = registry.GetComponent<components::TransformComponent>(entity);

                nlohmann::json transform;
                transform["translation"] = {tc.Translation.x, tc.Translation.y, tc.Translation.z};
                transform["rotation"]    = {tc.Rotation.x, tc.Rotation.y, tc.Rotation.z, tc.Rotation.w};
                transform["scale"]       = {tc.Scale.x, tc.Scale.y, tc.Scale.z};

                entityJson["transform"] = transform;
            }

            // -- HierarchyComponent ---------------------------------------------
            if (registry.HasComponent<components::HierarchyComponent>(entity))
            {
                const auto& hc = registry.GetComponent<components::HierarchyComponent>(entity);

                nlohmann::json hierarchy;
                hierarchy["parent"]        = EntityToIndex(hc.Parent);
                hierarchy["first_child"]   = EntityToIndex(hc.FirstChild);
                hierarchy["next_sibling"]  = EntityToIndex(hc.NextSibling);
                hierarchy["prev_sibling"]  = EntityToIndex(hc.PrevSibling);
                hierarchy["child_count"]   = hc.ChildCount;

                entityJson["hierarchy"] = hierarchy;
            }

            entitiesJson.push_back(entityJson);
        }

        root["entities"] = entitiesJson;

        return root;
    }

    // ========================================================================
    // Deserialize
    // ========================================================================

    std::unique_ptr<scene::Scene> SceneSerializer::Deserialize(const nlohmann::json& data)
    {
        std::string name = data.value("name", "Untitled Scene");
        auto scene = std::make_unique<scene::Scene>(name);

        auto& registry = scene->GetRegistry();

        if (!data.contains("entities") || !data["entities"].is_array())
            return scene;

        // First pass: create all entities and restore TagComponent / IDComponent.
        std::unordered_map<u32, ecs::Entity> indexToEntity;

        for (const auto& entityJson : data["entities"])
        {
            u32 index = entityJson.value("index", 0u);

            std::string tag = entityJson.value("tag", "Entity");
            auto handle = scene->CreateEntity(tag);
            ecs::Entity entity = handle.GetId();

            indexToEntity[index] = entity;

            // Restore IDComponent if present.
            if (entityJson.contains("uuid"))
            {
                auto uuid = core::UUID::FromString(entityJson["uuid"].get<std::string>());
                registry.GetComponent<components::IDComponent>(entity).ID = uuid;
            }
        }

        // Second pass: restore TransformComponent and HierarchyComponent.
        for (const auto& entityJson : data["entities"])
        {
            u32 index = entityJson.value("index", 0u);
            auto it = indexToEntity.find(index);
            if (it == indexToEntity.end())
                continue;

            ecs::Entity entity = it->second;

            // -- TransformComponent ---------------------------------------------
            if (entityJson.contains("transform"))
            {
                const auto& t = entityJson["transform"];

                math::Vec3 translation{0.0f, 0.0f, 0.0f};
                math::Quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
                math::Vec3 scale{1.0f, 1.0f, 1.0f};

                if (t.contains("translation") && t["translation"].is_array() && t["translation"].size() == 3)
                {
                    translation = math::Vec3(
                        t["translation"][0].get<f32>(),
                        t["translation"][1].get<f32>(),
                        t["translation"][2].get<f32>()
                    );
                }

                if (t.contains("rotation") && t["rotation"].is_array() && t["rotation"].size() == 4)
                {
                    rotation = math::Quat(
                        t["rotation"][0].get<f32>(),
                        t["rotation"][1].get<f32>(),
                        t["rotation"][2].get<f32>(),
                        t["rotation"][3].get<f32>()
                    );
                }

                if (t.contains("scale") && t["scale"].is_array() && t["scale"].size() == 3)
                {
                    scale = math::Vec3(
                        t["scale"][0].get<f32>(),
                        t["scale"][1].get<f32>(),
                        t["scale"][2].get<f32>()
                    );
                }

                registry.AddComponent<components::TransformComponent>(
                    entity, components::TransformComponent{translation, rotation, scale});
            }

            // -- HierarchyComponent ---------------------------------------------
            if (entityJson.contains("hierarchy"))
            {
                const auto& h = entityJson["hierarchy"];

                components::HierarchyComponent hc;
                hc.Parent       = IndexToEntity(h.value("parent", std::numeric_limits<u32>::max()));
                hc.FirstChild   = IndexToEntity(h.value("first_child", std::numeric_limits<u32>::max()));
                hc.NextSibling  = IndexToEntity(h.value("next_sibling", std::numeric_limits<u32>::max()));
                hc.PrevSibling  = IndexToEntity(h.value("prev_sibling", std::numeric_limits<u32>::max()));
                hc.ChildCount   = h.value("child_count", 0u);

                registry.AddComponent<components::HierarchyComponent>(entity, hc);
            }
        }

        return scene;
    }

} // namespace engine::serialization