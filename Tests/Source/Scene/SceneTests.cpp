// ============================================================================
// File: Tests/Source/Scene/SceneTests.cpp
// Tests for engine::scene::Scene - construction, entities, systems.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Scene/Scene.h"
#include "Engine/Systems/ISystem.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Entities/EntityHandle.h"

#include <memory>
#include <string>
#include <vector>

using engine::core::f64;
using engine::ecs::Entity;
using engine::scene::Scene;

// ============================================================================
// Mock system for verifying lifecycle callbacks.
// Uses shared_ptr counters so the copy made by Scene::AddSystem still
// shares state with the original test-local object.
// ============================================================================

class MockSystem final : public engine::systems::ISystem
{
public:
    std::shared_ptr<int>  updateCount     = std::make_shared<int>(0);
    std::shared_ptr<f64>  lastDt          = std::make_shared<f64>(0.0);
    std::shared_ptr<bool> onAttachCalled  = std::make_shared<bool>(false);

    std::string_view GetName() const noexcept override { return "MockSystem"; }

    void OnAttach([[maybe_unused]] engine::ecs::Registry& registry) override
    {
        *onAttachCalled = true;
    }

    void Update(f64 dt) override
    {
        ++(*updateCount);
        *lastDt = dt;
    }
};

// ============================================================================
// Construction
// ============================================================================

TEST(SceneTest, Construction_HasValidUUID)
{
    Scene scene("TestScene");
    EXPECT_TRUE(scene.GetUUID().IsValid());
}

TEST(SceneTest, Construction_HasName)
{
    Scene scene("MyScene");
    EXPECT_EQ(scene.GetName(), "MyScene");
}

// ============================================================================
// Entity lifetime
// ============================================================================

TEST(SceneTest, CreateEntity_ReturnsValidHandle)
{
    Scene scene("TestScene");
    auto handle = scene.CreateEntity("TestEnt");
    EXPECT_TRUE(handle.IsValid());
}

TEST(SceneTest, CreateEntity_HasTagComponent)
{
    Scene scene("TestScene");
    auto handle = scene.CreateEntity("Tagged");
    EXPECT_TRUE(handle.HasComponent<engine::components::TagComponent>());
    auto& tag = handle.GetComponent<engine::components::TagComponent>();
    EXPECT_EQ(tag.Tag, "Tagged");
}

TEST(SceneTest, CreateEntity_HasIDComponent)
{
    Scene scene("TestScene");
    auto handle = scene.CreateEntity("IDEnt");
    EXPECT_TRUE(handle.HasComponent<engine::components::IDComponent>());
    auto& id = handle.GetComponent<engine::components::IDComponent>();
    EXPECT_TRUE(id.ID.IsValid());
}

TEST(SceneTest, CreateEntity_IncrementsCount)
{
    Scene scene("TestScene");
    scene.CreateEntity("Ent1");
    scene.CreateEntity("Ent2");
    EXPECT_EQ(scene.EntityCount(), 2u);
}

TEST(SceneTest, DestroyEntity_DecrementsCount)
{
    Scene scene("TestScene");
    auto handle = scene.CreateEntity("ToRemove");
    Entity entityId = handle.GetId();
    EXPECT_EQ(scene.EntityCount(), 1u);

    scene.DestroyEntity(entityId);
    EXPECT_EQ(scene.EntityCount(), 0u);
}

// ============================================================================
// System management
// ============================================================================

TEST(SceneTest, AddSystem_CallsOnAttach)
{
    Scene scene("TestScene");
    MockSystem system;
    scene.AddSystem<MockSystem>(system);
    EXPECT_TRUE(*system.onAttachCalled);
}

TEST(SceneTest, OnUpdate_CallsSystemUpdate)
{
    Scene scene("TestScene");
    MockSystem system;
    scene.AddSystem<MockSystem>(system);
    scene.OnUpdate(0.033);
    EXPECT_EQ(*system.updateCount, 1);
    EXPECT_NEAR(*system.lastDt, 0.033, 0.001);
}

TEST(SceneTest, MultipleSystems_AllUpdated)
{
    Scene scene("TestScene");

    MockSystem sysA;
    MockSystem sysB;
    scene.AddSystem<MockSystem>(sysA);
    scene.AddSystem<MockSystem>(sysB);

    scene.OnUpdate(0.016);

    EXPECT_EQ(*sysA.updateCount, 1);
    EXPECT_EQ(*sysB.updateCount, 1);
}

TEST(SceneTest, OnFixedUpdate_CallsSystemFixedUpdate)
{
    Scene scene("TestScene");

    auto fixedCount = std::make_shared<int>(0);

    class FixedMockSystem final : public engine::systems::ISystem
    {
    public:
        std::shared_ptr<int> count;
        std::string_view GetName() const noexcept override { return "FixedMockSystem"; }
        void FixedUpdate(f64 dt) override { (void)dt; ++(*count); }
    };

    FixedMockSystem fixedSys;
    fixedSys.count = fixedCount;
    scene.AddSystem<FixedMockSystem>(fixedSys);

    scene.OnFixedUpdate(0.02);
    scene.OnFixedUpdate(0.02);

    EXPECT_EQ(*fixedCount, 2);
}