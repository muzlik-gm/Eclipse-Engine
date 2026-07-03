#include <gtest/gtest.h>

#include "Engine/Configuration/Config.h"

#include <algorithm>

using namespace engine::config;

TEST(ConfigTest, SetAndGet)
{
    Config cfg;

    cfg.SetString("name", "engine");
    cfg.SetInt("width", 1920);
    cfg.SetFloat("ratio", 1.5);
    cfg.SetBool("fullscreen", true);

    EXPECT_EQ(cfg.GetString("name"), "engine");
    EXPECT_EQ(cfg.GetInt("width"), 1920);
    EXPECT_DOUBLE_EQ(cfg.GetFloat("ratio"), 1.5);
    EXPECT_TRUE(cfg.GetBool("fullscreen"));
}

TEST(ConfigTest, DefaultValues)
{
    Config cfg;

    EXPECT_EQ(cfg.GetString("missing", "fallback"), "fallback");
    EXPECT_EQ(cfg.GetInt("missing", 42), 42);
    EXPECT_DOUBLE_EQ(cfg.GetFloat("missing", 3.14), 3.14);
    EXPECT_FALSE(cfg.GetBool("missing", false));
}

TEST(ConfigTest, HasAndRemove)
{
    Config cfg;
    cfg.SetString("key", "value");

    EXPECT_TRUE(cfg.Has("key"));

    bool removed = cfg.Remove("key");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(cfg.Has("key"));
}

TEST(ConfigTest, Merge)
{
    Config a;
    a.SetString("a_key", "a_val");
    a.SetInt("shared", 1);

    Config b;
    b.SetString("b_key", "b_val");
    b.SetInt("shared", 2);

    a.Merge(b);

    EXPECT_EQ(a.GetString("a_key"), "a_val");
    EXPECT_EQ(a.GetString("b_key"), "b_val");
    // Merge overwrites existing values.
    EXPECT_EQ(a.GetInt("shared"), 2);
}

TEST(ConfigTest, MergeDefaults)
{
    Config cfg;
    cfg.SetInt("fps", 60);

    Config defaults;
    defaults.SetInt("fps", 30);
    defaults.SetInt("width", 1280);

    cfg.MergeDefaults(defaults);

    // Existing value should NOT be overwritten.
    EXPECT_EQ(cfg.GetInt("fps"), 60);
    // Missing default should be populated.
    EXPECT_EQ(cfg.GetInt("width"), 1280);
}

TEST(ConfigTest, JSONRoundTrip)
{
    Config original;
    original.SetString("title", "Test");
    original.SetInt("count", 7);
    original.SetBool("enabled", true);

    std::string json = original.SaveToString("json");
    EXPECT_FALSE(json.empty());

    Config loaded;
    loaded.LoadFromString(json, "json");

    EXPECT_EQ(loaded.GetString("title"), "Test");
    EXPECT_EQ(loaded.GetInt("count"), 7);
    EXPECT_TRUE(loaded.GetBool("enabled"));
}

TEST(ConfigTest, Keys)
{
    Config cfg;
    cfg.SetString("alpha", "1");
    cfg.SetInt("beta", 2);
    cfg.SetBool("gamma", true);

    auto keys = cfg.Keys();
    ASSERT_EQ(keys.size(), 3u);

    // Verify all three expected keys are present (order is unspecified).
    auto contains = [&](std::string_view k) {
        return std::find(keys.begin(), keys.end(), k) != keys.end();
    };
    EXPECT_TRUE(contains("alpha"));
    EXPECT_TRUE(contains("beta"));
    EXPECT_TRUE(contains("gamma"));
}