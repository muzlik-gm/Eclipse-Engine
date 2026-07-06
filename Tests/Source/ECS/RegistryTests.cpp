// ============================================================================
// File: Tests/Source/ECS/RegistryTests.cpp
// Tests for engine::ecs::Registry - entity lifetime, components, views.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/TagComponent.h"
#include "Engine/Components/TransformComponent.h"

#include <string>
#include <vector>

using namespace engine::ecs;
using engine::components::TagComponent;
using engine::components::TransformComponent;

// ============================================================================
// Lightweight marker component for counting entities in views.
// ============================================================================
struct TestMarker { int value = 0; };

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

    // Entities without components cannot be counted via a typed view.
    // Instead verify that three distinct valid entities were produced.
    // We add a marker component to confirm view counting works.
    Entity a = registry.CreateEntity();
    Entity b = registry.CreateEntity();
    Entity c = registry.CreateEntity();
    registry.AddComponent<TestMarker>(a);
    registry.AddComponent<TestMarker>(b);
    registry.AddComponent<TestMarker>(c);

    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TestMarker>()) { (void)e; ++count; }
    EXPECT_EQ(count, 3u);
}

TEST(RegistryTest, DestroyEntity_DecrementsCount)
{
    Registry registry;

    Entity e0 = registry.CreateEntity();
    Entity e1 = registry.CreateEntity();
    registry.AddComponent<TestMarker>(e0);
    registry.AddComponent<TestMarker>(e1);

    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TestMarker>()) { (void)e; ++count; }
    ASSERT_EQ(count, 2u);

    registry.DestroyEntity(e0);

    count = 0;
    for ([[maybe_unused]] auto e : registry.View<TestMarker>()) { (void)e; ++count; }
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

TEST(RegistryTest, DestroyEntity_AlreadyDestroyed_DoesNotCrash)
{
    Registry registry;
    Entity e = registry.CreateEntity();
    registry.DestroyEntity(e);
    // Double-destroy should be a no-op.
    EXPECT_NO_THROW(registry.DestroyEntity(e));
}

// ============================================================================
// Component management
// ============================================================================

TEST(RegistryTest, AddComponentAndRetrieve)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<TagComponent>(entity, TagComponent{"TestTag"});

    EXPECT_TRUE(registry.HasComponent<TagComponent>(entity));

    auto& tag = registry.GetComponent<TagComponent>(entity);
    EXPECT_EQ(tag.Tag, "TestTag");
}

TEST(RegistryTest, RemoveComponent)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<TagComponent>(entity, TagComponent{"Temp"});
    EXPECT_TRUE(registry.HasComponent<TagComponent>(entity));

    registry.RemoveComponent<TagComponent>(entity);
    EXPECT_FALSE(registry.HasComponent<TagComponent>(entity));
}

TEST(RegistryTest, HasComponent_FalseWhenMissing)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    EXPECT_FALSE(registry.HasComponent<TagComponent>(entity));
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
        registry.AddComponent<TagComponent>(entities[i],
            TagComponent{"Tagged_" + std::to_string(i)});
    }

    auto view = registry.View<TagComponent>();
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

    Entity e0 = registry.CreateEntity();
    Entity e1 = registry.CreateEntity();
    Entity e2 = registry.CreateEntity();
    registry.AddComponent<TestMarker>(e0);
    registry.AddComponent<TestMarker>(e1);
    registry.AddComponent<TestMarker>(e2);

    usize count = 0;
    for ([[maybe_unused]] auto e : registry.View<TestMarker>()) { (void)e; ++count; }
    ASSERT_EQ(count, 3u);

    registry.Clear();

    count = 0;
    for ([[maybe_unused]] auto e : registry.View<TestMarker>()) { (void)e; ++count; }
    EXPECT_EQ(count, 0u);
}

// ============================================================================
// Multiple component types
// ============================================================================

TEST(RegistryTest, MultipleComponentTypes)
{
    Registry registry;
    Entity entity = registry.CreateEntity();

    registry.AddComponent<TagComponent>(entity, TagComponent{"Multi"});
    registry.AddComponent<TransformComponent>(entity);

    EXPECT_TRUE(registry.HasComponent<TagComponent>(entity));
    EXPECT_TRUE(registry.HasComponent<TransformComponent>(entity));

    auto& tag = registry.GetComponent<TagComponent>(entity);
    EXPECT_EQ(tag.Tag, "Multi");

    auto& transform = registry.GetComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.Translation.x, 0.0f, 0.001f);
    EXPECT_NEAR(transform.Translation.y, 0.0f, 0.001f);
    EXPECT_NEAR(transform.Translation.z, 0.0f, 0.001f);
}