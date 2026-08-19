#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * Simple thread pool.
 * - Compile time thread count size and max tasks.
 * - Thread safe submit.
 * - Simple function<void()> tasks for submit() api.
 * - Graceful shutdown, drains all queued tasks before joining.
 */
template<std::size_t ThreadCount, std::size_t MaxTasks = 1024>
class SimpleThreadPool
{
    static_assert(ThreadCount > 0, "ThreadCount must be 1 or more.");
    static_assert(MaxTasks > 0, "MaxTasks must be 1 or more.");

public:
    using Task = std::function<void()>;

    explicit SimpleThreadPool()
    {
        // Set up all threads
        for (std::size_t i{0}; i < ThreadCount; ++i)
        {
            m_threads.emplace_back([this] 
                {
                    while (true)
                    {
                        Task task;

                        {
                            std::unique_lock ul(m_mutex);
                            m_HasTasksCV.wait(ul, [&] { return !m_taskQueue.empty() || m_shuttingDown; });

                            if (m_shuttingDown && m_taskQueue.empty())
                            {
                                return; // Exit
                            }

                            task = std::move(m_taskQueue.front()); // pop task while still under mutex protection
                            m_taskQueue.pop();
                        }

                        try
                        {
                            task(); // Execute task.
                        }
                        catch(...)
                        {
                            // Handle exception...
                        }
                    }
                });
        }
    }

    ~SimpleThreadPool()
    {
        shutdown();
    }

    void shutdown()
    {
        {
            std::scoped_lock sl(m_mutex);

            if (m_shuttingDown)
            {
                return;
            }

            m_shuttingDown = true;
        }

        m_HasTasksCV.notify_all(); // Wake all threads

        for(auto& t : m_threads)
        {
            if(t.joinable())
            {
                t.join();
            }
        }
    }

    bool submit(Task task)
    {
        {
            std::scoped_lock sl(m_mutex);

            if (m_shuttingDown)
            {
                return false;
            }

            if (m_taskQueue.size() == MaxTasks)
            {
                return false;
            }

            m_taskQueue.push(std::move(task));
        }

        m_HasTasksCV.notify_one(); // Wake after lock scope, otherwise wakes worker and potentially blocks at same time.
        return true;
    }

    constexpr std::size_t threadCount() const
    {
        return ThreadCount;
    }

private:

    std::mutex m_mutex;
    std::condition_variable m_HasTasksCV;

    std::vector<std::thread> m_threads; // All spun up threads.
    std::queue<Task> m_taskQueue; // queue of outstanding tasks.

    bool m_shuttingDown{false};
};