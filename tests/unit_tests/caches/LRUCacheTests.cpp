#include <gtest/gtest.h>

#include <caches/LRUCache.h>

#include <cstddef>

TEST(LRUCacheTest, StartsEmptyAndStoresValues)
{
    LRUCache<int, int> cache(2);

    EXPECT_EQ(cache.getSize(), 0U);
    EXPECT_TRUE(cache.put(1, 10));
    EXPECT_EQ(cache.getSize(), 1U);

    int value{};
    EXPECT_TRUE(cache.get(1, value));
    EXPECT_EQ(value, 10);
}

TEST(LRUCacheTest, GetReturnsFalseForAMissingKey)
{
    LRUCache<int, int> cache(2);

    int value{};
    EXPECT_FALSE(cache.get(99, value));
}

TEST(LRUCacheTest, AddingPastCapacityEvictsTheLeastRecentlyUsedValue)
{
    LRUCache<int, int> cache(2);
    ASSERT_TRUE(cache.put(1, 10));
    ASSERT_TRUE(cache.put(2, 20));
    ASSERT_TRUE(cache.put(3, 30));

    int value{};
    EXPECT_FALSE(cache.get(1, value));
    EXPECT_TRUE(cache.get(2, value));
    EXPECT_EQ(value, 20);
    EXPECT_TRUE(cache.get(3, value));
    EXPECT_EQ(value, 30);
    EXPECT_EQ(cache.getSize(), 2U);
}

TEST(LRUCacheTest, GetRefreshesAValuesRecency)
{
    LRUCache<int, int> cache(2);
    ASSERT_TRUE(cache.put(1, 10));
    ASSERT_TRUE(cache.put(2, 20));

    int value{};
    ASSERT_TRUE(cache.get(1, value));
    ASSERT_TRUE(cache.put(3, 30));

    EXPECT_TRUE(cache.get(1, value));
    EXPECT_FALSE(cache.get(2, value));
    EXPECT_TRUE(cache.get(3, value));
}
