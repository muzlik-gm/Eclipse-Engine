// ============================================================================
// File: Tests/Source/Runtime/EngineStateTests.cpp
// Tests for the engine state machine — valid and invalid transitions.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Runtime/EngineState.h"

using namespace engine::runtime;

// ============================================================================
// EngineStateToString
// ============================================================================

TEST(EngineStateTest, ToStringAllStates)
{
    EXPECT_EQ(EngineStateToString(EngineState::Starting),     "Starting");
    EXPECT_EQ(EngineStateToString(EngineState::Initializing), "Initializing");
    EXPECT_EQ(EngineStateToString(EngineState::Running),      "Running");
    EXPECT_EQ(EngineStateToString(EngineState::Paused),       "Paused");
    EXPECT_EQ(EngineStateToString(EngineState::Stopping),     "Stopping");
    EXPECT_EQ(EngineStateToString(EngineState::Shutdown),     "Shutdown");
}

// ============================================================================
// Valid transitions
// ============================================================================

TEST(EngineStateTest, ValidTransitions)
{
    // Starting → Initializing
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Starting, EngineState::Initializing));

    // Starting → Stopping (early exit)
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Starting, EngineState::Stopping));

    // Initializing → Running
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Initializing, EngineState::Running));

    // Initializing → Stopping (init failure)
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Initializing, EngineState::Stopping));

    // Running → Paused
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Running, EngineState::Paused));

    // Running → Stopping
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Running, EngineState::Stopping));

    // Paused → Running
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Paused, EngineState::Running));

    // Paused → Stopping
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Paused, EngineState::Stopping));

    // Stopping → Shutdown
    EXPECT_TRUE(IsEngineStateTransitionValid(EngineState::Stopping, EngineState::Shutdown));
}

// ============================================================================
// Invalid transitions
// ============================================================================

TEST(EngineStateTest, InvalidTransitions)
{
    // Same state is never valid.
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Starting, EngineState::Starting));
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Running, EngineState::Running));

    // Cannot go backwards.
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Running, EngineState::Starting));
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Shutdown, EngineState::Running));
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Shutdown, EngineState::Starting));

    // Cannot go from Shutdown to anything.
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Shutdown, EngineState::Stopping));
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Shutdown, EngineState::Initializing));

    // Cannot jump to Running directly from Starting.
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Starting, EngineState::Running));

    // Cannot jump to Paused from non-Running states.
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Starting, EngineState::Paused));
    EXPECT_FALSE(IsEngineStateTransitionValid(EngineState::Initializing, EngineState::Paused));
}