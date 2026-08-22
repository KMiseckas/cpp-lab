#pragma once

#include <vector>
#include <utility>
#include <cstddef>
#include <tuple>

template<typename K, typename V>
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
        const std::size_t hash = std::hash<Key>(){key};
        std::size_t index = hash & m_sizeMaskModulus; // equivilant to hash % modulus but faster (10x-20x cpu cycles)

        if (!m_buckets.empty())
        {
            for (auto &t : m_buckets[index])
            {
                if (std::get<2>(t) == hash && std::get<0>(t) == key)
                {
                    std::get<1>(t) = val;
                    return;
                }
            }
        }

        const float lf = (m_size + 1) / m_buckets.size();
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

    bool find(const K& key, V& outVal)
    {
        const std::size_t hash = std::hash<Key>(){key};
        const std::size_t index = hash & m_sizeMaskModulus;

        if (!m_buckets[index].empty())
        {
            for (const auto &t = m_buckets[index])
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
        const std::size_t hash = std::hash<Key>(){key};
        const std::size_t index = hash & m_sizeMaskModulus;

        if (!m_buckets[index].empty())
        {
            for (std::size_t i = 0; i < m_buckets[index].size(); ++i)
            {
                Tuple& t = m_buckets[index][i];
                if (std::get<2>(t) == hash && std::get<0>(t) == key)
                {
                    std::swap(m_buckets[index][i], m_buckets[index].end() - 1);
                    m_buckets[index].erase(m_buckets.end() - 1);
                    return true;
                }
            }
        }

        return true; // If element not found, same as erased.
    }

private:

    bool rehash()
    {
        if (m_size > std::numeric_limits<std::size_t>::max() / 2) 
        {
            // Should never hit but Log warning here with logger just incase...
            return false;
        }

        const auto newBucketSize = m_buckets.size() << 1;
        m_sizeMaskModulus = newBucketSize - 1;

        // LOGIC TODO
    }

    using Tuple = std::tuple<K, V, std::size_t>;
    using Elements = std::vector<Tuple>;
    using Bucket = std::vector<Elements>;

    constexpr float k_loadFactorThreshold{0.75F};

    Bucket m_buckets;
    std::size_t m_size{0U};
    std::size_t m_sizeMaskModulus{0U};


// Buckets hash indexed. Max count to begin with. Buckets contain max element capacity.
// Bucket load index. When reaches specific load, increase buckets, move elements around.

};