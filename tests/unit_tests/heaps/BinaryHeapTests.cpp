#include <gtest/gtest.h>

#include <heaps/BinaryHeap.h>

#include <functional>
#include <vector>

namespace
{
template<typename Heap>
void expectPopOrder(Heap& heap, const std::vector<int>& expected)
{
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        ASSERT_FALSE(heap.empty());
        EXPECT_EQ(heap.top(), expected[index]);
        heap.pop();
        EXPECT_EQ(heap.size(), expected.size() - index - 1U);
    }

    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0U);
}
} // namespace

TEST(BinaryHeapTest, StartsEmpty)
{
    BinaryHeap<int> heap;

    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0U);
}

TEST(BinaryHeapTest, PushExposesTheSmallestValueAndTracksSize)
{
    BinaryHeap<int> heap;

    heap.push(7);
    EXPECT_FALSE(heap.empty());
    EXPECT_EQ(heap.size(), 1U);
    EXPECT_EQ(heap.top(), 7);

    heap.push(3);
    EXPECT_EQ(heap.size(), 2U);
    EXPECT_EQ(heap.top(), 3);

    heap.push(5);
    EXPECT_EQ(heap.size(), 3U);
    EXPECT_EQ(heap.top(), 3);
}

TEST(BinaryHeapTest, PopsValuesInAscendingOrder)
{
    BinaryHeap<int> heap;

    for (int value : {9, 1, 7, 3, 2, 8, 4, 6, 5})
    {
        heap.push(value);
    }

    expectPopOrder(heap, {1, 2, 3, 4, 5, 6, 7, 8, 9});
}

TEST(BinaryHeapTest, PreservesAllEqualValues)
{
    BinaryHeap<int> heap;

    heap.push(4);
    heap.push(4);
    heap.push(4);
    heap.push(4);

    EXPECT_EQ(heap.size(), 4U);
    expectPopOrder(heap, {4, 4, 4, 4});
}

TEST(BinaryHeapTest, SupportsAComparatorForMaxHeapOrdering)
{
    BinaryHeap<int, std::greater<int>> heap;

    for (int value : {2, 8, 3, 8, 1, 5})
    {
        heap.push(value);
    }

    expectPopOrder(heap, {8, 8, 5, 3, 2, 1});
}

TEST(BinaryHeapTest, PopOnAnEmptyHeapIsANoOp)
{
    BinaryHeap<int> heap;

    heap.pop();

    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0U);
}

TEST(BinaryHeapTest, CanBeReusedAfterBeingDrained)
{
    BinaryHeap<int> heap;

    heap.push(3);
    heap.push(1);
    heap.push(2);
    expectPopOrder(heap, {1, 2, 3});

    heap.push(10);
    heap.push(-1);
    heap.push(0);
    expectPopOrder(heap, {-1, 0, 10});
}

TEST(BinaryHeapTest, MaintainsOrderAfterGrowingItsInternalStorage)
{
    constexpr int ValueCount = 2048;
    BinaryHeap<int> heap;

    for (int value = ValueCount - 1; value >= 0; --value)
    {
        heap.push(value);
    }

    EXPECT_EQ(heap.size(), static_cast<std::size_t>(ValueCount));

    for (int expected = 0; expected < ValueCount; ++expected)
    {
        ASSERT_FALSE(heap.empty());
        EXPECT_EQ(heap.top(), expected);
        heap.pop();
    }

    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0U);
}
