#pragma once

#include <assert.h>
#include <cstddef>
#include <functional>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

/**
 * Open Addressing hash map exercise (linear propagation)
 * - Open Addressing strategy for holding elements. Better cpu cache locality. Faster access.
 * - Possibly more often rehashing due to rebuilding linear entry slots statuses to prevent long search loops
 * if many collisions occured do to weak hashing. No separate linked/list nodes per collision
 * - More complicated than Simple hashmap using separate chaining (buckets).
 * - Using power of 2 for modulus
 * - 
 */
template<typename K, typename V, typename Hash = std::hash<K>>
class OpenAddressingHashMap
{
    enum class EntryStatus
    {
        Active,
        Deleted,
        Empty,
    };

    struct Entry
    {
        std::size_t hash{0U};
        K key{};
        V value{};
        EntryStatus status{EntryStatus::Empty};
    };

public:

    OpenAddressingHashMap()
    {
        m_elements.resize(16); // Power of 2 strategy for bucket count (faster modulus calculation).
        m_sizeMaskModulus = m_elements.size() - 1; // size mask for fast modulus operation (only works with power of 2 sizes)
    }

    ~OpenAddressingHashMap() = default;

    bool insert(const K& key, const V& val)
    {
        const std::size_t hash = Hash{}(key);
        const std::size_t index = hash & m_sizeMaskModulus; // equivilant to hash % modulus but faster (with trade offs if hash bit distribution is not good)

        Entry* possibleInsertionPos{nullptr};

        std::size_t currIndex = index;
        for(std::size_t i{0}; i < m_elements.size(); ++i)
        {
            auto& entry = m_elements[currIndex];
            currIndex = (currIndex + 1) & m_sizeMaskModulus;

            if(entry.status == EntryStatus::Deleted && possibleInsertionPos == nullptr)
            {
                possibleInsertionPos = &entry;
                continue;
            }

            // Replace value if key already exists.
            if (entry.status == EntryStatus::Active)
            {
                if (entry.hash == hash && entry.key == key)
                {
                    entry.value = val;
                    return true;
                }
            }

            // If nothing in slot, make entry active, update members.
            if(entry.status == EntryStatus::Empty)
            {
                if(possibleInsertionPos == nullptr)
                {
                    possibleInsertionPos = &entry;
                    //Only increment on insertion into untouched `Empty` slot (Inserting into deleted doesnt increment count).
                    ++m_touchedEntries;
                }
                break;
            }
        }

        if(possibleInsertionPos != nullptr)
        {
            possibleInsertionPos->key = key;
            possibleInsertionPos->value = val;
            possibleInsertionPos->hash = hash;
            possibleInsertionPos->status = EntryStatus::Active;
            ++m_size;
        }

        const float lf = static_cast<float>(m_touchedEntries) / static_cast<float>(m_elements.size());
        if(lf >= k_loadFactorThreshold)
        {
            rehash();
        }

        return true;
    }

    bool find(const K& key, V& outVal) const
    {
        const std::size_t hash = Hash{}(key);
        const std::size_t index = hash & m_sizeMaskModulus;

        std::size_t currIndex = index;
        for(std::size_t i{0}; i < m_elements.size(); ++i)
        {
            const auto& entry = m_elements[currIndex];

            if(entry.status == EntryStatus::Empty)
            {
                return false;
            }

            if(entry.status == EntryStatus::Active)
            {
                if(entry.hash == hash && entry.key == key)
                {
                    outVal = entry.value;
                    return true;
                }
            }

            currIndex = (currIndex + 1) & m_sizeMaskModulus;
        }

        return false;
    }

    bool erase(const K& key)
    {
        const std::size_t hash = Hash{}(key);
        const std::size_t index = hash & m_sizeMaskModulus;

        std::size_t currIndex = index;
        for(std::size_t i{0}; i < m_elements.size(); ++i)
        {
            auto& entry = m_elements[currIndex];

            if(entry.status == EntryStatus::Empty)
            {
                return true;
            }

            if (entry.status == EntryStatus::Active)
            {
                if (entry.hash == hash && entry.key == key)
                {
                    entry.status = EntryStatus::Deleted;
                    --m_size;
                    return true;
                }
            }

            currIndex = (currIndex + 1) & m_sizeMaskModulus;
        }

        return true;
    }

    std::size_t size() const
    {
        return m_size;
    }

private:

    bool rehash()
    {
        std::size_t newSize{m_elements.size()};

        const float sizeLf = static_cast<float>(m_size) / static_cast<float>(m_elements.size());
        if (sizeLf >= k_loadFactorThreshold)
        {
            if (m_elements.size() > std::numeric_limits<std::size_t>::max() / 2)
            {
                // Should never hit but Log warning here if required.
                return false;
            }

            newSize = m_elements.size() << 1;
        }

        const auto newMask = newSize - 1;

        Elements newElements;
        newElements.resize(newSize);

        for (auto& e : m_elements)
        {
            if(e.status != EntryStatus::Active)
            {
                continue;
            }

            std::size_t newIndex{e.hash & newMask};
            for(std::size_t i{0}; i < newElements.size(); ++i)
            {
                if(newElements[newIndex].status == EntryStatus::Empty)
                {
                    newElements[newIndex] = std::move(e);
                    break;
                }
                newIndex = (newIndex + 1) & newMask;
            }
        }

        m_elements = std::move(newElements);
        m_sizeMaskModulus = newMask;
        m_touchedEntries = m_size;
        return true;
    }

    using Elements = std::vector<Entry>;

    static constexpr float k_loadFactorThreshold{0.75F};

    Elements m_elements;
    std::size_t m_size{0U};
    std::size_t m_sizeMaskModulus{0U};
    std::size_t m_touchedEntries{0U};


// Buckets hash indexed. Max count to begin with. Buckets contain max element capacity.
// Bucket load index. When reaches specific load, increase buckets, move elements around.

};