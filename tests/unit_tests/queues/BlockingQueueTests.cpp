#include <gtest/gtest.h>

#include <queues/BlockingQueue.h>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <utility>

using namespace std::chrono_literals;

class BlockingQueueTest : public ::testing::Test
{
protected:
    static constexpr std::size_t Capacity = 3;
    BlockingQueue<int, Capacity> queue;
};

struct EmplaceOnlyValue
{
    EmplaceOnlyValue() = default;

    EmplaceOnlyValue(int id, std::string name)
        : id(id), name(std::make_unique<std::string>(std::move(name)))
    {
    }

    EmplaceOnlyValue(const EmplaceOnlyValue&) = delete;
    EmplaceOnlyValue& operator=(const EmplaceOnlyValue&) = delete;
    EmplaceOnlyValue(EmplaceOnlyValue&&) noexcept = default;
    EmplaceOnlyValue& operator=(EmplaceOnlyValue&&) noexcept = default;

    int id{};
    std::unique_ptr<std::string> name;
};

TEST_F(BlockingQueueTest, StartsEmptyWithItsDeclaredCapacity)
{
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());
    EXPECT_EQ(queue.getSize(), 0U);
    EXPECT_EQ(queue.getCapacity(), Capacity);
}

TEST_F(BlockingQueueTest, PushAndPopPreserveFifoOrder)
{
    queue.push(10);
    queue.push(20);
    queue.push(30);

    int value{};
    queue.pop(value);
    EXPECT_EQ(value, 10);
    queue.pop(value);
    EXPECT_EQ(value, 20);
    queue.pop(value);
    EXPECT_EQ(value, 30);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(BlockingQueueTest, TracksSizeAndFullStateAtCapacity)
{
    for (int value = 0; value < static_cast<int>(Capacity); ++value)
    {
        queue.push(value);
        EXPECT_EQ(queue.getSize(), static_cast<std::size_t>(value + 1));
    }

    EXPECT_TRUE(queue.isFull());
    EXPECT_EQ(queue.getSize(), Capacity);

    int value{};
    queue.pop(value);
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(queue.isFull());
    EXPECT_EQ(queue.getSize(), Capacity - 1);
}

TEST_F(BlockingQueueTest, ReusesStorageAfterItemsArePopped)
{
    for (int value = 0; value < static_cast<int>(Capacity); ++value)
    {
        queue.push(value);
    }

    int value{};
    queue.pop(value);
    EXPECT_EQ(value, 0);
    queue.pop(value);
    EXPECT_EQ(value, 1);
    queue.push(3);
    queue.push(4);

    queue.pop(value);
    EXPECT_EQ(value, 2);
    queue.pop(value);
    EXPECT_EQ(value, 3);
    queue.pop(value);
    EXPECT_EQ(value, 4);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueMoveTest, PushAndPopTransferMoveOnlyValues)
{
    BlockingQueue<std::unique_ptr<int>, 1> queue;
    auto input = std::make_unique<int>(42);

    queue.push(std::move(input));
    EXPECT_EQ(input, nullptr);

    std::unique_ptr<int> output;
    queue.pop(output);
    ASSERT_NE(output, nullptr);
    EXPECT_EQ(*output, 42);
}

TEST(BlockingQueueEmplaceTest, ConstructsAndQueuesAMoveOnlyValue)
{
    BlockingQueue<EmplaceOnlyValue, 1> queue;

    ASSERT_TRUE(queue.emplace(7, "compile"));

    EmplaceOnlyValue output;
    ASSERT_TRUE(queue.pop(output));
    EXPECT_EQ(output.id, 7);
    ASSERT_NE(output.name, nullptr);
    EXPECT_EQ(*output.name, "compile");
}

TEST(BlockingQueueEmplaceTest, WaitsForCapacityThenEnqueuesTheValue)
{
    BlockingQueue<int, 1> queue;
    ASSERT_TRUE(queue.emplace(1));

    auto emplaceResult = std::async(std::launch::async, [&queue]
        {
            return queue.emplace(2);
        });

    EXPECT_EQ(emplaceResult.wait_for(50ms), std::future_status::timeout);

    int value{};
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(emplaceResult.get());

    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 2);
}

TEST(BlockingQueueEmplaceTest, ReturnsFalseAfterCloseWithoutAddingAValue)
{
    BlockingQueue<int, 1> queue;
    queue.close();

    EXPECT_FALSE(queue.emplace(42));
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueBlockingTest, PopWaitsUntilAnotherThreadPushesAnItem)
{
    BlockingQueue<int, 1> queue;
    auto popResult = std::async(std::launch::async, [&queue] 
        { 
            int value{};
            queue.pop(value);
            return value;
        });

    EXPECT_EQ(popResult.wait_for(50ms), std::future_status::timeout);

    queue.push(42);
    EXPECT_EQ(popResult.get(), 42);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueBlockingTest, PushWaitsUntilAnotherThreadPopsAnItem)
{
    BlockingQueue<int, 1> queue;
    queue.push(1);

    auto pushResult = std::async(std::launch::async, [&queue]
        {
            queue.push(2);
        });

    EXPECT_EQ(pushResult.wait_for(50ms), std::future_status::timeout);

    int value{};
    queue.pop(value);
    EXPECT_EQ(value, 1);
    pushResult.get();

    queue.pop(value);
    EXPECT_EQ(value, 2);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueCloseTest, CloseUnblocksAWaitingPop)
{
    BlockingQueue<int, 1> queue;
    auto popResult = std::async(std::launch::async, [&queue]
        {
            int value{};
            return queue.pop(value);
        });

    EXPECT_EQ(popResult.wait_for(50ms), std::future_status::timeout);

    queue.close();

    EXPECT_FALSE(popResult.get());
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueCloseTest, CloseAllowsRemainingItemsToBePopped)
{
    BlockingQueue<int, 3> queue;
    ASSERT_TRUE(queue.push(10));
    ASSERT_TRUE(queue.push(20));
    queue.close();

    int value{-1};
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 10);
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 20);

    value = -1;
    EXPECT_FALSE(queue.pop(value));
    EXPECT_EQ(value, -1);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueCloseTest, PopReturnsFalseImmediatelyWhenClosedAndEmpty)
{
    BlockingQueue<int, 1> queue;
    queue.close();

    int value{42};
    EXPECT_FALSE(queue.pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueCloseTest, CloseRejectsMultipleSubsequentPushes)
{
    BlockingQueue<int, 2> queue;
    queue.close();

    int value{10};
    EXPECT_FALSE(queue.push(value));
    EXPECT_FALSE(queue.push(20));
    EXPECT_FALSE(queue.push(30));
    EXPECT_EQ(queue.getSize(), 0U);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BlockingQueueConcurrencyTest, MultipleProducersPreserveItems)
{
    constexpr int ItemCount = 2000;
    constexpr std::size_t Capacity = ItemCount * 2;

    BlockingQueue<int, Capacity> queue;

    std::thread producer1([&]
    {
        for (int i = 0; i < ItemCount; ++i)
            queue.push(i);
    });

    std::thread producer2([&]
    {
        for (int i = 0; i < ItemCount; ++i)
            queue.push(ItemCount + i);
    });

    producer1.join();
    producer2.join();

    std::set<int> values;

    for (int i = 0; i < ItemCount * 2; ++i)
    {
        int value{};
        queue.pop(value);
        values.insert(value);
    }

    EXPECT_EQ(values.size(), ItemCount * 2);
}

TEST(BlockingQueueConcurrencyTest, MultipleConsumersCanPopSameItem)
{
    constexpr int ItemCount = 4000;

    BlockingQueue<int, ItemCount> queue;

    for (int i = 0; i < ItemCount; ++i)
        queue.push(i);

    std::set<int> results;
    std::mutex resultsMutex;

    std::atomic<int> pops{0};

    auto consumer = [&]
    {
        while (true)
        {
            int index = pops.fetch_add(1);

            if (index >= ItemCount)
                return;

            int value{};
            queue.pop(value);

            std::lock_guard lock(resultsMutex);
            results.insert(value);
        }
    };

    std::thread consumer1(consumer);
    std::thread consumer2(consumer);

    consumer1.join();
    consumer2.join();

    EXPECT_EQ(results.size(), ItemCount);
}
