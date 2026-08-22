#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <future>
#include <expected>
#include <concepts>
#include <type_traits>
#include <functional>
#include <utility>

enum class ErrorType
{
    Unknown = 0U,
    ShuttingDown = 1U,
    ReachedMaxTasks = 2U,
};

struct Error
{
    Error() = default;
    explicit Error(ErrorType type) : type(type) {}

    ErrorType type{ErrorType::Unknown};
};

/**
 * Simple thread pool V2
 * - Compile time thread count and max tasks.
 * - Thread safe submit.
 * - Simple function<void()> tasks for submit() api.
 * - Graceful shutdown, drains all queued tasks before joining.
 * - Submit arbitrary callables and args. Type erasure through function<void()>. Use of std::packaged_tasks and futures
 * for return API along side std::exception<RetResult, Error> for cleaner separation of failure vs success to submit
 * a task.
 */
template<std::size_t ThreadCount, std::size_t MaxTasks = 1024>
class SimpleThreadPoolV2
{
    static_assert(ThreadCount > 0, "ThreadCount must be 1 or more.");
    static_assert(MaxTasks > 0, "MaxTasks must be 1 or more.");

    using Task = std::move_only_function<void()>;

public:
    SimpleThreadPoolV2()
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

                        // Execute task. (No need for try_catch, packaged_task handles exception surfacing for us)
                        task();
                    }
                });
        }
    }

    ~SimpleThreadPoolV2()
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


    template<typename F, typename... Args>
    requires std::invocable<F, Args...>
    std::expected<std::future<std::invoke_result_t<F, Args...>>, Error> submit(F&& func, Args&& ...args)
    {
        using Fn = std::decay_t<F>;
        using RetResult = std::invoke_result_t<Fn, std::decay_t<Args>...>;

        std::future<RetResult> f;

        {
            std::scoped_lock sl(m_mutex);

            if (m_shuttingDown)
            {
                return std::unexpected(Error(ErrorType::ShuttingDown));
            }

            if (m_taskQueue.size() == MaxTasks)
            {
                return std::unexpected(Error(ErrorType::ReachedMaxTasks));
            }

            std::packaged_task<RetResult()> packagedTask{
                [func = Fn(std::forward<F>(func)),
                 ...args = std::decay_t<Args>(std::forward<Args>(args))]() mutable
                { 
                    return std::invoke(func, args...);
                }};

            f = packagedTask.get_future();

            Task task = [p = std::move(packagedTask)] mutable {
                p();
            };

            m_taskQueue.push(std::move(task));
        }

        m_HasTasksCV.notify_one(); // Wake after lock scope, otherwise wakes worker and potentially blocks at same time.
        return f;
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