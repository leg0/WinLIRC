#include "../dump_config.h"
#include "../../winlirc/constants.h"

#include <gtest/gtest.h>

TEST(DumpConfigTest, No_flags)
{
    FILE* f = tmpfile();
    fprint_flags(f, 0);
    fpos_t pos;
    fgetpos(f, &pos);
    EXPECT_EQ(0, pos);
    fclose(f);
}

TEST(DumpConfigTest, One_flag_1)
{
    FILE* f = tmpfile();
    fpos_t startPos;
    fgetpos(f, &startPos);
    fprint_flags(f, RAW_CODES);
    fpos_t pos;
    fgetpos(f, &pos);
    EXPECT_NE(0, pos);
    fsetpos(f, &startPos);

    char buf[1024] = "";
    auto bytesRead = fread(buf, 1, pos, f);
    EXPECT_EQ(pos, bytesRead);
    EXPECT_STREQ("  flags RAW_CODES\n", buf);
    fclose(f);
}

TEST(DumpConfigTest, One_flag_2)
{
    FILE* f = tmpfile();
    fpos_t startPos;
    fgetpos(f, &startPos);
    fprint_flags(f, SERIAL);
    fpos_t pos;
    fgetpos(f, &pos);
    EXPECT_NE(0, pos);
    fsetpos(f, &startPos);

    char buf[1024] = "";
    auto bytesRead = fread(buf, 1, pos, f);
    EXPECT_EQ(pos, bytesRead);
    EXPECT_STREQ("  flags SERIAL\n", buf);
    fclose(f);
}

TEST(DumpConfigTest, TwoFlags)
{
    FILE* f = tmpfile();
    fpos_t startPos;
    fgetpos(f, &startPos);
    fprint_flags(f, XMP|SPACE_FIRST);
    fpos_t pos;
    fgetpos(f, &pos);
    EXPECT_NE(0, pos);
    fsetpos(f, &startPos);

    char buf[1024] = "";
    auto bytesRead = fread(buf, 1, pos, f);
    EXPECT_EQ(pos, bytesRead);
    EXPECT_STREQ("  flags SPACE_FIRST|XMP\n", buf);
    fclose(f);
}