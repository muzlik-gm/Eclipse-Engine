// ============================================================================
// File: Tests/Source/Application/ApplicationTests.cpp
// Tests for the Application class — construction, initialization,
// command-line parsing, and lifecycle.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Core/Log.h"
#include "Engine/Core/Types.h"
#include "Engine/Application/Application.h"
#include "Engine/Runtime/Engine.h"
#include "Engine/Runtime/ISubsystem.h"

using engine::core::f64;

using namespace engine;
using namespace engine::application;

// ============================================================================
// Construction
// ============================================================================

TEST(ApplicationTest, ConstructionFromArgv)
{
    const char* argv[] = {"TestApp", "--log-level", "3"};
    Application app(3, argv);

    EXPECT_EQ(app.GetSpec().name, "TestApp");
    EXPECT_TRUE(app.GetCommandLine().Has("log-level"));
    EXPECT_EQ(app.GetCommandLine().GetInt("log-level"), 3);
    EXPECT_FALSE(app.IsInitialized());
}

TEST(ApplicationTest, ConstructionFromSpec)
{
    application::ApplicationSpec spec;
    spec.name = "MyApp";

    Application app(spec);
    EXPECT_EQ(app.GetSpec().name, "MyApp");
    EXPECT_FALSE(app.IsInitialized());
}

// ============================================================================
// Engine access before initialization
// ============================================================================

TEST(ApplicationTest, EngineAccessibleBeforeInit)
{
    const char* argv[] = {"TestApp"};
    Application app(1, argv);

    // The engine should be created but not initialized.
    EXPECT_NE(&app.GetEngine(), nullptr);
    EXPECT_EQ(app.GetEngine().GetState(), runtime::EngineState::Starting);
}

// ============================================================================
// Subsystem registration before initialization
// ============================================================================

class NopSubsystem final : public runtime::ISubsystem
{
public:
    [[nodiscard]] std::string_view GetName() const noexcept override { return "Nop"; }
    bool Initialize() override { m_initialized = true; return true; }
    void Shutdown() override { m_initialized = false; }
    void Update(f64) override {}
    void FixedUpdate(f64) override {}
    void LateUpdate(f64) override {}

    bool m_initialized = false;
};

TEST(ApplicationTest, RegisterSubsystemBeforeInit)
{
    const char* argv[] = {"TestApp"};
    Application app(1, argv);

    auto nop = std::make_unique<NopSubsystem>();
    app.GetEngine().GetSubsystemManager().Register(std::move(nop));

    EXPECT_TRUE(app.GetEngine().GetSubsystemManager().Has("Nop"));
}

// ============================================================================
// Initialize and shutdown
// ============================================================================

TEST(ApplicationTest, InitializeAndShutdown)
{
    const char* argv[] = {"TestApp", "--headless", "--no-build-info"};
    Application app(4, argv);

    auto nop = std::make_unique<NopSubsystem>();
    app.GetEngine().GetSubsystemManager().Register(std::move(nop));

    ASSERT_TRUE(app.Initialize());
    EXPECT_TRUE(app.IsInitialized());

    app.Shutdown();
    EXPECT_FALSE(app.IsInitialized());
}

// ============================================================================
// Command-line overrides
// ============================================================================

TEST(ApplicationTest, CommandLineOverridesConfig)
{
    const char* argv[] = {"TestApp", "--no-build-info"};
    Application app(2, argv);

    // The --no-build-info flag should have set printBuildInfo to false.
    EXPECT_FALSE(app.GetConfig().engineConfig.printBuildInfo);
}