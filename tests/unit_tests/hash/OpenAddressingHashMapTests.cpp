#include <gtest/gtest.h>

#include <hash/OpenAddressingHashMap.h>

#include <cstddef>
#include <functional>

struct OpenAddressingCollisionKey
{
    int id{};

    bool operator==(const OpenAddressingCollisionKey&) const = default;
};

template <>
struct std::hash<OpenAddressingCollisionKey>
{
    std::size_t operator()(const OpenAddressingCollisionKey&) const noexcept
    {
        return 0U;
    }
};

TEST(OpenAddressingHashMapTest, InsertMakesValueAvailableToFind)
{
    OpenAddressingHashMap<int, int> map;

    map.insert(42, 100);

    int value{};
    EXPECT_TRUE(map.find(42, value));
    EXPECT_EQ(value, 100);
}

TEST(OpenAddressingHashMapTest, FindReturnsFalseForAMissingKey)
{
    OpenAddressingHashMap<int, int> map;

    int value{};
    EXPECT_FALSE(map.find(42, value));
}

TEST(OpenAddressingHashMapTest, InsertReplacesTheValueForAnExistingKey)
{
    OpenAddressingHashMap<int, int> map;
    map.insert(42, 100);

    map.insert(42, 200);

    int value{};
    ASSERT_TRUE(map.find(42, value));
    EXPECT_EQ(value, 200);
}

TEST(OpenAddressingHashMapTest, EraseRemovesAnExistingKey)
{
    OpenAddressingHashMap<int, int> map;
    map.insert(42, 100);

    EXPECT_TRUE(map.erase(42));

    int value{};
    EXPECT_FALSE(map.find(42, value));
}

TEST(OpenAddressingHashMapTest, EraseOfAMissingKeySucceeds)
{
    OpenAddressingHashMap<int, int> map;

    EXPECT_TRUE(map.erase(42));
}

TEST(OpenAddressingHashMapTest, StoresEntriesAfterGrowingBeyond2048Keys)
{
    OpenAddressingHashMap<int, int> map;
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

TEST(OpenAddressingHashMapTest, SizeStartsAtZero)
{
    OpenAddressingHashMap<int, int> map;

    EXPECT_EQ(map.size(), 0U);
}

TEST(OpenAddressingHashMapTest, SizeIncrementsOnlyForNewKeys)
{
    OpenAddressingHashMap<int, int> map;

    map.insert(1, 10);
    EXPECT_EQ(map.size(), 1U);
    map.insert(2, 20);

    EXPECT_EQ(map.size(), 2U);
}

TEST(OpenAddressingHashMapTest, UpdatingAnExistingKeyDoesNotIncreaseSize)
{
    OpenAddressingHashMap<int, int> map;
    map.insert(1, 10);

    map.insert(1, 20);

    EXPECT_EQ(map.size(), 1U);
}

TEST(OpenAddressingHashMapTest, SuccessfulEraseDecreasesSize)
{
    OpenAddressingHashMap<int, int> map;
    map.insert(1, 10);

    EXPECT_TRUE(map.erase(1));
    EXPECT_EQ(map.size(), 0U);
}

TEST(OpenAddressingHashMapTest, ErasingAMissingKeyDoesNotChangeSize)
{
    OpenAddressingHashMap<int, int> map;
    map.insert(1, 10);

    EXPECT_TRUE(map.erase(2));

    EXPECT_EQ(map.size(), 1U);
}

TEST(OpenAddressingHashMapTest, EntriesRemainFindableAfterMultipleResizes)
{
    OpenAddressingHashMap<int, int> map;
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

TEST(OpenAddressingHashMapTest, CollisionsRemainFindableAfterResize)
{
    OpenAddressingHashMap<OpenAddressingCollisionKey, int> map;
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

TEST(OpenAddressingHashMapTest, ErasingFromACollisionChainPreservesOtherEntries)
{
    OpenAddressingHashMap<OpenAddressingCollisionKey, int> map;
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

TEST(OpenAddressingHashMapTest, InsertsIntoTombstoneWithoutBreakingProbeSequence)
{
    OpenAddressingHashMap<OpenAddressingCollisionKey, int> map;
    map.insert({1}, 10);
    map.insert({2}, 20);
    map.insert({3}, 30);
    ASSERT_TRUE(map.erase({2}));

    map.insert({4}, 40);

    int value{};
    ASSERT_TRUE(map.find({3}, value));
    EXPECT_EQ(value, 30);
    ASSERT_TRUE(map.find({4}, value));
    EXPECT_EQ(value, 40);
    EXPECT_EQ(map.size(), 3U);
}

TEST(OpenAddressingHashMapTest, FindsMissingKeyAfterProbeClusterWithoutModifyingOutput)
{
    OpenAddressingHashMap<OpenAddressingCollisionKey, int> map;
    map.insert({1}, 10);
    map.insert({2}, 20);

    int value{999};
    EXPECT_FALSE(map.find({3}, value));
    EXPECT_EQ(value, 999);
}
