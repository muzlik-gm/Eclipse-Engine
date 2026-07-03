#include <gtest/gtest.h>

#include "Engine/Utilities/Hash.h"

#include <cstring>

using namespace engine::util;

TEST(HashTest, FNV1a64Basic)
{
    const char* data = "hello";
    u64 hash = FNV1a64(data, std::strlen(data));

    // A non-empty input must produce a non-zero hash.
    EXPECT_NE(hash, 0u);
}

TEST(HashTest, FNV1a64Consistency)
{
    const char* data = "consistency test";
    std::size_t len = std::strlen(data);

    u64 h1 = FNV1a64(data, len);
    u64 h2 = FNV1a64(data, len);

    EXPECT_EQ(h1, h2);
}

TEST(HashTest, FNV1a64DifferentInputs)
{
    const char* a = "alpha";
    const char* b = "bravo";

    u64 ha = FNV1a64(a, std::strlen(a));
    u64 hb = FNV1a64(b, std::strlen(b));

    EXPECT_NE(ha, hb);
}

TEST(HashTest, HashCombine)
{
    u64 seed = 0x123456789ABCDEF0ULL;
    u64 hash = 0xDEADBEEFCAFEBABEULL;

    u64 result1 = HashCombine(seed, hash);
    u64 result2 = HashCombine(seed, hash);

    // Same inputs must yield the same output (deterministic).
    EXPECT_EQ(result1, result2);
}