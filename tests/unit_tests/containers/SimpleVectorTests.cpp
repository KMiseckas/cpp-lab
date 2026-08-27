#include <gtest/gtest.h>

#include <containers/SimpleVector.h>

#include <cstddef>
#include <memory>
#include <utility>

namespace
{
struct MoveOnlyValue
{
    explicit MoveOnlyValue(int value)
        : value(value)
    {
    }

    MoveOnlyValue(const MoveOnlyValue&) = delete;
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

    MoveOnlyValue(MoveOnlyValue&&) noexcept = default;
    MoveOnlyValue& operator=(MoveOnlyValue&&) noexcept = default;

    int value;
};

struct LifetimeTrackedValue
{
    explicit LifetimeTrackedValue(int value)
        : value(value)
    {
        ++liveCount;
    }

    LifetimeTrackedValue(const LifetimeTrackedValue& other)
        : value(other.value)
    {
        ++liveCount;
    }

    LifetimeTrackedValue(LifetimeTrackedValue&& other) noexcept
        : value(other.value)
    {
        ++liveCount;
    }

    ~LifetimeTrackedValue()
    {
        --liveCount;
    }

    inline static int liveCount = 0;
    int value;
};
} // namespace

TEST(SimpleVectorTest, StartsEmptyWithTheRequestedCapacity)
{
    SimpleVector<int> values(4U);

    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0U);
    EXPECT_GE(values.capacity(), 4U);
}

TEST(SimpleVectorTest, UsesTheDefaultInitialCapacity)
{
    SimpleVector<int> values;

    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0U);
    EXPECT_EQ(values.capacity(), 8U);
}

TEST(SimpleVectorTest, PushBackAppendsACopyAndExposesItByIndex)
{
    SimpleVector<int> values(2U);
    int first = 11;

    values.push_back(first);
    values.push_back(42);

    EXPECT_FALSE(values.empty());
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0U], 11);
    EXPECT_EQ(values[1U], 42);
}

TEST(SimpleVectorTest, SubscriptReturnsAMutableReference)
{
    SimpleVector<int> values;
    values.push_back(11);

    values[0U] = 99;

    EXPECT_EQ(values[0U], 99);
}

TEST(SimpleVectorTest, ConstSubscriptProvidesReadAccess)
{
    SimpleVector<int> values;
    values.push_back(11);
    const SimpleVector<int>& constValues = values;

    EXPECT_EQ(constValues[0U], 11);
}

TEST(SimpleVectorTest, SupportsMoveOnlyValues)
{
    SimpleVector<MoveOnlyValue> values(1U);

    values.push_back(MoveOnlyValue{3});
    values.push_back(MoveOnlyValue{7});

    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0U].value, 3);
    EXPECT_EQ(values[1U].value, 7);
}

TEST(SimpleVectorTest, GrowsAutomaticallyWithoutLosingOrReorderingValues)
{
    SimpleVector<int> values(1U);

    for (int value = 0; value < 64; ++value)
    {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 64U);
    EXPECT_GE(values.capacity(), values.size());

    for (std::size_t index = 0; index < values.size(); ++index)
    {
        EXPECT_EQ(values[index], static_cast<int>(index));
    }
}

TEST(SimpleVectorTest, GrowsWhenConstructedWithZeroCapacity)
{
    SimpleVector<int> values(0U);

    values.push_back(42);

    EXPECT_EQ(values.size(), 1U);
    EXPECT_GE(values.capacity(), 1U);
    EXPECT_EQ(values[0U], 42);
}

TEST(SimpleVectorTest, ReserveGrowsCapacityAndPreservesContents)
{
    SimpleVector<int> values(2U);
    values.push_back(4);
    values.push_back(9);

    values.reserve(32U);

    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values.capacity(), 32U);
    EXPECT_EQ(values[0U], 4);
    EXPECT_EQ(values[1U], 9);
}

TEST(SimpleVectorTest, ReserveWithASmallerCapacityDoesNotShrink)
{
    SimpleVector<int> values(8U);
    values.push_back(4);

    values.reserve(3U);

    EXPECT_EQ(values.size(), 1U);
    EXPECT_EQ(values.capacity(), 8U);
    EXPECT_EQ(values[0U], 4);
}

TEST(SimpleVectorTest, PopBackRemovesTheLastValueAndLeavesEarlierValues)
{
    SimpleVector<int> values;
    values.push_back(4);
    values.push_back(9);
    values.push_back(15);

    values.pop_back();

    EXPECT_EQ(values.size(), 2U);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ(values[0U], 4);
    EXPECT_EQ(values[1U], 9);
}

TEST(SimpleVectorTest, PopBackOnAnEmptyVectorIsANoOp)
{
    SimpleVector<int> values;

    values.pop_back();

    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0U);
}

TEST(SimpleVectorTest, DestroysRemovedAndRemainingElements)
{
    LifetimeTrackedValue::liveCount = 0;
    LifetimeTrackedValue source{5};

    {
        SimpleVector<LifetimeTrackedValue> values(1U);
        values.push_back(source);
        values.push_back(source);

        EXPECT_EQ(LifetimeTrackedValue::liveCount, 3);

        values.pop_back();

        EXPECT_EQ(LifetimeTrackedValue::liveCount, 2);
    }

    EXPECT_EQ(LifetimeTrackedValue::liveCount, 1);
}
