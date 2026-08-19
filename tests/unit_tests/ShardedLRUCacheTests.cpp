#include <gtest/gtest.h>

#include <caches/ShardedLRUCache.h>

#include <cstddef>
#include <functional>
#include <atomic>
#include <barrier>
#include <thread>
#include <vector>

namespace
{

struct ShardedCacheKey
{
    int value{};

    bool operator==(const ShardedCacheKey&) const = default;
};

} // namespace

template<>
struct std::hash<ShardedCacheKey>
{
    std::size_t operator()(const ShardedCacheKey& key) const noexcept
    {
        return static_cast<std::size_t>(key.value);
    }
};

TEST(ShardedLRUCacheTest, StartsEmptyAndStoresValuesInDifferentShards)
{
    ShardedLRUCache<ShardedCacheKey, int, 2U, 2U> cache;

    EXPECT_EQ(cache.getSize(), 0U);
    ASSERT_TRUE(cache.put({0}, 10));
    ASSERT_TRUE(cache.put({1}, 20));
    EXPECT_EQ(cache.getSize(), 2U);

    int value{};
    EXPECT_TRUE(cache.get({0}, value));
    EXPECT_EQ(value, 10);
    EXPECT_TRUE(cache.get({1}, value));
    EXPECT_EQ(value, 20);
}

TEST(ShardedLRUCacheTest, GetReturnsFalseForAMissingKey)
{
    ShardedLRUCache<ShardedCacheKey, int, 2U, 2U> cache;

    int value{42};
    EXPECT_FALSE(cache.get({99}, value));
    EXPECT_EQ(value, 42);
}

TEST(ShardedLRUCacheTest, UpdatingAnExistingKeyReplacesItsValueWithoutGrowingTheCache)
{
    ShardedLRUCache<ShardedCacheKey, int, 2U, 2U> cache;
    ASSERT_TRUE(cache.put({0}, 10));
    ASSERT_TRUE(cache.put({0}, 99));

    int value{};
    EXPECT_TRUE(cache.get({0}, value));
    EXPECT_EQ(value, 99);
    EXPECT_EQ(cache.getSize(), 1U);
}

TEST(ShardedLRUCacheTest, EvictsTheLeastRecentlyUsedEntryWithinAShard)
{
    ShardedLRUCache<ShardedCacheKey, int, 2U, 1U> cache;
    ASSERT_TRUE(cache.put({1}, 10));
    ASSERT_TRUE(cache.put({2}, 20));
    ASSERT_TRUE(cache.put({3}, 30));

    int value{};
    EXPECT_FALSE(cache.get({1}, value));
    EXPECT_TRUE(cache.get({2}, value));
    EXPECT_EQ(value, 20);
    EXPECT_TRUE(cache.get({3}, value));
    EXPECT_EQ(value, 30);
    EXPECT_EQ(cache.getSize(), 2U);
}

TEST(ShardedLRUCacheTest, GetRefreshesAnEntriesRecencyWithinItsShard)
{
    ShardedLRUCache<ShardedCacheKey, int, 2U, 1U> cache;
    ASSERT_TRUE(cache.put({1}, 10));
    ASSERT_TRUE(cache.put({2}, 20));

    int value{};
    ASSERT_TRUE(cache.get({1}, value));
    ASSERT_TRUE(cache.put({3}, 30));

    EXPECT_TRUE(cache.get({1}, value));
    EXPECT_FALSE(cache.get({2}, value));
    EXPECT_TRUE(cache.get({3}, value));
}

TEST(ShardedLRUCacheTest, SupportsConcurrentReadersAndWriters)
{
    constexpr std::size_t shardCount{4U};
    constexpr std::size_t writerCount{8U};
    constexpr std::size_t readerCount{8U};
    constexpr std::size_t valuesPerWriter{64U};
    constexpr std::size_t readerOperations{2'000U};
    constexpr std::size_t totalValues{writerCount * valuesPerWriter};

    ShardedLRUCache<ShardedCacheKey, int, totalValues, shardCount> cache;
    std::barrier start{static_cast<std::ptrdiff_t>(writerCount + readerCount)};
    std::atomic<std::size_t> failedWrites{0U};
    std::atomic<std::size_t> incorrectValues{0U};
    std::vector<std::thread> threads;
    threads.reserve(writerCount + readerCount);

    for(std::size_t writer{0U}; writer < writerCount; ++writer)
    {
        threads.emplace_back([&, writer]
        {
            start.arrive_and_wait();

            const std::size_t firstKey{writer * valuesPerWriter};
            for(std::size_t offset{0U}; offset < valuesPerWriter; ++offset)
            {
                const int key{static_cast<int>(firstKey + offset)};
                if(!cache.put({key}, key * 10))
                {
                    ++failedWrites;
                }
            }
        });
    }

    for(std::size_t reader{0U}; reader < readerCount; ++reader)
    {
        threads.emplace_back([&, reader]
        {
            start.arrive_and_wait();

            for(std::size_t operation{0U}; operation < readerOperations; ++operation)
            {
                const int key{static_cast<int>((operation + reader) % totalValues)};
                int value{};

                if(cache.get({key}, value) && value != key * 10)
                {
                    ++incorrectValues;
                }
            }
        });
    }

    for(auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(failedWrites.load(), 0U);
    EXPECT_EQ(incorrectValues.load(), 0U);
    EXPECT_EQ(cache.getSize(), totalValues);

    for(std::size_t key{0U}; key < totalValues; ++key)
    {
        int value{};
        EXPECT_TRUE(cache.get({static_cast<int>(key)}, value));
        EXPECT_EQ(value, static_cast<int>(key) * 10);
    }
}
