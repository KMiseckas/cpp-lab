#include <gtest/gtest.h>

#include <ring_buffers/SPSCRingBuffer.h>

#include <atomic>
#include <barrier>
#include <cstddef>
#include <memory>
#include <thread>

class SPSCRingBufferTest : public ::testing::Test
{
protected:
    static constexpr std::size_t Capacity = 5;
    SPSCRingBuffer<int, Capacity> buffer;
};

TEST_F(SPSCRingBufferTest, StartsEmpty)
{
    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(buffer.getCount(), 0U);

    int value{};
    EXPECT_FALSE(buffer.pop(value));
}

TEST_F(SPSCRingBufferTest, PushPopPreservesFifoOrderAndFullBoundary)
{
    // One slot is deliberately left unused so head == tail means empty.
    for (int value = 0; value < static_cast<int>(Capacity - 1); ++value)
    {
        EXPECT_TRUE(buffer.push(value));
    }

    EXPECT_TRUE(buffer.isFull());
    EXPECT_FALSE(buffer.push(99));

    for (int expected = 0; expected < static_cast<int>(Capacity - 1); ++expected)
    {
        int actual{};
        ASSERT_TRUE(buffer.pop(actual));
        EXPECT_EQ(actual, expected);
    }

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.getCount(), 0U);
}

TEST_F(SPSCRingBufferTest, WrapsAroundWithoutLosingOrder)
{
    for (int value = 0; value < 100; ++value)
    {
        while (!buffer.push(value))
        {
            int discarded{};
            ASSERT_TRUE(buffer.pop(discarded));
            EXPECT_EQ(discarded, value - static_cast<int>(Capacity - 1));
        }
    }

    for (int expected = 100 - static_cast<int>(Capacity - 1); expected < 100; ++expected)
    {
        int actual{};
        ASSERT_TRUE(buffer.pop(actual));
        EXPECT_EQ(actual, expected);
    }
}

TEST(SPSCRingBufferMoveTest, TransfersMoveOnlyValues)
{
    SPSCRingBuffer<std::unique_ptr<int>, 2> buffer;
    auto input = std::make_unique<int>(42);

    ASSERT_TRUE(buffer.push(std::move(input)));
    EXPECT_EQ(input, nullptr);

    std::unique_ptr<int> output;
    ASSERT_TRUE(buffer.pop(output));
    ASSERT_NE(output, nullptr);
    EXPECT_EQ(*output, 42);
}

TEST(SPSCRingBufferConcurrencyTest, TransfersOrderedValuesUnderContention)
{
    constexpr std::size_t capacity = 64;
    constexpr int itemCount = 200'000;
    SPSCRingBuffer<int, capacity> buffer;

    std::barrier start{2};
    std::atomic<bool> valuesWereOrdered{true};

    std::jthread producer([&] {
        start.arrive_and_wait();
        for (int value = 0; value < itemCount; ++value)
        {
            while (!buffer.push(value))
            {
                std::this_thread::yield();
            }
        }
    });

    std::jthread consumer([&] {
        start.arrive_and_wait();
        for (int expected = 0; expected < itemCount; ++expected)
        {
            int actual{};
            while (!buffer.pop(actual))
            {
                std::this_thread::yield();
            }

            if (actual != expected)
            {
                valuesWereOrdered.store(false, std::memory_order_relaxed);
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(valuesWereOrdered.load(std::memory_order_relaxed));
    EXPECT_TRUE(buffer.isEmpty());
}
