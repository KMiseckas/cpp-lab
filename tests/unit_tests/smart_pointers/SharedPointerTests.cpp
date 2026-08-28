#include <gtest/gtest.h>

#include <memory>
#include <smart_pointers/SharedPointer.h>

#include <utility>

namespace
{
struct Widget
{
    Widget(int id, int value)
        : id(id)
        , value(value)
    {
    }

    int id;
    int value;
};

struct LifetimeTrackedWidget
{
    explicit LifetimeTrackedWidget(int value)
        : value(value)
    {
        ++liveCount;
    }

    ~LifetimeTrackedWidget()
    {
        --liveCount;
        ++destructionCount;
    }

    inline static int liveCount{0};
    inline static int destructionCount{0};

    int value;
};
} // namespace

TEST(SharedPointerTest, DefaultConstructedPointerIsEmpty)
{
    SharedPointer<int> pointer;

    EXPECT_EQ(pointer.get(), nullptr);
}

TEST(SharedPointerTest, MakeSharedConstructsTheValueWithForwardedArguments)
{
    auto pointer = make_shared<Widget>(7, 42);

    ASSERT_NE(pointer.get(), nullptr);
    EXPECT_EQ(pointer.get()->id, 7);
    EXPECT_EQ(pointer.get()->value, 42);
}

TEST(SharedPointerTest, ArrowOperatorProvidesMutableAccessToTheValue)
{
    auto pointer = make_shared<Widget>(7, 42);

    pointer.get()->value = 99;

    EXPECT_EQ(pointer.get()->value, 99);
}

TEST(SharedPointerTest, ConstPointerProvidesReadOnlyAccessToTheValue)
{
    const auto pointer = make_shared<Widget>(7, 42);

    EXPECT_EQ(pointer.get()->id, 7);
    EXPECT_EQ(pointer.get()->value, 42);
}

TEST(SharedPointerTest, CopyConstructionSharesTheSameValue)
{
    auto original = make_shared<Widget>(7, 42);
    SharedPointer<Widget> copy{original};

    EXPECT_EQ(copy.get(), original.get());
    (*copy).value = 99;
    EXPECT_EQ(original.get()->value, 99);
}

TEST(SharedPointerTest, CopyAssignmentReleasesItsPreviousValueAndSharesTheNewValue)
{
    LifetimeTrackedWidget::liveCount = 0;
    LifetimeTrackedWidget::destructionCount = 0;

    {
        auto source = make_shared<LifetimeTrackedWidget>(1);
        auto destination = make_shared<LifetimeTrackedWidget>(2);

        destination = source;

        EXPECT_EQ(destination.get(), source.get());
        EXPECT_EQ(LifetimeTrackedWidget::liveCount, 1);
        EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 1);
    }

    EXPECT_EQ(LifetimeTrackedWidget::liveCount, 0);
    EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 2);
}

TEST(SharedPointerTest, SelfCopyAssignmentKeepsTheValueAlive)
{
    auto pointer = make_shared<Widget>(7, 42);
    auto* const value = pointer.get();

    pointer = pointer;

    EXPECT_EQ(pointer.get(), value);
    EXPECT_EQ(pointer.get()->value, 42);
}

TEST(SharedPointerTest, MoveConstructionTransfersOwnershipAndEmptiesTheSource)
{
    auto source = make_shared<Widget>(7, 42);
    auto* const value = source.get();
    SharedPointer<Widget> destination{std::move(source)};

    EXPECT_EQ(destination.get(), value);
    EXPECT_EQ(source.get(), nullptr);
    EXPECT_EQ(destination.get()->value, 42);
}

TEST(SharedPointerTest, MoveAssignmentReleasesItsPreviousValueAndEmptiesTheSource)
{
    LifetimeTrackedWidget::liveCount = 0;
    LifetimeTrackedWidget::destructionCount = 0;

    {
        auto source = make_shared<LifetimeTrackedWidget>(1);
        auto destination = make_shared<LifetimeTrackedWidget>(2);
        auto* const value = source.get();

        destination = std::move(source);

        EXPECT_EQ(destination.get(), value);
        EXPECT_EQ(source.get(), nullptr);
        EXPECT_EQ(LifetimeTrackedWidget::liveCount, 1);
        EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 1);
    }

    EXPECT_EQ(LifetimeTrackedWidget::liveCount, 0);
    EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 2);
}

TEST(SharedPointerTest, DestroysTheManagedValueOnceAfterTheLastOwnerIsDestroyed)
{
    LifetimeTrackedWidget::liveCount = 0;
    LifetimeTrackedWidget::destructionCount = 0;

    {
        auto first = make_shared<LifetimeTrackedWidget>(42);
        {
            SharedPointer<LifetimeTrackedWidget> second{first};

            EXPECT_EQ(LifetimeTrackedWidget::liveCount, 1);
            EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 0);
        }

        EXPECT_EQ(LifetimeTrackedWidget::liveCount, 1);
        EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 0);
    }

    EXPECT_EQ(LifetimeTrackedWidget::liveCount, 0);
    EXPECT_EQ(LifetimeTrackedWidget::destructionCount, 1);
}
