#include "../wl_string.h"

#include <gtest/gtest.h>

using namespace winlirc;
TEST(Tokenize, without_separators)
{
    istring_view s        = "123456789";
    auto const   expected = s;
    auto         z        = strtok(s, " \r\n");
    EXPECT_EQ(expected, z);
    EXPECT_TRUE(s.empty());
}

TEST(Tokenize, one_separator)
{
    istring_view s = "1234 56789";
    auto         z = strtok(s, " \r\n");
    EXPECT_EQ("1234", z);
    EXPECT_EQ("56789", s);
}

TEST(Tokenize, repeated_separator)
{
    istring_view s = "1234 \r \n \r\r  56789";
    auto         z = strtok(s, " \r\n");
    EXPECT_EQ("1234", z);
    EXPECT_EQ("56789", s);
}

TEST(Tokenize, multiple_tokens)
{
    istring_view s = "1 2 3 4 5\r6\r7\r8\n9";
    std::vector<istring_view> const expecteds{ "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    for (auto expected : expecteds) {
        auto z = strtok(s, " \r\n");
        EXPECT_EQ(expected, z);
    }
    EXPECT_TRUE(s.empty());
}

TEST(Tokenize, start_with_separator)
{
    istring_view s        = "\n \r 1234";
    auto         actual   = strtok(s, " \r\n");
    EXPECT_TRUE(s.empty());
    EXPECT_EQ("1234", actual);
}

TEST(Tokenize, end_with_separator)
{
    istring_view s      = "1234 ";
    auto         actual = strtok(s, " \r\n");
    EXPECT_TRUE(s.empty());
    EXPECT_EQ("1234", actual);
}

TEST(Tokenize, separators_only)
{
    istring_view s      = "\n \r      ";
    auto         actual = strtok(s, " \r\n");
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(actual.empty());
}

TEST(Trim, empty_string)
{
    istring_view s = "";
    EXPECT_EQ("", rtrim(s, " \r\n"));
}

TEST(Trim, only_spaces)
{
    istring_view s = "    ";
    EXPECT_EQ("", rtrim(s, " \r\n"));
}

TEST(Trim, ends_with_one_space)
{
    istring_view s = "x ";
    EXPECT_EQ("x", rtrim(s, " \r\n"));
}

TEST(Trim, ends_with_two_spaces)
{
    istring_view s = "   x  ";
    EXPECT_EQ("   x", rtrim(s, " \r\n"));
}
