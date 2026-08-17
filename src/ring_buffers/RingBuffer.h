#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

template<typename T, std::size_t Capacity>
class RingBuffer
{
static_assert(Capacity > 1, "Ring buffer requires minimum capacity of 2");

public:
    RingBuffer() = default;
    ~RingBuffer() = default;

    bool push(const T& value)
    {
        if(isFull())
        {
            return false;
        }

        m_buffer[m_head] = value;
        m_head = (m_head + 1) % Capacity;
        return true;
    }

    bool push(T&& value)
    {
        if(isFull())
        {
            return false;
        }

        m_buffer[m_head] = std::move(value);
        m_head = (m_head + 1) % Capacity;
        return true;
    }

    bool pop(T& value)
    {
        if(isEmpty())
        {
            return false;
        }

        if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
        {
            value = std::move(m_buffer[m_tail]);
        }
        else
        {
            value = m_buffer[m_tail];
        }

        m_tail = (m_tail + 1) % Capacity;
        return true;
    }

    bool isEmpty() const noexcept
    {
        return m_head == m_tail;
    }

    bool isFull() const noexcept
    {
        return (m_head + 1) % Capacity == m_tail; // 1 empty slot in buffer to allow `full` state to be calculated.
    }

    /**
     * @brief Get the Count of elements. Excludes the empty slot used for internal `full` state calculation.
     * 
     * @return std::size_t Number of elements stored between tail and head in buffer.
     */
    std::size_t getCount() const noexcept
    {
        return (Capacity + m_head - m_tail) % Capacity;
    }

private:

    T m_buffer[Capacity];
    std::size_t m_head{0};
    std::size_t m_tail{0};
};