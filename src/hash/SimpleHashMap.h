#pragma once

#include <vector>
#include <utility>
#include <cstddef>
#include <tuple>

/**
 * Simple hash map exercise
 * - Separate chaining strategy for holding elements: 
 *      - Buckets hold entries (in this case tuples) of Key, Value, Hash.
 *      - Collisions into same bucket chain the tuple entries.
 * - Power of 2 for load factor, fast modulus, but tradeoff if hash bit distribution is weak because
 *      it might cause more collisions.
 * - 
 */
template<typename K, typename V, typename Hash = std::hash<K>>
class SimpleHashMap
{
public:

    SimpleHashMap()
    {
        m_buckets.resize(8); // Power of 2 strategy for bucket count (faster modulus calculation).
        m_sizeMaskModulus = m_buckets.size() - 1; // size mask for fast modulus operation (only works with power of 2 sizes)
    }

    ~SimpleHashMap() = default;

    void insert(const K& key, const V& val)
    {
        const std::size_t hash = Hash{}(key);
        std::size_t index = hash & m_sizeMaskModulus; // equivilant to hash % modulus but faster (with trade offs if hash bit distribution is not good)

        for (auto &t : m_buckets[index])
        {
            if (std::get<2>(t) == hash && std::get<0>(t) == key)
            {
                std::get<1>(t) = val;
                return;
            }
        }

        const float lf = static_cast<float>(m_size + 1) / static_cast<float>(m_buckets.size());
        if(lf >= k_loadFactorThreshold)
        {
            if(rehash()) //If cant rehash then live with key collisions from there onwards.
            {
                index = hash & m_sizeMaskModulus;
            }
        }

        m_buckets[index].emplace_back(key, val, hash);
        ++m_size;

        return;
    }

    bool find(const K& key, V& outVal) const
    {
        const std::size_t hash = Hash{}(key);
        const std::size_t index = hash & m_sizeMaskModulus;

        if (!m_buckets[index].empty())
        {
            for (auto& t : m_buckets[index])
            {
                if (std::get<2>(t) == hash && std::get<0>(t) == key)
                {
                    outVal = std::get<1>(t);
                    return true;
                }
            }
        }

        return false;
    }

    bool erase(const K& key)
    {
        const std::size_t hash = Hash{}(key);
        const std::size_t index = hash & m_sizeMaskModulus;

        if (!m_buckets[index].empty())
        {
            for (std::size_t i = 0; i < m_buckets[index].size(); ++i)
            {
                Tuple& t = m_buckets[index][i];
                if (std::get<2>(t) == hash && std::get<0>(t) == key)
                {
                    m_buckets[index].erase(m_buckets[index].begin() + i);
                    --m_size;
                    return true;
                }
            }
        }

        return true; // If element not found, same as erased.
    }

    std::size_t size() const
    {
        return m_size;
    }

private:

    bool rehash()
    {
        if (m_buckets.size() > std::numeric_limits<std::size_t>::max() / 2) 
        {
            // Should never hit but Log warning here if required.
            return false;
        }

        const std::size_t newBucketSize = m_buckets.size() << 1;
        const auto newMask = newBucketSize - 1;

        Bucket newBuckets;
        newBuckets.resize(newBucketSize);

        for(auto& b : m_buckets)
        {
            for(auto& t : b)
            {
                const auto newIndex{std::get<2>(t) & newMask};
                newBuckets[newIndex].push_back(std::move(t));
            }
        }

        m_buckets = std::move(newBuckets);
        m_sizeMaskModulus = newMask;
        return true;
    }

    using Tuple = std::tuple<K, V, std::size_t>;
    using Elements = std::vector<Tuple>;
    using Bucket = std::vector<Elements>;

    static constexpr float k_loadFactorThreshold{0.75F};

    Bucket m_buckets;
    std::size_t m_size{0U};
    std::size_t m_sizeMaskModulus{0U};


// Buckets hash indexed. Max count to begin with. Buckets contain max element capacity.
// Bucket load index. When reaches specific load, increase buckets, move elements around.

};