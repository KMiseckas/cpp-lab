#include <gtest/gtest.h>
#include <ring_buffers/RingBuffer.h>

class RingBufferTest : public ::testing::Test
{
protected:
    static constexpr std::size_t BUFFER_SIZE = 5;
    using TestBuffer = RingBuffer<int, BUFFER_SIZE>;

    TestBuffer buffer;
};

// Test basic push and pop operations
TEST_F(RingBufferTest, PushAndPopSingleElement)
{
    int value = 42;
    EXPECT_TRUE(buffer.push(value));
    
    int retrieved = 0;
    EXPECT_TRUE(buffer.pop(retrieved));
    EXPECT_EQ(retrieved, 42);
}

// Test empty buffer state
TEST_F(RingBufferTest, EmptyBufferInitially)
{
    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.getCount(), 0);
}

// Test pushing multiple elements
TEST_F(RingBufferTest, PushMultipleElements)
{
    for (int i = 1; i <= 4; ++i)
    {
        EXPECT_TRUE(buffer.push(i));
    }
    EXPECT_EQ(buffer.getCount(), 4);
}

// Test FIFO order
TEST_F(RingBufferTest, FIFOOrder)
{
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    int val;
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 3);
}

// Test full buffer detection
TEST_F(RingBufferTest, FullBufferDetection)
{
    for (int i = 0; i < BUFFER_SIZE - 1; ++i)
    {
        buffer.push(i);
    }
    EXPECT_TRUE(buffer.isFull());
}

// Test push to full buffer returns false
TEST_F(RingBufferTest, PushToFullBufferFails)
{
    for (int i = 0; i < BUFFER_SIZE - 1; ++i)
    {
        buffer.push(i);
    }
    EXPECT_FALSE(buffer.push(999));
}

// Test pop from empty buffer returns false
TEST_F(RingBufferTest, PopFromEmptyBufferFails)
{
    int value;
    EXPECT_FALSE(buffer.pop(value));
}

// Test circular wrapping behavior
TEST_F(RingBufferTest, CircularWrappingBehavior)
{
    // Fill and empty multiple times to test wrap-around
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        for (int i = 0; i < BUFFER_SIZE - 1; ++i)
        {
            EXPECT_TRUE(buffer.push(i + cycle * 10));
        }

        int val;
        for (int i = 0; i < BUFFER_SIZE - 1; ++i)
        {
            EXPECT_TRUE(buffer.pop(val));
            EXPECT_EQ(val, i + cycle * 10);
        }
    }
}

// Test rvalue reference push (move semantics)
TEST_F(RingBufferTest, MoveSemanticsPush)
{
    int tempValue = 99;
    EXPECT_TRUE(buffer.push(std::move(tempValue)));
    
    int retrieved;
    EXPECT_TRUE(buffer.pop(retrieved));
    EXPECT_EQ(retrieved, 99);
}

// Test count tracking
TEST_F(RingBufferTest, CountTracking)
{
    EXPECT_EQ(buffer.getCount(), 0);
    
    buffer.push(1);
    EXPECT_EQ(buffer.getCount(), 1);
    
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.getCount(), 3);
    
    int val;
    buffer.pop(val);
    EXPECT_EQ(buffer.getCount(), 2);
}

// Test alternating push and pop
TEST_F(RingBufferTest, AlternatingPushPop)
{
    buffer.push(10);
    buffer.push(20);
    
    int val;
    buffer.pop(val);
    EXPECT_EQ(val, 10);
    
    buffer.push(30);
    buffer.push(40);
    
    buffer.pop(val);
    EXPECT_EQ(val, 20);
    
    buffer.pop(val);
    EXPECT_EQ(val, 30);
}

// Test buffer with single element capacity conceptually
TEST_F(RingBufferTest, FullCapacityBoundary)
{
    for (int i = 0; i < BUFFER_SIZE - 1; ++i)
    {
        EXPECT_TRUE(buffer.push(i));
    }
    
    EXPECT_TRUE(buffer.isFull());
    
    int val;
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_FALSE(buffer.isFull());
}
