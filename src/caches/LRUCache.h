#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>

template<typename Key, typename Val>
class LRUCache
{
public:
    LRUCache(std::size_t capacity) : m_capacity{capacity}
    {
        m_keyValueMap.reserve(capacity);
    }

    ~LRUCache() = default;

    bool get(const Key& key, Val& outValue)
    {
        auto it = m_keyValueMap.find(key);
        if(it == m_keyValueMap.end())
        {
            return false;
        }

        outValue = it->second->second; // second = Node Iterator. second->second = Node pair Val
        m_valueOrder.splice(m_valueOrder.begin(), m_valueOrder, it->second); // Move in linked list to beginning

        return true;
    }

    bool put(const Key& key, const Val& value)
    {
        auto it = m_keyValueMap.find(key);
        if(it == m_keyValueMap.end())
        {
            if(m_keyValueMap.size() == m_capacity)
            {
                remove();
            }

            m_valueOrder.push_front({key, value});
            m_keyValueMap[key] = m_valueOrder.begin();

            return true;
        }

        it->second->second = value;
        m_valueOrder.splice(m_valueOrder.begin(), m_valueOrder, it->second); // Move in linked list to beginning

        return true;
    }

    std::size_t getSize() const
    {
        return m_keyValueMap.size();
    }

private:

    void remove()
    {
        auto it = std::prev(m_valueOrder.end());
        m_keyValueMap.erase(it->first);
        m_valueOrder.erase(it);
    }

    using KVListIter = std::list<std::pair<Key, Val>>::iterator;

    std::size_t m_capacity{0U};

    std::list<std::pair<Key, Val>> m_valueOrder;
    std::unordered_map<Key, KVListIter> m_keyValueMap;
};