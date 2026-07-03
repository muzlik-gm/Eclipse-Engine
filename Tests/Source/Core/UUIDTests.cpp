#include <gtest/gtest.h>

#include "Engine/Core/UUID.h"

#include <unordered_set>

using namespace engine::core;

TEST(UUIDTest, DefaultConstructor)
{
    UUID id;
    EXPECT_TRUE(id.IsValid());
}

TEST(UUIDTest, InvalidUUID)
{
    UUID invalid = UUID::Invalid();
    EXPECT_FALSE(invalid.IsValid());
}

TEST(UUIDTest, ToStringFormat)
{
    UUID id;
    std::string str = id.ToString();

    // Standard UUID string is 36 characters: 8-4-4-4-12 with hyphens.
    ASSERT_EQ(str.length(), 36u);
    EXPECT_EQ(str[8],  '-');
    EXPECT_EQ(str[13], '-');
    EXPECT_EQ(str[18], '-');
    EXPECT_EQ(str[23], '-');

    // Version nibble (character at index 14) must be '4' for UUID v4.
    EXPECT_EQ(str[14], '4');
}

TEST(UUIDTest, RoundTrip)
{
    UUID original;
    std::string str = original.ToString();
    UUID restored = UUID::FromString(str);

    EXPECT_EQ(original, restored);
}

TEST(UUIDTest, Uniqueness)
{
    constexpr int kCount = 1000;
    std::unordered_set<UUID> set;
    set.reserve(kCount);

    for (int i = 0; i < kCount; ++i)
    {
        UUID id;
        ASSERT_TRUE(set.insert(id).second)
            << "Duplicate UUID generated at index " << i << ": " << id.ToString();
    }

    EXPECT_EQ(set.size(), static_cast<std::size_t>(kCount));
}

TEST(UUIDTest, Equality)
{
    UUID a;
    UUID b;

    // Two independently generated UUIDs should differ.
    EXPECT_NE(a, b);
    EXPECT_FALSE(a == b);

    // A UUID should equal itself and a copy.
    UUID copy = a;
    EXPECT_EQ(a, copy);
    EXPECT_FALSE(a != copy);
}

TEST(UUIDTest, HashConsistency)
{
    UUID id;
    std::size_t h1 = id.Hash();
    std::size_t h2 = id.Hash();

    EXPECT_EQ(h1, h2);

    // Also verify via std::hash specialization.
    std::hash<UUID> hasher;
    EXPECT_EQ(hasher(id), id.Hash());
}