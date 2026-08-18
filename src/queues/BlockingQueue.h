#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>
#include <mutex>
#include <condition_variable>

template<typename T, std::size_t Capacity>
class BlockingQueue
{
static_assert(Capacity > 0, "Bounded blocking queue capacity must be atleast 1.");

public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;

    void push(const T& value)
    {
        {
            std::unique_lock sl(m_mutex);
            m_pcv.wait(sl, [&]() { return m_size < Capacity; });

            m_queue[m_head] = value;
            m_head = (m_head + 1) % Capacity;
            ++m_size;
        }

        m_ccv.notify_one();
    }

    void push(T&& value)
    {
        {
            std::unique_lock sl(m_mutex);
            m_pcv.wait(sl, [&]() { return m_size < Capacity; });

            m_queue[m_head] = std::move(value);
            m_head = (m_head + 1) % Capacity;
            ++m_size;
        }

        m_ccv.notify_one();
    }

    void pop(T& outVal)
    {
        {
            std::unique_lock uniqueLock(m_mutex);
            m_ccv.wait(uniqueLock, [&]() { return m_size != 0; });

            outVal = std::move(m_queue[m_tail]);
            m_tail = (m_tail + 1) % Capacity;
            --m_size;
        }

        m_pcv.notify_one();
    }

    std::size_t getSize() noexcept
    {
        std::scoped_lock sl(m_mutex);
        return m_size;
    }

    std::size_t getCapacity() noexcept
    {
        return Capacity;
    }

    bool isFull() const noexcept
    {
        std::scoped_lock sl(m_mutex);
        return m_size == Capacity;
    }

    bool isEmpty() const noexcept
    {
        std::scoped_lock sl(m_mutex);
        return m_size == 0;
    }

private:
    T m_queue[Capacity];

    std::size_t m_head{0};
    std::size_t m_tail{0};
    std::size_t m_size{0};

    mutable std::mutex m_mutex;
    std::condition_variable m_pcv;
    std::condition_variable m_ccv;
};