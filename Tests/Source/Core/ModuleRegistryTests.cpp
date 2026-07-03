// ============================================================================
// File: Tests/Source/Core/ModuleRegistryTests.cpp
// ============================================================================

#include <gtest/gtest.h>

#include "Engine/Core/Log.h"
#include "Engine/Core/ModuleRegistry.h"

#include <string>
#include <vector>

using namespace engine::core;

namespace
{

/// Helper: clean up the singleton registry so tests don't pollute each other.
void CleanupRegistry()
{
    auto& reg = ModuleRegistry::Instance();
    // Collect names first to avoid invalidating iterators while erasing.
    auto all = reg.GetAllModules();
    for (const auto* mod : all)
    {
        reg.Unregister(mod->name);
    }
}

} // anonymous namespace

TEST(ModuleRegistryTest, RegisterAndGet)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    reg.Register("TestModule", "A test module", 10,
                 []() { return true; },
                 []() {});

    const ModuleInfo* mod = reg.GetModule("TestModule");
    ASSERT_NE(mod, nullptr);
    EXPECT_EQ(mod->name, "TestModule");
    EXPECT_EQ(mod->description, "A test module");
    EXPECT_EQ(mod->priority, 10u);

    CleanupRegistry();
    Log::Shutdown();
}

TEST(ModuleRegistryTest, InitializeAll)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    std::vector<std::string> initOrder;

    reg.Register("Mod_High",   "High priority",   10,
                 [&]() { initOrder.push_back("Mod_High");   return true; },
                 []() {});
    reg.Register("Mod_Mid",    "Mid priority",     5,
                 [&]() { initOrder.push_back("Mod_Mid");    return true; },
                 []() {});
    reg.Register("Mod_Low",    "Low priority",     1,
                 [&]() { initOrder.push_back("Mod_Low");    return true; },
                 []() {});

    bool ok = reg.InitializeAll();
    EXPECT_TRUE(ok);

    ASSERT_EQ(initOrder.size(), 3u);
    EXPECT_EQ(initOrder[0], "Mod_Low");   // priority 1
    EXPECT_EQ(initOrder[1], "Mod_Mid");   // priority 5
    EXPECT_EQ(initOrder[2], "Mod_High");  // priority 10

    CleanupRegistry();
    Log::Shutdown();
}

TEST(ModuleRegistryTest, ShutdownOrder)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    std::vector<std::string> shutdownOrder;

    reg.Register("Mod_High", "High priority", 10,
                 []() { return true; },
                 [&]() { shutdownOrder.push_back("Mod_High"); });
    reg.Register("Mod_Mid", "Mid priority", 5,
                 []() { return true; },
                 [&]() { shutdownOrder.push_back("Mod_Mid"); });
    reg.Register("Mod_Low", "Low priority", 1,
                 []() { return true; },
                 [&]() { shutdownOrder.push_back("Mod_Low"); });

    reg.InitializeAll();
    reg.ShutdownAll();

    ASSERT_EQ(shutdownOrder.size(), 3u);
    EXPECT_EQ(shutdownOrder[0], "Mod_High");  // highest priority shuts down first
    EXPECT_EQ(shutdownOrder[1], "Mod_Mid");
    EXPECT_EQ(shutdownOrder[2], "Mod_Low");

    CleanupRegistry();
    Log::Shutdown();
}

TEST(ModuleRegistryTest, IsInitialized)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    reg.Register("InitCheck", "Check init flag", 1,
                 []() { return true; },
                 []() {});

    EXPECT_FALSE(reg.IsInitialized("InitCheck"));

    reg.InitializeAll();

    EXPECT_TRUE(reg.IsInitialized("InitCheck"));

    CleanupRegistry();
    Log::Shutdown();
}

TEST(ModuleRegistryTest, Unregister)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    reg.Register("ToRemove", "Will be removed", 1,
                 []() { return true; },
                 []() {});

    ASSERT_NE(reg.GetModule("ToRemove"), nullptr);

    reg.Unregister("ToRemove");

    EXPECT_EQ(reg.GetModule("ToRemove"), nullptr);

    Log::Shutdown();
}

TEST(ModuleRegistryTest, DuplicateNameReplace)
{
    Log::Initialize("ModuleRegistryTests");
    CleanupRegistry();

    auto& reg = ModuleRegistry::Instance();

    reg.Register("DupMod", "Original", 1,
                 []() { return true; },
                 []() {});

    reg.Register("DupMod", "Replaced", 99,
                 []() { return true; },
                 []() {});

    const ModuleInfo* mod = reg.GetModule("DupMod");
    ASSERT_NE(mod, nullptr);
    EXPECT_EQ(mod->description, "Replaced");
    EXPECT_EQ(mod->priority, 99u);

    CleanupRegistry();
    Log::Shutdown();
}