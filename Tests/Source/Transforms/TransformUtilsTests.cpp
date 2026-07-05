// ============================================================================
// File: Tests/Source/Transforms/TransformUtilsTests.cpp
// Tests for engine::transforms free functions — compose, decompose, local-to-world.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Transforms/TransformUtils.h"
#include "Engine/Math/Math.h"

using namespace engine::transforms;
using namespace engine::math;

// ============================================================================
// ComposeTRS round-trip
// ============================================================================

TEST(TransformUtilsTest, ComposeTRS_RoundTrip)
{
    Vec3 pos(1.0f, 2.0f, 3.0f);
    Quat rot = glm::angleAxis(glm::radians(45.0f), Vec3(0.0f, 1.0f, 0.0f));
    Vec3 scl(2.0f, 3.0f, 4.0f);

    Mat4 m = ComposeTRS(pos, rot, scl);

    Vec3 outPos = DecomposePosition(m);
    Quat outRot = DecomposeRotation(m);
    Vec3 outScl = DecomposeScale(m);

    // Position should round-trip exactly.
    EXPECT_NEAR(outPos.x, pos.x, 0.001f);
    EXPECT_NEAR(outPos.y, pos.y, 0.001f);
    EXPECT_NEAR(outPos.z, pos.z, 0.001f);

    // Rotation: compare dot product (quaternions q and -q represent the
    // same rotation).  A dot product close to 1 means they match.
    f32 dot = glm::abs(glm::dot(rot, outRot));
    EXPECT_NEAR(dot, 1.0f, 0.001f);

    // Scale should round-trip.
    EXPECT_NEAR(outScl.x, scl.x, 0.001f);
    EXPECT_NEAR(outScl.y, scl.y, 0.001f);
    EXPECT_NEAR(outScl.z, scl.z, 0.001f);
}

// ============================================================================
// LocalToWorld — simple translation concatenation
// ============================================================================

TEST(TransformUtilsTest, LocalToWorld_Simple)
{
    Mat4 parentWorld = ComposeTRS(Vec3(10.0f, 0.0f, 0.0f), Quat{}, Vec3(1.0f));
    Mat4 childLocal  = ComposeTRS(Vec3(5.0f, 0.0f, 0.0f),  Quat{}, Vec3(1.0f));

    Mat4 result = LocalToWorld(parentWorld, childLocal);
    Vec3 worldPos = DecomposePosition(result);

    EXPECT_NEAR(worldPos.x, 15.0f, 0.001f);
    EXPECT_NEAR(worldPos.y, 0.0f, 0.001f);
    EXPECT_NEAR(worldPos.z, 0.0f, 0.001f);
}

// ============================================================================
// DecomposePosition
// ============================================================================

TEST(TransformUtilsTest, DecomposePosition)
{
    Vec3 expected(7.5f, -3.2f, 100.0f);
    Mat4 m = ComposeTRS(expected, Quat{}, Vec3(1.0f));

    Vec3 pos = DecomposePosition(m);

    EXPECT_NEAR(pos.x, expected.x, 0.001f);
    EXPECT_NEAR(pos.y, expected.y, 0.001f);
    EXPECT_NEAR(pos.z, expected.z, 0.001f);
}

// ============================================================================
// DecomposeScale
// ============================================================================

TEST(TransformUtilsTest, DecomposeScale)
{
    Vec3 scale(2.0f, 3.0f, 4.0f);
    Mat4 m = ComposeTRS(Vec3(0.0f), Quat{}, scale);

    Vec3 outScale = DecomposeScale(m);

    EXPECT_NEAR(outScale.x, scale.x, 0.001f);
    EXPECT_NEAR(outScale.y, scale.y, 0.001f);
    EXPECT_NEAR(outScale.z, scale.z, 0.001f);
}