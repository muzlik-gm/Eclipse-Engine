// ============================================================================
// File: Tests/Source/Runtime/ModuleManagerTests.cpp
// Tests for module registration, dependency validation, version
// compatibility, and lifecycle management.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Core/Log.h"
#include "Engine/Runtime/ModuleManager.h"
#include "Engine/Runtime/IModule.h"

#include <string>
#include <vector>

using namespace engine::runtime;

// ============================================================================
// Test module implementation
// ============================================================================

class TestModule final : public IModule
{
public:
    explicit TestModule(std::string name, u32 major = 1, u32 minor = 0, u32 patch = 0)
        : m_name(std::move(name))
        , m_major(major), m_minor(minor), m_patch(patch)
    {}

    [[nodiscard]] std::string_view GetName() const noexcept override { return m_name; }
    [[nodiscard]] std::string_view GetDescription() const noexcept override { return "Test module"; }

    [[nodiscard]] u32 GetVersionMajor() const noexcept override { return m_major; }
    [[nodiscard]] u32 GetVersionMinor() const noexcept override { return m_minor; }
    [[nodiscard]] u32 GetVersionPatch() const noexcept override { return m_patch; }

    [[nodiscard]] std::vector<std::string> GetDependencies() const override { return m_deps; }

    bool Initialize() override { ++m_initCount; return m_initSuccess; }
    void Shutdown() override { ++m_shutdownCount; }

    std::string              m_name;
    std::vector<std::string> m_deps;
    int                     m_initCount     = 0;
    int                     m_shutdownCount = 0;
    bool                    m_initSuccess   = true;
    u32                     m_major = 1;
    u32                     m_minor = 0;
    u32                     m_patch = 0;
};

// ============================================================================
// Registration
// ============================================================================

TEST(ModuleManagerTest, RegisterAndLookup)
{
    ModuleManager mgr;
    auto mod = std::make_unique<TestModule>("Core");
    const auto* raw = mod.get();

    const bool registered = mgr.Register(std::move(mod));
    EXPECT_TRUE(registered);
    EXPECT_TRUE(mgr.Has("Core"));
    EXPECT_EQ(mgr.Count(), 1u);
    EXPECT_EQ(mgr.GetRaw("Core"), raw);
    EXPECT_EQ(mgr.Get<TestModule>("Core"), raw);
}

TEST(ModuleManagerTest, DuplicateDetection)
{
    ModuleManager mgr;
    mgr.Register(std::make_unique<TestModule>("Core"));

    // Second registration with the same name should be rejected.
    const bool second = mgr.Register(std::make_unique<TestModule>("Core"));
    EXPECT_FALSE(second);
    EXPECT_EQ(mgr.Count(), 1u);
}

TEST(ModuleManagerTest, NullModuleRejected)
{
    ModuleManager mgr;
    // Should not crash; returns false.
    const bool result = mgr.Register(nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(mgr.Count(), 0u);
}

TEST(ModuleManagerTest, Unregister)
{
    ModuleManager mgr;
    mgr.Register(std::make_unique<TestModule>("X"));

    const bool removed = mgr.Unregister("X");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(mgr.Has("X"));
    EXPECT_EQ(mgr.Count(), 0u);

    EXPECT_FALSE(mgr.Unregister("X"));
}

// ============================================================================
// Version compatibility
// ============================================================================

TEST(ModuleManagerTest, VersionCompatible)
{
    ModuleManager mgr;
    mgr.Register(std::make_unique<TestModule>("Mod", 2, 3, 1));

    EXPECT_TRUE(mgr.CheckVersionCompatibility("Mod", 2));
    EXPECT_FALSE(mgr.CheckVersionCompatibility("Mod", 1));
    EXPECT_FALSE(mgr.CheckVersionCompatibility("Mod", 3));
}

TEST(ModuleManagerTest, VersionCheckMissingModule)
{
    ModuleManager mgr;
    EXPECT_FALSE(mgr.CheckVersionCompatibility("NonExistent", 1));
}

// ============================================================================
// Dependency ordering
// ============================================================================

TEST(ModuleManagerTest, DependencyOrdering)
{
    ModuleManager mgr;

    auto c = std::make_unique<TestModule>("C");
    auto b = std::make_unique<TestModule>("B");
    auto a = std::make_unique<TestModule>("A");

    c->m_deps = {"B"};
    b->m_deps = {"A"};

    mgr.Register(std::move(c));
    mgr.Register(std::move(b));
    mgr.Register(std::move(a));

    ASSERT_TRUE(mgr.InitializeAll());

    EXPECT_TRUE(mgr.IsInitialized("A"));
    EXPECT_TRUE(mgr.IsInitialized("B"));
    EXPECT_TRUE(mgr.IsInitialized("C"));

    mgr.ShutdownAll();
}

TEST(ModuleManagerTest, MissingDependencyFails)
{
    ModuleManager mgr;

    auto mod = std::make_unique<TestModule>("X");
    mod->m_deps = {"NonExistent"};
    mgr.Register(std::move(mod));

    EXPECT_FALSE(mgr.InitializeAll());
}

TEST(ModuleManagerTest, CyclicDependencyFails)
{
    ModuleManager mgr;

    auto a = std::make_unique<TestModule>("A");
    auto b = std::make_unique<TestModule>("B");

    a->m_deps = {"B"};
    b->m_deps = {"A"};

    mgr.Register(std::move(a));
    mgr.Register(std::move(b));

    EXPECT_FALSE(mgr.InitializeAll());
}

TEST(ModuleManagerTest, InitFailureStopsChain)
{
    ModuleManager mgr;

    auto ok   = std::make_unique<TestModule>("OK");
    auto fail = std::make_unique<TestModule>("Fail");
    fail->m_initSuccess = false;

    mgr.Register(std::move(ok));
    mgr.Register(std::move(fail));

    EXPECT_FALSE(mgr.InitializeAll());
    EXPECT_TRUE(mgr.IsInitialized("OK"));
    EXPECT_FALSE(mgr.IsInitialized("Fail"));
}

// ============================================================================
// GetNames
// ============================================================================

TEST(ModuleManagerTest, GetNames)
{
    ModuleManager mgr;
    mgr.Register(std::make_unique<TestModule>("Alpha"));
    mgr.Register(std::make_unique<TestModule>("Beta"));

    auto names = mgr.GetNames();
    EXPECT_EQ(names.size(), 2u);
}