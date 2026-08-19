#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>
#include <mutex>
#include <condition_variable>

/**
 * Bounded MPMC blocking queue.
 * - Mutexes
 * - Condition Variables (Block push on full, block pop on empty)
 * - Compile time capacity
 * - RAII locks
 */
template<typename T, std::size_t Capacity>
class BlockingQueue
{
static_assert(Capacity > 0, "Bounded blocking queue capacity must be atleast 1.");

public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;

    void close()
    {
        {
            std::scoped_lock sl(m_mutex);
            m_shuttingDown = true;
        }

        m_ccv.notify_all();
        m_pcv.notify_all();
    }

    bool push(const T& value)
    {
        {
            std::unique_lock sl(m_mutex);
            m_pcv.wait(sl, [&]() { return m_shuttingDown || m_size < Capacity; });

            if(m_shuttingDown)
            {
                return false;
            }

            m_queue[m_head] = value;
            m_head = (m_head + 1) % Capacity;
            ++m_size;
        }

        m_ccv.notify_one();
        return true;
    }

    bool push(T&& value)
    {
        {
            std::unique_lock sl(m_mutex);
            m_pcv.wait(sl, [&]() { return m_shuttingDown || m_size < Capacity; });

            if(m_shuttingDown)
            {
                return false;
            }

            m_queue[m_head] = std::move(value);
            m_head = (m_head + 1) % Capacity;
            ++m_size;
        }

        m_ccv.notify_one();
        return true;
    }

    template<typename... Args>
    bool emplace(Args&&...args)
    {
        // not really good emplacement example. Would need to write buffer as
        // something like std::optional<T> m_buffer[Capacity] for real emplacement.
        return push(T(std::forward<Args>(args)...));
    }

    bool pop(T& outVal)
    {
        {
            std::unique_lock uniqueLock(m_mutex);
            m_ccv.wait(uniqueLock, [&]() { return m_shuttingDown || m_size != 0; });

            if(m_size == 0)
            {
                return false;
            }

            outVal = std::move(m_queue[m_tail]);
            m_tail = (m_tail + 1) % Capacity;
            --m_size;
        }

        m_pcv.notify_one();
        return true;
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

    bool m_shuttingDown{false};

    mutable std::mutex m_mutex;
    std::condition_variable m_pcv;
    std::condition_variable m_ccv;
};