#include <winlirc/winlirc_api.h>
#include <gtest/gtest.h>
#include <intrin.h>

TEST(Remote, bits_set)
{
    EXPECT_EQ(0, bits_set(0));
    EXPECT_EQ(32, bits_set(0xFFFF'FFFF));
    EXPECT_EQ(32, bits_set(0xFFFF'FFFF'0000'0000));
    EXPECT_EQ(16, bits_set(0x1111'1111'1111'1111));
    EXPECT_EQ(64, bits_set(~0));
}

TEST(Remote, reverse)
{
    EXPECT_EQ(0, reverse(0b0000, 0));
    EXPECT_EQ(0, reverse(0b0001, 0));
    EXPECT_EQ(0, reverse(~0, 0));
    EXPECT_EQ(0b000'0, reverse(0b000'0, 1));
    EXPECT_EQ(0b000'1, reverse(0b000'1, 1));
    EXPECT_EQ(0b000'1, reverse(0b111'1, 1));
    EXPECT_EQ(0b00'00, reverse(0b00'00, 2));
    EXPECT_EQ(0b00'10, reverse(0b00'01, 2));
    EXPECT_EQ(0b00'11, reverse(0b11'11, 2));
    EXPECT_EQ(0b00'01, reverse(0b11'10, 2));

    EXPECT_EQ(0b00'00000000000000000000, reverse(0b10'00000000000000000000, 20));
    EXPECT_EQ(0b00'10000000000000000000, reverse(0b01'00000000000000000001, 20));
    EXPECT_EQ(0b00'01101000000000000000, reverse(0b00'00000000000000010110, 20));
}

TEST(Remote, gen_mask)
{
    EXPECT_EQ(0x0000000000000000, gen_mask(0));
    EXPECT_EQ(0x0000000000000001, gen_mask(1));
    EXPECT_EQ(0x0000000000000003, gen_mask(2));
    EXPECT_EQ(0x0000000000000007, gen_mask(3));
    EXPECT_EQ(0x000000000000000F, gen_mask(4));
    EXPECT_EQ(0x000000000000001F, gen_mask(5));
    EXPECT_EQ(0x000000000000003F, gen_mask(6));
    EXPECT_EQ(0x000000000000007F, gen_mask(7));
    EXPECT_EQ(0x00000000000000FF, gen_mask(8));
    EXPECT_EQ(0x000000000000FFFF, gen_mask(16));
    EXPECT_EQ(0x0000000001FFFFFF, gen_mask(25));
    EXPECT_EQ(0x00000000FFFFFFFF, gen_mask(32));
    EXPECT_EQ(0x0000007FFFFFFFFF, gen_mask(39));
    EXPECT_EQ(0xFFFFFFFFFFFFFFFF, gen_mask(64));
}

TEST(Remote, gen_mask2)
{
    for (int i = 0; i < 64; ++i)
    {
        EXPECT_EQ(gen_mask(i), reverse(gen_mask(i), i));
        EXPECT_EQ(i, bits_set(gen_mask(i)));
    }
}