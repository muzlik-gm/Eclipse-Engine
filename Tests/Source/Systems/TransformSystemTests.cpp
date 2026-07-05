// ============================================================================
// File: Tests/Source/Systems/TransformSystemTests.cpp
// Tests for engine::systems::TransformSystem — world matrix propagation.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/ECS/Registry.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/HierarchyComponent.h"
#include "Engine/Systems/TransformSystem.h"
#include "Engine/Hierarchy/HierarchyUtils.h"
#include "Engine/Transforms/TransformUtils.h"

using namespace engine::ecs;
using namespace engine::components;
using namespace engine::systems;
using namespace engine::hierarchy;
using namespace engine::transforms;
using namespace engine::math;

// ============================================================================
// Helper: compare two Mat4 element-by-element
// ============================================================================

static void ExpectMat4Near(const Mat4& a, const Mat4& b, f32 eps = 0.001f)
{
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(a[col][row], b[col][row], eps)
                << "Mismatch at [" << col << "][" << row << "]";
        }
    }
}

// ============================================================================
// Tests
// ============================================================================

TEST(TransformSystemTest, SingleEntity_WorldMatrixEqualsLocal)
{
    Registry registry;
    TransformSystem system;
    system.OnAttach(registry);

    Entity entity = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(entity); // Root (Parent == Invalid)
    registry.AddComponent<TransformComponent>(entity);

    auto& transform = registry.GetComponent<TransformComponent>(entity);
    transform.Translation = Vec3(5.0f, 10.0f, 15.0f);

    system.Update(0.016);

    Mat4 expected = transform.GetLocalMatrix();
    ExpectMat4Near(transform.WorldMatrix, expected);
}

TEST(TransformSystemTest, ParentChild_Propagation)
{
    Registry registry;
    TransformSystem system;
    system.OnAttach(registry);

    Entity parent = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(parent);
    registry.AddComponent<TransformComponent>(parent);
    registry.GetComponent<TransformComponent>(parent).Translation = Vec3(10.0f, 0.0f, 0.0f);

    Entity child = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(child);
    registry.AddComponent<TransformComponent>(child);
    registry.GetComponent<TransformComponent>(child).Translation = Vec3(5.0f, 0.0f, 0.0f);

    SetParent(registry, child, parent);

    system.Update(0.016);

    // Child world position should be (15, 0, 0).
    Vec3 childWorldPos = DecomposePosition(
        registry.GetComponent<TransformComponent>(child).WorldMatrix);
    EXPECT_NEAR(childWorldPos.x, 15.0f, 0.001f);
    EXPECT_NEAR(childWorldPos.y, 0.0f, 0.001f);
    EXPECT_NEAR(childWorldPos.z, 0.0f, 0.001f);
}

TEST(TransformSystemTest, ThreeLevelChain)
{
    Registry registry;
    TransformSystem system;
    system.OnAttach(registry);

    // Grandparent at (10, 0, 0)
    Entity grandparent = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(grandparent);
    registry.AddComponent<TransformComponent>(grandparent);
    registry.GetComponent<TransformComponent>(grandparent).Translation = Vec3(10.0f, 0.0f, 0.0f);

    // Parent at (5, 0, 0)
    Entity parent = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(parent);
    registry.AddComponent<TransformComponent>(parent);
    registry.GetComponent<TransformComponent>(parent).Translation = Vec3(5.0f, 0.0f, 0.0f);

    // Child at (1, 0, 0)
    Entity child = registry.CreateEntity();
    registry.AddComponent<HierarchyComponent>(child);
    registry.AddComponent<TransformComponent>(child);
    registry.GetComponent<TransformComponent>(child).Translation = Vec3(1.0f, 0.0f, 0.0f);

    SetParent(registry, parent, grandparent);
    SetParent(registry, child, parent);

    system.Update(0.016);

    // Parent world = (10+5, 0, 0) = (15, 0, 0)
    Vec3 parentWorldPos = DecomposePosition(
        registry.GetComponent<TransformComponent>(parent).WorldMatrix);
    EXPECT_NEAR(parentWorldPos.x, 15.0f, 0.001f);
    EXPECT_NEAR(parentWorldPos.y, 0.0f, 0.001f);

    // Child world = (10+5+1, 0, 0) = (16, 0, 0)
    Vec3 childWorldPos = DecomposePosition(
        registry.GetComponent<TransformComponent>(child).WorldMatrix);
    EXPECT_NEAR(childWorldPos.x, 16.0f, 0.001f);
    EXPECT_NEAR(childWorldPos.y, 0.0f, 0.001f);
    EXPECT_NEAR(childWorldPos.z, 0.0f, 0.001f);
}

TEST(TransformSystemTest, NoHierarchyComponent_NoCrash)
{
    Registry registry;
    TransformSystem system;
    system.OnAttach(registry);

    // Entity with Transform but NO HierarchyComponent.
    Entity entity = registry.CreateEntity();
    registry.AddComponent<TransformComponent>(entity);
    registry.GetComponent<TransformComponent>(entity).Translation = Vec3(3.0f, 7.0f, 9.0f);

    // Should not crash — the system's view requires HierarchyComponent,
    // so this entity is simply ignored.
    EXPECT_NO_THROW(system.Update(0.016));
}