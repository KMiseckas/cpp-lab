#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>
#include <mutex>
#include <utility>

/**
 * Sharder and thread safe LRU Cache
 * - Sharding, LRU cache consists of shards, each one basically a small LRU cache. Helps higher concurrency access as multiple shards can be
 * accessed at the same time.
 * - Downside, no global least used values. LRU per shard only.
 * - Current implementation has no capacity adjustment. A single shard may experience high usage but not do anything about it to relieve pressure on it.
 * - No exception handling (in this implementation). Could try catch several areas to safely exit functions without invalidating cache.
 */
template<typename Key, typename Val, std::size_t Capacity, std::size_t ShardCount = 8U>
class ShardedLRUCache
{
    static_assert(ShardCount > 0, "Must contain atleast 1 shard.");
    static_assert(Capacity > 0, "Must contain atleast 1 Capacity.");
    static_assert(Capacity >= ShardCount, "Must have more or equal ShardCount to Capacity.");

    struct Shard
    {
        using KVListIter = std::list<std::pair<Key, Val>>::iterator;

        mutable std::mutex mutex;
        std::list<std::pair<Key, Val>> valueOrder; //Double linked list of KV Pair nodes, holds the actual value. Allows quick reordering - O(1)
        std::unordered_map<Key, KVListIter> keyValueMap; // Map, holds iterator of list for quick access based on key - O(1)
        std::size_t capacity{0U};
    };

public:
    ShardedLRUCache()
    {
        std::size_t basePerShard{Capacity / ShardCount}; //Get base capacity to go into each shard.
        std::size_t remainder{Capacity % ShardCount}; // Remainder of what capscity left to assign after bases go into shards.

        for(std::size_t i{0}; i < ShardCount; ++i)
        {
            auto& shard = m_shards[i];
            shard.capacity = basePerShard + (i < remainder ? 1 : 0); // Add 1 additional capacity per shard until past remainder. Meets full capacity criteria.
            shard.keyValueMap.reserve(shard.capacity);
        }
    }

    ~ShardedLRUCache() = default;

    bool get(const Key& key, Val& outValue)
    {
        const std::size_t shardIndex{std::hash<Key>{}(key) % ShardCount};
        Shard& shard = m_shards[shardIndex];

        std::scoped_lock sl(shard.mutex);

        auto it = shard.keyValueMap.find(key);
        if(it == shard.keyValueMap.end())
        {
            return false;
        }

        outValue = it->second->second; // second = Node Iterator. second->second = Node pair Val
        shard.valueOrder.splice(shard.valueOrder.begin(), shard.valueOrder, it->second); // Move in linked list to beginning

        return true;
    }

    bool put(const Key& key, const Val& value)
    {
        const std::size_t shardIndex{std::hash<Key>{}(key) % ShardCount};
        Shard& shard = m_shards[shardIndex];

        std::scoped_lock sl(shard.mutex);

        auto it = shard.keyValueMap.find(key);
        if(it == shard.keyValueMap.end())
        {
            if(shard.keyValueMap.size() == shard.capacity)
            {
                auto listIter = std::prev(shard.valueOrder.end());
                shard.keyValueMap.erase(listIter->first);
                shard.valueOrder.erase(listIter);
            }

            shard.valueOrder.push_front({key, value});
            shard.keyValueMap[key] = shard.valueOrder.begin();

            return true;
        }

        it->second->second = value;
        shard.valueOrder.splice(shard.valueOrder.begin(), shard.valueOrder, it->second); // Move in linked list to beginning

        return true;
    }

    /**
     * Just a approx snapshot, not guaranteed to be exact if a shard thats not locked has a value evicted/added.
     */
    std::size_t getSize() const
    {
        std::size_t size{0U};

        for(int i = 0; i < ShardCount; ++i)
        {
            std::scoped_lock sl(m_shards[i].mutex);
            size += m_shards[i].keyValueMap.size();
        }

        return size;
    }

private:

    Shard m_shards[ShardCount];
    std::size_t m_next{0};
    
};