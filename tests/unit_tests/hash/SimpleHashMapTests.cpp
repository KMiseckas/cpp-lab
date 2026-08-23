#include <gtest/gtest.h>

#include <hash/SimpleHashMap.h>

#include <cstddef>
#include <functional>

struct CollisionKey
{
    int id{};

    bool operator==(const CollisionKey&) const = default;
};

template <>
struct std::hash<CollisionKey>
{
    std::size_t operator()(const CollisionKey&) const noexcept
    {
        return 0U;
    }
};

TEST(SimpleHashMapTest, InsertMakesValueAvailableToFind)
{
    SimpleHashMap<int, int> map;

    map.insert(42, 100);

    int value{};
    EXPECT_TRUE(map.find(42, value));
    EXPECT_EQ(value, 100);
}

TEST(SimpleHashMapTest, FindReturnsFalseForAMissingKey)
{
    SimpleHashMap<int, int> map;

    int value{};
    EXPECT_FALSE(map.find(42, value));
}

TEST(SimpleHashMapTest, InsertReplacesTheValueForAnExistingKey)
{
    SimpleHashMap<int, int> map;
    map.insert(42, 100);

    map.insert(42, 200);

    int value{};
    ASSERT_TRUE(map.find(42, value));
    EXPECT_EQ(value, 200);
}

TEST(SimpleHashMapTest, EraseRemovesAnExistingKey)
{
    SimpleHashMap<int, int> map;
    map.insert(42, 100);

    EXPECT_TRUE(map.erase(42));

    int value{};
    EXPECT_FALSE(map.find(42, value));
}

TEST(SimpleHashMapTest, EraseOfAMissingKeySucceeds)
{
    SimpleHashMap<int, int> map;

    EXPECT_TRUE(map.erase(42));
}

TEST(SimpleHashMapTest, StoresEntriesAfterGrowingBeyond2048Keys)
{
    SimpleHashMap<int, int> map;
    constexpr int entryCount{2049};

    for (int key{}; key < entryCount; ++key)
    {
        map.insert(key, key * 10);
    }

    for (int key{}; key < entryCount; ++key)
    {
        int value{};
        ASSERT_TRUE(map.find(key, value));
        EXPECT_EQ(value, key * 10);
    }
}

TEST(SimpleHashMapTest, SizeStartsAtZero)
{
    SimpleHashMap<int, int> map;

    EXPECT_EQ(map.size(), 0U);
}

TEST(SimpleHashMapTest, SizeIncrementsOnlyForNewKeys)
{
    SimpleHashMap<int, int> map;

    map.insert(1, 10);
    EXPECT_EQ(map.size(), 1U);
    map.insert(2, 20);

    EXPECT_EQ(map.size(), 2U);
}

TEST(SimpleHashMapTest, UpdatingAnExistingKeyDoesNotIncreaseSize)
{
    SimpleHashMap<int, int> map;
    map.insert(1, 10);

    map.insert(1, 20);

    EXPECT_EQ(map.size(), 1U);
}

TEST(SimpleHashMapTest, SuccessfulEraseDecreasesSize)
{
    SimpleHashMap<int, int> map;
    map.insert(1, 10);

    EXPECT_TRUE(map.erase(1));
    EXPECT_EQ(map.size(), 0U);
}

TEST(SimpleHashMapTest, ErasingAMissingKeyDoesNotChangeSize)
{
    SimpleHashMap<int, int> map;
    map.insert(1, 10);

    EXPECT_TRUE(map.erase(2));

    EXPECT_EQ(map.size(), 1U);
}

TEST(SimpleHashMapTest, EntriesRemainFindableAfterMultipleResizes)
{
    SimpleHashMap<int, int> map;
    constexpr int entryCount{10000};

    for (int key{}; key < entryCount; ++key)
    {
        map.insert(key, key * 10);
    }

    for (int key{}; key < entryCount; ++key)
    {
        int value{};
        ASSERT_TRUE(map.find(key, value));
        EXPECT_EQ(value, key * 10);
    }
}

TEST(SimpleHashMapTest, CollisionsRemainFindableAfterResize)
{
    SimpleHashMap<CollisionKey, int> map;
    constexpr int entryCount{64};

    for (int id{}; id < entryCount; ++id)
    {
        map.insert({id}, id * 10);
    }

    for (int id{}; id < entryCount; ++id)
    {
        int value{};
        ASSERT_TRUE(map.find({id}, value));
        EXPECT_EQ(value, id * 10);
    }
}

TEST(SimpleHashMapTest, ErasingFromACollisionChainPreservesOtherEntries)
{
    SimpleHashMap<CollisionKey, int> map;
    map.insert({1}, 10);
    map.insert({2}, 20);
    map.insert({3}, 30);

    ASSERT_TRUE(map.erase({2}));

    int value{};
    EXPECT_TRUE(map.find({1}, value));
    EXPECT_EQ(value, 10);
    EXPECT_FALSE(map.find({2}, value));
    EXPECT_TRUE(map.find({3}, value));
    EXPECT_EQ(value, 30);
}
