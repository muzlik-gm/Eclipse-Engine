// ============================================================================
// File: Tests/Source/Scene/SceneTests.cpp
// Tests for engine::scene::Scene — construction, entities, systems.
// ============================================================================

#include <gtest/gtest.h>

#include <gtest/gtest.h>

#include "Engine/Scene/Scene.h"
#include "Engine/Systems/ISystem.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/IDComponent.h"
#include "Engine/Entities/EntityHandle.h"

#include <string>
#include <vector>

// ============================================================================
// Mock system for verifying lifecycle callbacks.
// ============================================================================

class MockSystem final : public systems::ISystem
{
public:
    std::string_view GetName() const noexcept override { return "MockSystem"; }

    int updateCount     = 0;
    f64  lastDt          = 0.0;
    bool onAttachCalled = false;

    void OnAttach(ecs::Registry& registry) override { onAttachCalled = true; }
    void Update(f64 dt) override { updateCount++; lastDt = dt; }
};

// ============================================================================
// Construction
// ============================================================================

TEST(SceneTest, Construction_HasValidUUID)
{
    scene::Scene scene("TestScene");
    EXPECT_TRUE(scene.GetUUID().IsValid());
}

TEST(SceneTest, Construction_HasName)
{
    scene::Scene scene("MyScene");
    EXPECT_EQ(scene.GetName(), "MyScene");
}

// ============================================================================
// Entity lifetime
// ============================================================================

TEST(SceneTest, CreateEntity_ReturnsValidHandle)
{
    scene::Scene scene("TestScene");
    auto handle = scene.CreateEntity("TestEnt");
    EXPECT_TRUE(handle.IsValid());
}

TEST(SceneTest, CreateEntity_HasTagComponent)
{
    scene::Scene scene("TestScene");
    auto handle = scene.CreateEntity("Tagged");
    EXPECT_TRUE(handle.HasComponent<engine::components::TagComponent>());
    auto& tag = handle.GetComponent<engine::components::TagComponent>();
    EXPECT_EQ(tag.Tag, "Tagged");
}

TEST(SceneTest, CreateEntity_HasIDComponent)
{
    scene::Scene scene("TestScene");
    auto handle = scene.CreateEntity("IDEnt");
    EXPECT_TRUE(handle.HasComponent<engine::components::IDComponent>());
    auto& id = handle.GetComponent<engine::components::IDComponent>();
    EXPECT_TRUE(id.ID.IsValid());
}

TEST(SceneTest, DestroyEntity_RemovesFromRegistry)
{
    scene::Scene scene("TestScene");
    scene.CreateEntity("ToRemove");
    EXPECT_EQ(scene.EntityCount(), 1u);
}

// ============================================================================
// System management
// ============================================================================

TEST(SceneTest, AddSystem_CallsOnAttach)
{
    scene::Scene scene("TestScene");
    MockSystem system;
    scene.AddSystem<MockSystem>(system);
    EXPECT_TRUE(system.onAttachCalled);
}

TEST(SceneTest, OnUpdate_CallsSystemUpdate)
{
    scene::Scene scene("TestScene");
    MockSystem system;
    scene.AddSystem<MockSystem>(system);
    scene.OnUpdate(0.033);
    EXPECT_EQ(system.updateCount, 1);
    EXPECT_NEAR(system.lastDt, 0.033, 0.001f);
}

TEST(SceneTest, MultipleSystems_UpdatedInOrder)
{
    scene::Scene scene("TestScene");

    std::vector<int> order;
    auto orderSystem = [&](const std::string& name, auto& sys) {
        sys.GetName();
        order.push_back(static_cast<int>(order.size()) + 1));
    };

    scene.AddSystem<MockSystem>(std::make_unique<OrderSystem>("A", orderSystem));
    scene.AddSystem<MockSystem>(std::make_unique<OrderSystem>("B", orderSystem));
    scene.OnUpdate(0.016f);

    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
}