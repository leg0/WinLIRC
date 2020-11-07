#include "../config.h"
#include "../ir_remote.h"

#include <Windows.h>
#include <cstdio>
#include <gtest/gtest.h>
#include <string.h>
#include <filesystem>
#include <thread>

using namespace std::string_literals;
using namespace std::string_view_literals;
namespace fs = std::filesystem;

template <typename LinkedList>
size_t count_tail(size_t total, LinkedList const* l)
{
    return (l == nullptr) ? total : count_tail(total + 1, l->next);
};

template <typename LinkedList>
size_t count(LinkedList const* lst)
{
    return count_tail(0, lst);
}

size_t count(ir_ncode const* lst)
{
    size_t count = 0;
    for (; lst->name != nullptr; ++lst)
        ++count;
    return count;
}

struct ConfigParseTest : testing::Test
{
    static std::filesystem::path getDirectory()
    {
        wchar_t filePath[MAX_PATH];
        DWORD const filePathSize = GetModuleFileNameW(nullptr, filePath, std::size(filePath));
        return fs::path{ std::wstring_view{ filePath, filePathSize } }.remove_filename();
    }

    static fs::path getConfFilePath()
    {
        auto p = getDirectory() / testing::UnitTest::GetInstance()->current_test_info()->name();
        p.replace_extension("conf");
        return p;
    }

    void TearDown() override
    {
        auto cfgFile = getConfFilePath();
        if (fs::exists(cfgFile))
            fs::remove(cfgFile);
    }
};

TEST_F(ConfigParseTest, 1)
{
    constexpr auto config = "begin remote\n"
                        " name remote-name\n"
                        " bits 16\n"
                        " flags SPACE_ENC|CONST_LENGTH\n"
                        " eps            30\n"
                        " aeps          100\n"
                        " header       4592  4436\n"
                        " one           614  1626\n"
                        " zero          614   506\n"
                        " ptrail        614\n"
                        " pre_data_bits   16\n"
                        " pre_data       0xE0E0\n"
                        " gap          107991\n"
                        " toggle_bit_mask 0x0\n"
                        " begin codes\n"
                        "  1 0x20DF\n"
                        "  2 0xA05F\n"
                        "  3 0x609F\n"
                        "  4 0x10EF\n"
                        " end codes\n"
                        "end remote\n"sv;
    auto const cfgFile = getConfFilePath();
    FILE* f = _wfopen(cfgFile.c_str(), L"w+");
    fwrite(config.data(), config.size(), 1, f);
    fseek(f, 0, SEEK_SET);
    auto cfg = read_config(f, "my-config.txt");
    fclose(f);

    EXPECT_EQ(1, count(cfg));
    EXPECT_EQ("remote-name"s, cfg->name);
    EXPECT_EQ(16, cfg->bits);
    EXPECT_EQ(SPACE_ENC | CONST_LENGTH, cfg->flags);
    EXPECT_EQ(30, cfg->eps);
    EXPECT_EQ(100, cfg->aeps);
    EXPECT_EQ(614, cfg->pone);
    EXPECT_EQ(1626, cfg->sone);
    EXPECT_EQ(614, cfg->pzero);
    EXPECT_EQ(506, cfg->szero);
    EXPECT_EQ(614, cfg->ptrail);
    EXPECT_EQ(16, cfg->pre_data_bits);
    EXPECT_EQ(0xE0E0, cfg->pre_data);
    EXPECT_EQ(107991, cfg->gap);
    EXPECT_EQ(0, cfg->toggle_bit_mask);

    EXPECT_EQ(4, cfg->codes.size());
    auto code = begin(cfg->codes);
    EXPECT_EQ("1"s, code->name);
    EXPECT_EQ(0x20DF, code->code);
    ++code;
    EXPECT_EQ("2"s, code->name);
    EXPECT_EQ(0xA05F, code->code);
    ++code;
    EXPECT_EQ("3"s, code->name);
    EXPECT_EQ(0x609F, code->code);
    ++code;
    EXPECT_EQ("4"s, code->name);
    EXPECT_EQ(0x10EF, code->code);
    ++code;
    EXPECT_EQ(nullptr, code->name);

    free_config(cfg);
}
