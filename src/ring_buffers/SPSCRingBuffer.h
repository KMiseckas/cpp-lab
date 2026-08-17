#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>
#include <atomic>

template<typename T, std::size_t Capacity>
class SPSCRingBuffer
{
static_assert(Capacity > 1, "SPSC ring buffer requires minimum capacity of 2");

public:
    SPSCRingBuffer() = default;
    ~SPSCRingBuffer() = default;

    bool push(const T& value)
    {
        const auto head{m_head.load(std::memory_order_relaxed)};
        const auto nextHead{(head + 1) % Capacity};

        // Acquire the consumer's published tail. If this load observes the release-store
        // from pop(), the consumer has finished reading the corresponding buffer slot
        // before this producer can reuse it.
        if(nextHead == m_tail.load(std::memory_order_acquire))
        {
            return false;
        }

        m_buffer[head] = value;
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }

    bool push(T&& value)
    {
        const auto head{m_head.load(std::memory_order_relaxed)};
        const auto nextHead{(head + 1) % Capacity};

        // Acquire the consumer's published tail. If this load observes the release-store
        // from pop(), the consumer has finished reading the corresponding buffer slot
        // before this producer can reuse it.
        if(nextHead == m_tail.load(std::memory_order_acquire))
        {
            return false;
        }

        m_buffer[head] = std::move(value);
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }

    bool pop(T& value)
    {
        const auto tail = m_tail.load(std::memory_order_relaxed);

        // Acquire the producer's published head. If this load observes the release-store
        // from push(), the write to m_buffer[tail] is visible before we read that slot.
        const auto head = m_head.load(std::memory_order_acquire);

        if(head == tail)
        {
            return false;
        }

        if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
        {
            value = std::move(m_buffer[tail]);
        }
        else
        {
            value = m_buffer[tail];
        }

        m_tail.store((tail + 1) % Capacity, std::memory_order_release);
        return true;
    }

    bool isEmpty() const noexcept
    {
        return m_head.load(std::memory_order_relaxed) == m_tail.load(std::memory_order_relaxed);
    }

    bool isFull() const noexcept
    {
        return (m_head.load(std::memory_order_relaxed) + 1) % Capacity == m_tail.load(std::memory_order_relaxed); // 1 empty slot in buffer to allow `full` state to be calculated.
    }

    /**
     * @brief Get the Count of elements. Excludes the empty slot used for internal `full` state calculation.
     * 
     * @return std::size_t Number of elements stored between tail and head in buffer.
     */
    std::size_t getCount() const noexcept
    {
        return (Capacity + m_head.load(std::memory_order_relaxed) - m_tail.load(std::memory_order_relaxed)) % Capacity;
    }

private:

    T m_buffer[Capacity];
    std::atomic<std::size_t> m_head{0};
    std::atomic<std::size_t> m_tail{0};
};