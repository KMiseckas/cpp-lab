#include <gtest/gtest.h>

#include <pools/SimpleThreadPoolV2.h>

#include <atomic>
#include <barrier>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

TEST(SimpleThreadPoolV2Test, ReportsItsConfiguredWorkerCount)
{
    SimpleThreadPoolV2<3U> pool;

    EXPECT_EQ(pool.threadCount(), 3U);
}

TEST(SimpleThreadPoolV2Test, ExecutesASubmittedTask)
{
    SimpleThreadPoolV2<2U> pool;
    std::promise<int> result;
    auto completed = result.get_future();

    ASSERT_TRUE(pool.submit([&result]
        {
            result.set_value(42);
        }));

    EXPECT_EQ(completed.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(completed.get(), 42);
}

TEST(SimpleThreadPoolV2Test, RejectsSubmissionWhenTheQueueIsFull)
{
    SimpleThreadPoolV2<1U, 1U> pool;
    std::promise<void> workerStarted;
    auto started = workerStarted.get_future();
    std::promise<void> releaseWorker;
    auto release = releaseWorker.get_future().share();

    ASSERT_TRUE(pool.submit([&workerStarted, release]
        {
            workerStarted.set_value();
            release.wait();
        }));

    const bool workerIsRunning = started.wait_for(1s) == std::future_status::ready;
    if(!workerIsRunning)
    {
        releaseWorker.set_value();
        FAIL() << "The worker did not start the submitted task.";
    }

    EXPECT_TRUE(pool.submit([] {}));
    EXPECT_FALSE(pool.submit([] {}));
    releaseWorker.set_value();
}

TEST(SimpleThreadPoolV2Test, AcceptsWorkFromMultipleSubmittingThreads)
{
    constexpr std::size_t submitterCount{12U};
    constexpr std::size_t tasksPerSubmitter{100U};
    constexpr std::size_t taskCount{submitterCount * tasksPerSubmitter};

    SimpleThreadPoolV2<4U, taskCount * 2U> pool;
    std::barrier start{static_cast<std::ptrdiff_t>(submitterCount)};
    std::atomic<std::size_t> acceptedTasks{0U};
    std::atomic<std::size_t> completedTasks{0U};
    std::vector<std::thread> submitters;
    submitters.reserve(submitterCount);

    for(std::size_t submitter{0U}; submitter < submitterCount; ++submitter)
    {
        submitters.emplace_back([&]
        {
            start.arrive_and_wait();

            for(std::size_t task{0U}; task < tasksPerSubmitter; ++task)
            {
                if(pool.submit([&completedTasks]
                    {
                        ++completedTasks;
                    }))
                {
                    ++acceptedTasks;
                }
            }
        });
    }

    for(auto& submitter : submitters)
    {
        submitter.join();
    }

    const auto deadline = std::chrono::steady_clock::now() + 1s;
    while(completedTasks.load() != taskCount && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::yield();
    }

    EXPECT_EQ(acceptedTasks.load(), taskCount);
    EXPECT_EQ(completedTasks.load(), taskCount);
}

TEST(SimpleThreadPoolV2Test, ShutdownCompletesQueuedTasksAndRejectsNewTasks)
{
    SimpleThreadPoolV2<1U, 4U> pool;
    std::atomic<std::size_t> completedTasks{0U};
    std::promise<void> workerStarted;
    auto started = workerStarted.get_future();
    std::promise<void> releaseWorker;
    auto release = releaseWorker.get_future().share();

    ASSERT_TRUE(pool.submit([&]
        {
            workerStarted.set_value();
            release.wait();
            ++completedTasks;
        }));
    ASSERT_TRUE(pool.submit([&completedTasks] { ++completedTasks; }));
    ASSERT_TRUE(pool.submit([&completedTasks] { ++completedTasks; }));
    ASSERT_EQ(started.wait_for(1s), std::future_status::ready);

    auto shutdownCompleted = std::async(std::launch::async, [&pool]
        {
            pool.shutdown();
        });

    EXPECT_EQ(shutdownCompleted.wait_for(50ms), std::future_status::timeout);
    releaseWorker.set_value();
    EXPECT_EQ(shutdownCompleted.wait_for(1s), std::future_status::ready);

    EXPECT_EQ(completedTasks.load(), 3U);
    EXPECT_FALSE(pool.submit([] {}));
}

TEST(SimpleThreadPoolV2Test, DestructionCompletesAlreadyAcceptedTasks)
{
    std::atomic<std::size_t> completedTasks{0U};

    {
        SimpleThreadPoolV2<1U, 4U> pool;
        ASSERT_TRUE(pool.submit([&completedTasks] { ++completedTasks; }));
        ASSERT_TRUE(pool.submit([&completedTasks] { ++completedTasks; }));
        ASSERT_TRUE(pool.submit([&completedTasks] { ++completedTasks; }));
    }

    EXPECT_EQ(completedTasks.load(), 3U);
}
