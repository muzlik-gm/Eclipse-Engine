// ============================================================================
// File: Tests/Source/Runtime/SubsystemManagerTests.cpp
// Tests for subsystem registration, dependency ordering, lifecycle, and
// per-frame update dispatching.
// ============================================================================

#include <gtest/gtest.h>
#include "Engine/Core/Log.h"
#include "Engine/Runtime/SubsystemManager.h"
#include "Engine/Runtime/ISubsystem.h"

#include <string>
#include <vector>

using namespace engine::runtime;

// ============================================================================
// Test subsystem implementations
// ============================================================================

class CounterSubsystem final : public ISubsystem
{
public:
    explicit CounterSubsystem(std::string name)
        : m_name(std::move(name))
    {}

    [[nodiscard]] std::string_view GetName() const noexcept override { return m_name; }

    [[nodiscard]] std::vector<std::string> GetDependencies() const override { return m_deps; }

    bool Initialize() override { ++m_initCount; return m_initSuccess; }
    void Shutdown() override { ++m_shutdownCount; }
    void Update(f64) override { ++m_updateCount; }
    void FixedUpdate(f64) override { ++m_fixedUpdateCount; }
    void LateUpdate(f64) override { ++m_lateUpdateCount; }

    std::string              m_name;
    std::vector<std::string> m_deps;
    int                     m_initCount       = 0;
    int                     m_shutdownCount   = 0;
    int                     m_updateCount     = 0;
    int                     m_fixedUpdateCount = 0;
    int                     m_lateUpdateCount = 0;
    bool                    m_initSuccess     = true;
};

// ============================================================================
// Registration
// ============================================================================

TEST(SubsystemManagerTest, RegisterAndLookup)
{
    SubsystemManager mgr;
    auto sub = std::make_unique<CounterSubsystem>("TestSub");
    const auto* rawPtr = sub.get();

    mgr.Register(std::move(sub));

    EXPECT_TRUE(mgr.Has("TestSub"));
    EXPECT_EQ(mgr.Count(), 1u);
    EXPECT_EQ(mgr.GetRaw("TestSub"), rawPtr);
    EXPECT_EQ(mgr.Get<CounterSubsystem>("TestSub"), rawPtr);
    EXPECT_FALSE(mgr.Has("NonExistent"));
    EXPECT_EQ(mgr.GetRaw("NonExistent"), nullptr);
}

TEST(SubsystemManagerTest, Replacement)
{
    SubsystemManager mgr;
    auto sub1 = std::make_unique<CounterSubsystem>("A");
    auto sub2 = std::make_unique<CounterSubsystem>("A");

    mgr.Register(std::move(sub1));
    mgr.Register(std::move(sub2));

    EXPECT_EQ(mgr.Count(), 1u);
}

TEST(SubsystemManagerTest, Unregister)
{
    SubsystemManager mgr;
    mgr.Register(std::make_unique<CounterSubsystem>("X"));
    EXPECT_TRUE(mgr.Has("X"));

    const bool removed = mgr.Unregister("X");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(mgr.Has("X"));
    EXPECT_EQ(mgr.Count(), 0u);

    // Removing non-existent returns false.
    EXPECT_FALSE(mgr.Unregister("X"));
}

TEST(SubsystemManagerTest, NullSubsystemRejected)
{
    SubsystemManager mgr;
    // Should not crash; just log a warning.
    mgr.Register(nullptr);
    EXPECT_EQ(mgr.Count(), 0u);
}

// ============================================================================
// Dependency ordering
// ============================================================================

TEST(SubsystemManagerTest, DependencyOrdering)
{
    SubsystemManager mgr;

    // Register in reverse dependency order.
    auto c = std::make_unique<CounterSubsystem>("C");
    auto b = std::make_unique<CounterSubsystem>("B");
    auto a = std::make_unique<CounterSubsystem>("A");

    c->m_deps = {"B"};
    b->m_deps = {"A"};
    // A has no deps.

    mgr.Register(std::move(c));
    mgr.Register(std::move(b));
    mgr.Register(std::move(a));

    ASSERT_TRUE(mgr.InitializeAll());

    // All should be initialized.
    EXPECT_TRUE(mgr.IsInitialized("A"));
    EXPECT_TRUE(mgr.IsInitialized("B"));
    EXPECT_TRUE(mgr.IsInitialized("C"));

    mgr.ShutdownAll();
}

TEST(SubsystemManagerTest, MissingDependencyFails)
{
    SubsystemManager mgr;

    auto sub = std::make_unique<CounterSubsystem>("X");
    sub->m_deps = {"NonExistent"};
    mgr.Register(std::move(sub));

    EXPECT_FALSE(mgr.InitializeAll());
}

TEST(SubsystemManagerTest, CyclicDependencyFails)
{
    SubsystemManager mgr;

    auto a = std::make_unique<CounterSubsystem>("A");
    auto b = std::make_unique<CounterSubsystem>("B");

    a->m_deps = {"B"};
    b->m_deps = {"A"};

    mgr.Register(std::move(a));
    mgr.Register(std::move(b));

    EXPECT_FALSE(mgr.InitializeAll());
}

// ============================================================================
// Init failure stops chain
// ============================================================================

TEST(SubsystemManagerTest, InitFailureStopsChain)
{
    SubsystemManager mgr;

    auto first  = std::make_unique<CounterSubsystem>("First");
    auto second = std::make_unique<CounterSubsystem>("Second");
    second->m_initSuccess = false;

    mgr.Register(std::move(first));
    mgr.Register(std::move(second));

    EXPECT_FALSE(mgr.InitializeAll());

    // First was initialized, second was not.
    EXPECT_TRUE(mgr.IsInitialized("First"));
    EXPECT_FALSE(mgr.IsInitialized("Second"));
}

// ============================================================================
// Per-frame updates
// ============================================================================

TEST(SubsystemManagerTest, UpdateDispatch)
{
    SubsystemManager mgr;

    auto sub = std::make_unique<CounterSubsystem>("Updatable");
    const auto* raw = sub.get();
    mgr.Register(std::move(sub));
    mgr.InitializeAll();

    mgr.UpdateAll(0.016);
    mgr.FixedUpdateAll(1.0 / 60.0);
    mgr.LateUpdateAll(0.016);

    EXPECT_EQ(raw->m_updateCount, 1);
    EXPECT_EQ(raw->m_fixedUpdateCount, 1);
    EXPECT_EQ(raw->m_lateUpdateCount, 1);

    mgr.ShutdownAll();
}

TEST(SubsystemManagerTest, UpdateOnlyInitialzedSubsystems)
{
    SubsystemManager mgr;

    auto a = std::make_unique<CounterSubsystem>("A");
    auto b = std::make_unique<CounterSubsystem>("B");
    b->m_initSuccess = false;
    const auto* bRaw = b.get();

    mgr.Register(std::move(a));
    mgr.Register(std::move(b));
    mgr.InitializeAll(); // B fails

    mgr.UpdateAll(0.016);

    // B was not initialized so it should not receive updates.
    EXPECT_EQ(bRaw->m_updateCount, 0);

    mgr.ShutdownAll();
}

// ============================================================================
// Shutdown in reverse order
// ============================================================================

TEST(SubsystemManagerTest, ShutdownReverseOrder)
{
    SubsystemManager mgr;

    std::vector<std::string> shutdownOrder;

    class OrderTracker : public ISubsystem
    {
    public:
        explicit OrderTracker(std::string name, std::vector<std::string>& order)
            : m_name(std::move(name)), m_order(order) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return m_name; }
        [[nodiscard]] std::vector<std::string> GetDependencies() const override { return m_deps; }
        bool Initialize() override { return true; }
        void Shutdown() override { m_order.push_back(m_name); }
        void Update(f64) override {}
        void FixedUpdate(f64) override {}
        void LateUpdate(f64) override {}

        std::vector<std::string> m_deps;

    private:
        std::string m_name;
        std::vector<std::string>& m_order;
    };

    auto c = std::make_unique<OrderTracker>("C", shutdownOrder);
    auto b = std::make_unique<OrderTracker>("B", shutdownOrder);
    auto a = std::make_unique<OrderTracker>("A", shutdownOrder);

    c->m_deps = {"B"};
    b->m_deps = {"A"};

    mgr.Register(std::move(c));
    mgr.Register(std::move(b));
    mgr.Register(std::move(a));

    mgr.InitializeAll();
    mgr.ShutdownAll();

    // Init order was A, B, C.  Shutdown should be C, B, A.
    ASSERT_EQ(shutdownOrder.size(), 3u);
    EXPECT_EQ(shutdownOrder[0], "C");
    EXPECT_EQ(shutdownOrder[1], "B");
    EXPECT_EQ(shutdownOrder[2], "A");
}

// ============================================================================
// GetNames
// ============================================================================

TEST(SubsystemManagerTest, GetNames)
{
    SubsystemManager mgr;
    mgr.Register(std::make_unique<CounterSubsystem>("Alpha"));
    mgr.Register(std::make_unique<CounterSubsystem>("Beta"));

    auto names = mgr.GetNames();
    // Unordered, but should have 2 entries.
    EXPECT_EQ(names.size(), 2u);
}