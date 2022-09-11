#include <winlirc-common/UniqueHandle.h>
#include <functional>
#include <gtest/gtest.h>

using namespace winlirc;

struct TestTraits
{
    using HandleType = int;
    static constexpr HandleType invalidValue() noexcept { return 10; }
    static void close(HandleType h) noexcept { }
};

using TestHandle = UniqueHandle<TestTraits>;

struct UniqueHandleTest : testing::Test
{
    static void SetUpTestSuite() { }
    void SetUp() override { }

};

TEST_F(UniqueHandleTest, default_handle_is_invalid)
{
    TestHandle h;
    EXPECT_FALSE(h);
}

TEST_F(UniqueHandleTest, handle_with_invalid_value_is_not_valid)
{
    TestHandle h{ TestTraits::invalidValue() };
    EXPECT_FALSE(h);
}

TEST_F(UniqueHandleTest, handle_with_valid_value_is_valid)
{
    TestHandle h{ TestTraits::invalidValue()+1 };
    EXPECT_TRUE(h);
}

TEST_F(UniqueHandleTest, release_invalidates)
{
    TestHandle h{ TestTraits::invalidValue() + 1 };
    h.release();
    EXPECT_FALSE(h);
}

TEST_F(UniqueHandleTest, release_returns_old_value)
{
    TestHandle h{ TestTraits::invalidValue() + 1 };
    EXPECT_EQ(TestTraits::invalidValue() + 1, h.release());
}

struct TestTraits2
{
    using HandleType = int;
    static int invalidValue() { return 12; }
    static void close(int h) { ++closeCount; }
    static int closeCount;
};
int TestTraits2::closeCount = 0;

TEST_F(UniqueHandleTest, valid_handle_is_closed)
{
    TestTraits2::closeCount = 0;
    {
        UniqueHandle<TestTraits2> h{ TestTraits2::invalidValue() + 1 };
    }
    EXPECT_EQ(1, TestTraits2::closeCount);
}

TEST_F(UniqueHandleTest, valid_handle_is_not_closed_after_release)
{
    TestTraits2::closeCount = 0;
    {
        UniqueHandle<TestTraits2> h{ TestTraits2::invalidValue() + 1 };
        h.release();
        EXPECT_FALSE(h);
    }
    EXPECT_EQ(0, TestTraits2::closeCount);
}
