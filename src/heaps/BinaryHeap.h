#pragma once

#include <functional>
#include <utility>
#include <vector>

/**
 * @brief A binary heap structure, min heap default.
 * - Uses std::vector internally for contigous memory.
 * - Left child = (2 * i) + 1;
 * - Right child = (2 * i) + 2;
 * - Parent Node = (i - 1) / 2;
 *
 * @tparam T Type to store.
 * @tparam Compare Min heap default. Prioritises less than values.
 */
template<typename T, typename Compare = std::less<T>>
class BinaryHeap
{
public:
    void push(const T& value)
    {
        m_data.push_back(value);

        // Loop through parent nodes and swap while doing so if comparison allows so.
        std::size_t currIndex = m_data.size() - 1;
        while(currIndex != 0)
        {
            const std::size_t parentIndex = (currIndex - 1) / 2;

            // Compare as !Compare for cases where same value would result in false, in which case exit.
            if(!Compare{}(m_data[currIndex], m_data[parentIndex]))
            {
                break;
            }

            std::swap(m_data[parentIndex], m_data[currIndex]);
            currIndex = parentIndex;
        }
    }

    void pop()
    {
        if(m_data.empty())
        {
            return;
        }

        if(m_data.size() == 1)
        {
            m_data.pop_back();
            return;
        }

        std::size_t currIndex{0U};

        std::swap(m_data.front(), m_data.back());
        m_data.pop_back();

        while(true)
        {
            const std::size_t leftChildIndex{(2*currIndex) + 1};

            // Exit early, reached max data size.
            if(leftChildIndex >= m_data.size())
            {
                break;
            }

            std::size_t winnerIndex{leftChildIndex};
            const std::size_t rightChildIndex{leftChildIndex + 1};

            // Check if still a has remaining data in storage for right child.
            if (rightChildIndex < m_data.size()) 
            {
                // Choose higher priority value as the index to compare to our active node.
                winnerIndex =
                    Compare{}(m_data[rightChildIndex], m_data[leftChildIndex])
                        ? rightChildIndex
                        : leftChildIndex;
            }

            // Compare as !Compare for cases where same value would result in false, in which case exit.
            if(!Compare{}(m_data[winnerIndex], m_data[currIndex]))
            {
                break;
            }

            std::swap(m_data[winnerIndex], m_data[currIndex]);
            currIndex = winnerIndex;
        };
    }

    const T& top() const
    {
        return m_data.front();
    }

    bool empty() const
    {
        return m_data.empty();
    }

    std::size_t size() const
    {
        return m_data.size();
    }

private:

    std::vector<T> m_data;

};