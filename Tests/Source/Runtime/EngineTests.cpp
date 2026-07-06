// ============================================================================
// File: Tests/Source/Runtime/EngineTests.cpp
// Tests for the Engine class — initialization, shutdown, lifecycle,
// state transitions, and the main loop.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Core/Log.h"
#include "Engine/Core/Types.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Runtime/ISubsystem.h"

#include <atomic>
#include <thread>

// ============================================================================
// Test subsystem that tracks calls and can request stop.
// ============================================================================

class StopSubsystem final : public engine::runtime::ISubsystem
{
public:
    explicit StopSubsystem(engine::runtime::Engine* engine, engine::core::u64 stopAfterFrames)
        : m_engine(engine)
        , m_stopAfter(stopAfterFrames)
    {}

    [[nodiscard]] std::string_view GetName() const noexcept override { return "Stopper"; }
    bool Initialize() override { return true; }
    void Shutdown() override {}
    void Update(engine::core::f64) override
    {
        ++m_updateCount;
        if (m_updateCount >= m_stopAfter)
        {
            m_engine->RequestStop();
        }
    }
    void FixedUpdate(engine::core::f64) override {}
    void LateUpdate(engine::core::f64) override {}

    engine::runtime::Engine*  m_engine = nullptr;
    engine::core::u64         m_stopAfter   = 3;
    engine::core::u64         m_updateCount = 0;
};

// Helper macros to avoid namespace issues in test code.
#define TEST_LOG_INIT(...)  engine::core::Log::Initialize(__VA_ARGS__)
#define TEST_LOG_SHUTDOWN() engine::core::Log::Shutdown()

using namespace engine::runtime;

// ============================================================================
// Initialization and shutdown
// ============================================================================

TEST(EngineTest, InitializeAndShutdown)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    EngineConfig config;
    config.printBuildInfo = false;

    ASSERT_TRUE(engine.Initialize(config));
    EXPECT_TRUE(engine.IsRunning());
    EXPECT_EQ(engine.GetState(), EngineState::Running);

    engine.Shutdown();
    EXPECT_EQ(engine.GetState(), EngineState::Shutdown);
    EXPECT_FALSE(engine.IsRunning());

    TEST_LOG_SHUTDOWN();
}

TEST(EngineTest, DoubleInitializeIsIgnored)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    EngineConfig config;
    config.printBuildInfo = false;

    ASSERT_TRUE(engine.Initialize(config));
    ASSERT_TRUE(engine.Initialize(config)); // Second call is a no-op.

    engine.Shutdown();
    TEST_LOG_SHUTDOWN();
}

TEST(EngineTest, RunBeforeInitializeFails)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    const engine::core::i32 result = engine.Run();
    EXPECT_NE(result, 0);

    TEST_LOG_SHUTDOWN();
}

// ============================================================================
// Main loop
// ============================================================================

TEST(EngineTest, MainLoopRunsAndStops)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    auto stopper = std::make_unique<StopSubsystem>(&engine, 2);
    engine.GetSubsystemManager().Register(std::move(stopper));

    EngineConfig config;
    config.printBuildInfo = false;

    ASSERT_TRUE(engine.Initialize(config));

    // Run in a separate thread to avoid blocking the test.
    std::atomic<engine::core::i32> exitCode{-1};
    std::thread loopThread([&engine, &exitCode]() {
        exitCode = engine.Run();
    });

    loopThread.join();

    EXPECT_EQ(exitCode.load(), 0);
    EXPECT_GE(engine.GetFrameStats().FrameCount(), 2u);

    engine.Shutdown();
    TEST_LOG_SHUTDOWN();
}

// ============================================================================
// State transitions
// ============================================================================

TEST(EngineTest, StateProgression)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    EXPECT_EQ(engine.GetState(), EngineState::Starting);

    EngineConfig config;
    config.printBuildInfo = false;

    ASSERT_TRUE(engine.Initialize(config));
    EXPECT_EQ(engine.GetState(), EngineState::Running);

    engine.Shutdown();
    EXPECT_EQ(engine.GetState(), EngineState::Shutdown);

    TEST_LOG_SHUTDOWN();
}

// ============================================================================
// Subsystem ownership
// ============================================================================

TEST(EngineTest, EngineOwnsSubsystemManager)
{
    TEST_LOG_INIT("EngineTest");

    Engine engine;
    EngineConfig config;
    config.printBuildInfo = false;

    engine.GetSubsystemManager().Register(
        std::make_unique<StopSubsystem>(&engine, 1));

    ASSERT_TRUE(engine.Initialize(config));
    // Engine::Initialize() auto-registers the WorldManager subsystem,
    // so the count includes both the test's Stopper and WorldManager.
    EXPECT_GE(engine.GetSubsystemManager().Count(), 1u);
    EXPECT_TRUE(engine.GetSubsystemManager().IsInitialized("Stopper"));
    EXPECT_TRUE(engine.GetSubsystemManager().IsInitialized("WorldManager"));

    engine.Shutdown();
    TEST_LOG_SHUTDOWN();
}