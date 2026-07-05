// ============================================================================
// File: Tests/Source/World/WorldTests.cpp
// Tests for engine::world::World — scene management and active scene.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/World/World.h"

using namespace engine::world;

// ============================================================================
// Initialization
// ============================================================================

TEST(WorldTest, Initialize_CreatesDefaultScene)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    EXPECT_GE(world.SceneCount(), 1u);
    EXPECT_NE(world.GetActiveScene(), nullptr);

    world.Shutdown();
}

// ============================================================================
// Scene creation
// ============================================================================

TEST(WorldTest, CreateScene_IncrementsCount)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    // Initialize creates 1 default scene.
    ASSERT_EQ(world.SceneCount(), 1u);

    world.CreateScene("Scene 2");
    world.CreateScene("Scene 3");

    EXPECT_EQ(world.SceneCount(), 3u);

    world.Shutdown();
}

// ============================================================================
// Scene destruction
// ============================================================================

TEST(WorldTest, DestroyScene_DecrementsCount)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    auto& extra = world.CreateScene("Extra");
    core::UUID extraUUID = extra.GetUUID();

    ASSERT_EQ(world.SceneCount(), 2u);

    world.DestroyScene(extraUUID);

    EXPECT_EQ(world.SceneCount(), 1u);

    world.Shutdown();
}

// ============================================================================
// Active scene switching
// ============================================================================

TEST(WorldTest, SetActiveScene_SwitchesActive)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    auto& scene2 = world.CreateScene("Second");
    core::UUID scene2UUID = scene2.GetUUID();

    // The default "Main Scene" should be active initially.
    core::UUID mainUUID = world.GetActiveScene()->GetUUID();
    EXPECT_TRUE(world.GetActiveScene()->IsActive());

    // Switch to scene2.
    world.SetActiveScene(scene2UUID);
    EXPECT_EQ(world.GetActiveScene()->GetUUID(), scene2UUID);
    EXPECT_TRUE(world.GetActiveScene()->IsActive());

    // The old scene should no longer be active.
    scene::Scene* mainScene = world.GetScene(mainUUID);
    ASSERT_NE(mainScene, nullptr);
    EXPECT_FALSE(mainScene->IsActive());

    world.Shutdown();
}

// ============================================================================
// Destroying the active scene falls back
// ============================================================================

TEST(WorldTest, DestroyActiveScene_FallsBack)
{
    World world;
    ASSERT_TRUE(world.Initialize();

    // Create two additional scenes.
    auto& scene2 = world.CreateScene("Second");
    auto& scene3 = world.CreateScene("Third");

    core::UUID scene2UUID = scene2.GetUUID();
    core::UUID scene3UUID = scene3.GetUUID();

    // Make scene3 active.
    world.SetActiveScene(scene3UUID);
    ASSERT_EQ(world.GetActiveScene()->GetUUID(), scene3UUID);

    // Destroy the active scene. The active scene should fall back to another.
    world.DestroyScene(scene3UUID);

    // Active scene should still be set (to either Main Scene or scene2).
    // It should NOT be nullptr since other scenes remain.
    EXPECT_NE(world.GetActiveScene(), nullptr);

    world.Shutdown();
}

TEST(WorldTest, DestroyActiveScene_AllScenesFallbackToNullptr)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    // Only one scene (the default). Destroy it.
    core::UUID mainUUID = world.GetActiveScene()->GetUUID();
    world.DestroyScene(mainUUID);

    EXPECT_EQ(world.SceneCount(), 0u);
    EXPECT_EQ(world.GetActiveScene(), nullptr);

    world.Shutdown();
}

// ============================================================================
// Shutdown
// ============================================================================

TEST(WorldTest, Shutdown_ClearsAll)
{
    World world;
    ASSERT_TRUE(world.Initialize());

    world.CreateScene("Extra 1");
    world.CreateScene("Extra 2");
    ASSERT_EQ(world.SceneCount(), 3u);

    world.Shutdown();

    EXPECT_EQ(world.SceneCount(), 0u);
    EXPECT_EQ(world.GetActiveScene(), nullptr);
}