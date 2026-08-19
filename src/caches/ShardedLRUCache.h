#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>
#include <mutex>
#include <utility>

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
        std::list<std::pair<Key, Val>> valueOrder;
        std::unordered_map<Key, KVListIter> keyValueMap;
        std::size_t capacity{0U};
    };

public:
    ShardedLRUCache()
    {
        std::size_t basePerShard{Capacity / ShardCount};
        std::size_t remainder{Capacity % ShardCount};

        for(std::size_t i{0}; i < ShardCount; ++i)
        {
            auto& shard = m_shards[i];
            shard.capacity = basePerShard + (i < remainder ? 1 : 0);
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