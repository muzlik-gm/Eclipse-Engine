// ============================================================================
// File: Tests/Source/Components/TransformComponentTests.cpp
// Tests for engine::components::TransformComponent — defaults and matrices.
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Components/TransformComponent.h"
#include "Engine/Math/Math.h"

using namespace engine::components;
using namespace engine::math;

// ============================================================================
// Default values
// ============================================================================

TEST(TransformComponentTest, DefaultValues)
{
    TransformComponent tc;

    EXPECT_NEAR(tc.Translation.x, 0.0f, 0.001f);
    EXPECT_NEAR(tc.Translation.y, 0.0f, 0.001f);
    EXPECT_NEAR(tc.Translation.z, 0.0f, 0.001f);

    // Identity quaternion: w=1, x=y=z=0.
    EXPECT_NEAR(tc.Rotation.w, 1.0f, 0.001f);
    EXPECT_NEAR(tc.Rotation.x, 0.0f, 0.001f);
    EXPECT_NEAR(tc.Rotation.y, 0.0f, 0.001f);
    EXPECT_NEAR(tc.Rotation.z, 0.0f, 0.001f);

    EXPECT_NEAR(tc.Scale.x, 1.0f, 0.001f);
    EXPECT_NEAR(tc.Scale.y, 1.0f, 0.001f);
    EXPECT_NEAR(tc.Scale.z, 1.0f, 0.001f);

    EXPECT_TRUE(tc.WorldDirty);
}

// ============================================================================
// Local matrix
// ============================================================================

TEST(TransformComponentTest, GetLocalMatrix_IdentityForDefaults)
{
    TransformComponent tc;
    Mat4 m = tc.GetLocalMatrix();

    // Identity matrix: diagonal is 1, everything else is 0.
    Mat4 identity(1.0f);

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(m[col][row], identity[col][row], 0.001f)
                << "Mismatch at column " << col << " row " << row;
        }
    }
}

TEST(TransformComponentTest, GetLocalMatrix_Translation)
{
    TransformComponent tc;
    tc.Translation = Vec3(1.0f, 2.0f, 3.0f);

    Mat4 m = tc.GetLocalMatrix();

    // Translation is stored in column 3 (m[3]).
    EXPECT_NEAR(m[3][0], 1.0f, 0.001f);
    EXPECT_NEAR(m[3][1], 2.0f, 0.001f);
    EXPECT_NEAR(m[3][2], 3.0f, 0.001f);
    EXPECT_NEAR(m[3][3], 1.0f, 0.001f);
}

TEST(TransformComponentTest, GetLocalMatrix_Scale)
{
    TransformComponent tc;
    tc.Scale = Vec3(2.0f, 3.0f, 4.0f);

    Mat4 m = tc.GetLocalMatrix();

    // Scale affects the diagonal elements of columns 0, 1, 2.
    EXPECT_NEAR(m[0][0], 2.0f, 0.001f);
    EXPECT_NEAR(m[1][1], 3.0f, 0.001f);
    EXPECT_NEAR(m[2][2], 4.0f, 0.001f);
}

// ============================================================================
// WorldDirty flag
// ============================================================================

TEST(TransformComponentTest, WorldDirtyFlag)
{
    TransformComponent tc;
    EXPECT_TRUE(tc.WorldDirty);

    tc.WorldDirty = false;
    EXPECT_FALSE(tc.WorldDirty);
}