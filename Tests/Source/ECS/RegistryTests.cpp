// ============================================================================
// File: Tests/Source/ECS/RegistryTests.cpp
// Tests for engine::ecs::Registry — entity lifetime, components, views.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/TransformComponent.h"

using namespace engine::ecs;
using engine::components::TagComponent;
using engine::components::TransformComponent;

// ============================================================================
// Entity lifetime
// ============================================================================

TEST(RegistryTest, CreateEntity_ReturnsValidEntity)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    EXPECT_TRUE(IsValid(entity));
    EXPECT_TRUE(registry.IsValid(entity));
}

TEST(RegistryTest, CreateEntity_IncrementsCount)
{
    Registry registry;

    registry.CreateEntity();
    registry.CreateEntity();
    registry.CreateEntity();

    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TagComponent>()) { (void)e; ++count; }
    EXPECT_EQ(count, 3u);
}

TEST(RegistryTest, DestroyEntity_DecrementsCount)
{
    Registry registry;

    Entity e0 = registry.CreateEntity();
    Entity e1 = registry.CreateEntity();

    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TagComponent>()) { (void)e; ++count; }
    ASSERT_EQ(count, 2u);

    registry.DestroyEntity(e0);

    count = 0;
    for ([[maybe_unused]] auto e : registry.View<TagComponent>()) { (void)e; ++count; }
    EXPECT_EQ(count, 1u);
    EXPECT_FALSE(registry.IsValid(e0));
    EXPECT_TRUE(registry.IsValid(e1));
}

TEST(RegistryTest, DestroyEntity_InvalidEntity_DoesNotCrash)
{
    Registry registry;
    // Destroying an invalid (null) entity should not crash.
    EXPECT_NO_THROW(registry.DestroyEntity(Invalid));
}

// ============================================================================
// Component management
// ============================================================================

TEST(RegistryTest, AddComponentAndRetrieve)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<engine::components::TagComponent>(entity, engine::components::TagComponent{"TestTag"});

    EXPECT_TRUE(registry.HasComponent<engine::components::TagComponent>(entity));

    auto& tag = registry.GetComponent<engine::components::TagComponent>(entity);
    EXPECT_EQ(tag.Tag, "TestTag");
}

TEST(RegistryTest, RemoveComponent)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<engine::components::TagComponent>(entity, engine::components::TagComponent{"Temp"});
    EXPECT_TRUE(registry.HasComponent<engine::components::TagComponent>(entity));

    registry.RemoveComponent<engine::components::TagComponent>(entity);
    EXPECT_FALSE(registry.HasComponent<engine::components::TagComponent>(entity));
}

TEST(RegistryTest, HasComponent_FalseWhenMissing)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    EXPECT_FALSE(registry.HasComponent<engine::components::TagComponent>(entity));
}

// ============================================================================
// Views
// ============================================================================

TEST(RegistryTest, View_FiltersCorrectly)
{
    Registry registry;

    // Create 10 entities.
    std::vector<Entity> entities;
    for (int i = 0; i < 10; ++i)
    {
        entities.push_back(registry.CreateEntity());
    }

    // Add TagComponent to the first 5.
    for (int i = 0; i < 5; ++i)
    {
        registry.AddComponent<engine::components::TagComponent>(entities[i],
            engine::components::TagComponent{"Tagged_" + std::to_string(i)});
    }

    auto view = registry.View<engine::components::TagComponent>();
    usize count = 0;
    for ([[maybe_unused]] auto entityHandle : view)
    {
        ++count;
    }

    EXPECT_EQ(count, 5u);
}

// ============================================================================
// Bulk operations
// ============================================================================

TEST(RegistryTest, Clear_RemovesAllEntities)
{
    Registry registry;

    registry.CreateEntity();
    registry.CreateEntity();
    registry.CreateEntity();
    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TagComponent>()) { (void)e; ++count; }
    ASSERT_EQ(count, 3u);

    registry.Clear();

    count = 0;
    for ([[maybe_unused]] auto e : registry.View<TagComponent>()) { (void)e; ++count; }
    EXPECT_EQ(count, 0u);
}

// ============================================================================
// Multiple component types
// ============================================================================

TEST(RegistryTest, MultipleComponentTypes)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<engine::components::TagComponent>(entity, engine::components::TagComponent{"Multi"});
    registry.AddComponent<engine::components::TransformComponent>(entity);

    EXPECT_TRUE(registry.HasComponent<engine::components::TagComponent>(entity));
    EXPECT_TRUE(registry.HasComponent<engine::components::TransformComponent>(entity));

    auto& tag = registry.GetComponent<engine::components::TagComponent>(entity);
    EXPECT_EQ(tag.Tag, "Multi");

    auto& transform = registry.GetComponent<engine::components::TransformComponent>(entity);
    EXPECT_NEAR(transform.Translation.x, 0.0f, 0.001f);
    EXPECT_NEAR(transform.Translation.y, 0.0f, 0.001f);
    EXPECT_NEAR(transform.Translation.z, 0.0f, 0.001f);
}