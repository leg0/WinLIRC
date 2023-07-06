#include <winlirc/winlirc_api.h>
#include <cassert>
#include <ctype.h>
#include <errno.h>
#include <expected>
#include <span>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "remote.h"
#include "config.h"

#include "wl_string.h"
#include <charconv>
#include <filesystem>

enum directive { ID_none, ID_remote, ID_codes, ID_raw_codes, ID_raw_name };

static constexpr size_t LINE_LEN = 1024;
static constexpr uint32_t MAX_INCLUDES = 10;
static constexpr char whitespace[] = " \t";
static int g_line;
//static int g_parse_error;

static std::unique_ptr<ir_remote> read_config_recursive(FILE* f, winlirc::istring_view name, int depth);

template <typename Int>
    requires(std::is_integral_v<Int>)
static std::expected<Int, std::error_code> s_str_to_int(std::span<char const> s)
{
    auto const base = [&]() {
        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            s = s.subspan(2);
            return 16;
        }
        return 10;
    }();
    Int out;
    auto end = s.data() + s.size();
    auto res = std::from_chars(s.data(), end, out, base);
    if (res.ec == std::errc{}&& res.ptr == end)
        return out;

    return std::unexpected{std::make_error_code(std::errc::invalid_argument)};
}

static std::expected<ir_code, std::error_code> s_strtocode(std::span<char const> val)
{
    return s_str_to_int<ir_code>(val);
}

static std::expected<unsigned long, std::error_code> s_strtoul(std::span<char const> val)
{
    return s_str_to_int<unsigned long>(val);
}

static std::expected<int, std::error_code> s_strtoi(std::span<char const> val)
{
    return s_str_to_int<int>(val);
}

static std::expected<unsigned int, std::error_code> s_strtoui(std::span<char const> val)
{
    return s_str_to_int<unsigned int>(val);
}

static std::expected<lirc_t, std::error_code> s_strtolirc_t(std::span<char const> val)
{
    return s_str_to_int<lirc_t>(val);
}

static std::expected<void, std::error_code> checkMode(int is_mode, int c_mode, char const* error) noexcept
{
    if (is_mode != c_mode)
        return std::unexpected{ std::make_error_code(std::errc::state_not_recoverable) };
    return {};
}

static std::expected<void, std::error_code> addSignal(std::vector<lirc_t>& signals, winlirc::istring_view val) noexcept
{
    auto t = s_strtolirc_t(val);
    if (!t)
        return std::unexpected{t.error()};
    signals.push_back(*t);
    return {};
}

static std::expected<ir_ncode*, std::error_code> defineCode(winlirc::istring_view key, winlirc::istring_view val, ir_ncode* code) noexcept
{
    *code = ir_ncode{};
    code->name = std::string{ begin(key), end(key) };
    if (auto c = s_strtocode(val))
    {
        code->code = *c;
        return code;
    }
    else
    {
        return std::unexpected{c.error()};
    }
}

static std::expected<ir_code_node*, std::error_code> defineNode(ir_ncode* code, winlirc::istring_view val) noexcept
{
    if (auto c = s_strtocode(val))
    {
        auto node = std::make_unique<ir_code_node>();
        auto const res = node.get();
        node->code = *c;
        node->next = nullptr;

        if (code->current == nullptr)
        {
            code->current = node.get();
            code->next = std::move(node);
        }
        else
        {
            code->current = node.get();
            code->current->next = std::move(node);
        }
        return res;
    }
    else
    {
        return std::unexpected{c.error()};
    }
}

static std::expected<int, std::error_code> parseFlags(winlirc::istring_view val) noexcept
{
    int flags = 0;
    while (!val.empty())
    {
        auto flagName = strtok(val, "|");
        auto flagIt = std::find_if(std::begin(all_flags), std::end(all_flags), [&](auto& fp) {
            return fp.name == flagName;
        });
        if (flagIt == std::end(all_flags) ||
            (flagIt->flag & IR_PROTOCOL_MASK) && (flags & IR_PROTOCOL_MASK))
        {
            return std::unexpected{std::make_error_code(std::errc::invalid_argument)};
        }
        flags = flags | flagIt->flag;
    }
    return flags;
}

template <typename T, typename ParsedValue>
static std::expected<void, std::error_code> parse_field(
    T ir_remote::* member,
    ParsedValue val,
    ir_remote* rem) noexcept
{
    if (val) {
        rem->*member = *val;
        return {};
    }
    return std::unexpected{val.error()};
}

template <typename T, typename E, typename Fun>
requires (std::is_same_v<void, T> && std::is_invocable_v<Fun> ||
    !std::is_same_v<void, T> && std::is_invocable_v<Fun, T>)
std::expected<T, E> operator&&(std::expected<T, E>&& lhs, Fun&& fun)
{
    if (lhs) {
        if constexpr (std::is_same_v<void, T>)
            return fun();
        else
            return fun(std::move(*lhs));
    }
    return lhs;
}

static std::expected<void, std::error_code> defineRemote(
    winlirc::istring_view key,
    winlirc::istring_view val,
    winlirc::istring_view val2,
    ir_remote* rem) noexcept
{
    if ("name" == key) {
        rem->name.assign(val.begin(), val.end());
        return {};
    }
    else if ("bits" == key) {
        return parse_field(&ir_remote::bits, s_strtoi(val), rem);
    }
    else if ("flags" == key) {
        return parse_field(&ir_remote::flags, parseFlags(val), rem);
    }
    else if ("eps" == key) {
        return parse_field(&ir_remote::eps, s_strtoi(val), rem);
    }
    else if ("aeps" == key) {
        return parse_field(&ir_remote::aeps, s_strtoi(val), rem);
    }
    else if ("plead" == key) {
        return parse_field(&ir_remote::plead, s_strtolirc_t(val), rem);
    }
    else if ("ptrail" == key) {
        return parse_field(&ir_remote::ptrail, s_strtolirc_t(val), rem);
    }
    else if ("pre_data_bits" == key) {
        return parse_field(&ir_remote::pre_data_bits, s_strtoi(val), rem);
    }
    else if ("pre_data" == key) {
        return parse_field(&ir_remote::pre_data, s_strtocode(val), rem);
    }
    else if ("post_data_bits" == key) {
        return parse_field(&ir_remote::post_data_bits, s_strtoi(val), rem);
    }
    else if ("post_data" == key) {
        return parse_field(&ir_remote::post_data, s_strtocode(val), rem);
    }
    else if ("gap" == key) {
        if (!val2.empty()) {
            if (auto f = parse_field(&ir_remote::gap2, s_strtoul(val2), rem); !f)
                return f;
        }
        return parse_field(&ir_remote::gap, s_strtoul(val), rem);
    }
    else if ("repeat_gap" == key) {
        return parse_field(&ir_remote::repeat_gap, s_strtoul(val), rem);
    }
    else if ("toggle_bit" == key
        || "repeat_bit" == key/* obsolete name */) {
        /* obsolete: use toggle_bit_mask instead */
        return parse_field(&ir_remote::toggle_bit, s_strtoi(val), rem);
    }
    else if ("toggle_bit_mask" == key) {
        return parse_field(&ir_remote::toggle_bit_mask, s_strtocode(val), rem);
    }
    else if ("toggle_mask" == key) {
        return parse_field(&ir_remote::toggle_mask, s_strtocode(val), rem);
    }
    else if ("rc6_mask" == key) {
        return parse_field(&ir_remote::rc6_mask, s_strtocode(val), rem);
    }
    else if ("ignore_mask" == key) {
        return parse_field(&ir_remote::ignore_mask, s_strtocode(val), rem);
    }
    else if ("suppress_repeat" == key) {
        //rem->suppress_repeat=s_strtoi(val);	//TODO support this per remote
        return {};
    }
    else if ("min_repeat" == key) {
        return parse_field(&ir_remote::min_repeat, s_strtoi(val), rem);
    }
    else if ("min_code_repeat" == key) {
        return parse_field(&ir_remote::min_code_repeat, s_strtoi(val), rem);
    }
    else if ("frequency" == key) {
        return parse_field(&ir_remote::freq, s_strtoui(val), rem);
    }
    else if ("duty_cycle" == key) {
        return parse_field(&ir_remote::duty_cycle, s_strtoui(val), rem);
    }
    else if ("baud" == key) {
        return parse_field(&ir_remote::baud, s_strtoui(val), rem);
    }
    else if ("serial_mode" == key) {
        if (val[0] < '5' || '9' < val[0])
        {
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        }
        rem->bits_in_byte = val[0] - '0';
        switch (val[1])
        {
        case 'n': case 'N':
            rem->parity = IR_PARITY_NONE;
            break;
        case 'e': case 'E':
            rem->parity = IR_PARITY_EVEN;
            break;
        case 'o': case 'O':
            rem->parity = IR_PARITY_ODD;
            break;
        default:
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        }
        if (val.substr(2) == "1.5")
            rem->stop_bits = 3;
        else if (auto v = s_strtoui(val.substr(2)))
            rem->stop_bits = *v;
        else
            return std::unexpected(v.error());
        return {};
    }
    else if (!val2.empty())
    {
        if ("header" == key) {
            return parse_field(&ir_remote::phead, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::shead, s_strtolirc_t(val2), rem); };
        }
        else if ("three" == key) {
            return parse_field(&ir_remote::pthree, s_strtolirc_t(val), rem)
                &&  [&] { return parse_field(&ir_remote::sthree, s_strtolirc_t(val2), rem); };
        }
        else if ("two" == key) {
            return parse_field(&ir_remote::ptwo, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::stwo, s_strtolirc_t(val2), rem); };
        }
        else if ("one" == key) {
            return parse_field(&ir_remote::pone, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::sone, s_strtolirc_t(val2), rem); };
        }
        else if ("zero" == key) {
            return parse_field(&ir_remote::pzero, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::szero, s_strtolirc_t(val2), rem); };
        }
        else if ("foot" == key) {
            return parse_field(&ir_remote::pfoot, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::sfoot, s_strtolirc_t(val2), rem); };
        }
        else if ("repeat" == key) {
            return parse_field(&ir_remote::prepeat, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::srepeat, s_strtolirc_t(val2), rem); };
        }
        else if ("pre" == key) {
            return parse_field(&ir_remote::pre_p, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::pre_s, s_strtolirc_t(val2), rem); };
        }
        else if ("post" == key) {
            return parse_field(&ir_remote::post_p, s_strtolirc_t(val), rem)
                && [&] { return parse_field(&ir_remote::post_s, s_strtolirc_t(val2), rem); };
        }
    }
    return std::unexpected{std::make_error_code(std::errc::invalid_argument)};
}

static int sanityChecks(ir_remote* rem)
{
    if (rem->name.empty())
    {
        return 0;
    }

    if (is_raw(rem)) return 1;

    if ((rem->pre_data & gen_mask(rem->pre_data_bits)) != rem->pre_data)
    {
        rem->pre_data &= gen_mask(rem->pre_data_bits);
    }
    if ((rem->post_data & gen_mask(rem->post_data_bits)) != rem->post_data)
    {
        rem->post_data &= gen_mask(rem->post_data_bits);
    }
    for (auto codes = rem->codes.begin(); codes != rem->codes.end(); codes++)
    {
        if ((codes->code & gen_mask(rem->bits)) != codes->code)
        {
            codes->code &= gen_mask(rem->bits);
        }
        for (auto node = codes->next.get(); node != nullptr; node = node->next.get())
        {
            if ((node->code & gen_mask(rem->bits)) != node->code)
            {
                node->code &= gen_mask(rem->bits);
            }
        }
    }

    return 1;
}

static std::unique_ptr<ir_remote> sort_by_bit_count(std::unique_ptr<ir_remote> remotes) noexcept
{
    assert(remotes);
    std::vector<std::unique_ptr<ir_remote>> v;
    while (remotes)
    {
        v.push_back(std::exchange(remotes, std::move(remotes->next)));
    }
    std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return bit_count(a.get()) < bit_count(b.get()); });

    for (size_t i = v.size() - 2; i != ~size_t{}; --i)
    {
        v[i]->next = std::move(v[i + 1]);
    }
    return std::move(v[0]);
}

static winlirc::istring_view lirc_parse_include(winlirc::istring_view s)
{
    s = winlirc::rtrim(s, whitespace);
    if (!s.empty() && (s.front() == '"' && s.back() == '"' || s.front() == '<' && s.back() == '>'))
    {
        s.remove_prefix(1);
        s.remove_suffix(1);
        return s;
    }
    return {};
}

static winlirc::istring lirc_parse_relative(winlirc::istring_view child, winlirc::istring_view current)
{
    if (current.empty())
        return static_cast<winlirc::istring>(child);

    std::filesystem::path p{ child };
    if (p.is_absolute())
        return static_cast<winlirc::istring>(child);

    auto cur = std::filesystem::path{ current }.remove_filename() / child;
    return cur.string().c_str();
}

std::unique_ptr<ir_remote> read_config(FILE* f, const char* name)
{
    return read_config_recursive(f, name, 0);
}

static std::unique_ptr<ir_remote> read_config_recursive(FILE* f, winlirc::istring_view name, int depth)
{
    char bufx[LINE_LEN + 1];
    winlirc::istring_view key, val, val2;
    std::unique_ptr<ir_remote> top_rem;
    ir_remote* rem = nullptr;
    std::vector<ir_ncode> codes_list, raw_codes;
    std::vector<lirc_t> signals;
    ir_ncode raw_code{};
    ir_ncode name_code{};
    ir_ncode* code;
    int mode = ID_none;

    g_line = 0;

    while (fgets(bufx, LINE_LEN, f) != nullptr)
    {
        winlirc::istring_view buf{ bufx };
        g_line++;
        if (buf.size() == LINE_LEN && buf.back() != '\n')
        {
            return nullptr;
        }

        if (!buf.empty() && buf.back() == '\n')
            buf.remove_suffix(1);
        if (!buf.empty() && buf.back() == '\r')
            buf.remove_suffix(1);
        /* ignore comments */
        if (!buf.empty() && buf[0] == '#')
            continue;

        key = strtok(buf, whitespace);
        /* ignore empty lines */
        if (key.empty()) continue;
        val = strtok(buf, whitespace);
        if (!val.empty()) {
            val2 = strtok(buf, whitespace);

            if ("include" == key) {
                if (depth > MAX_INCLUDES) {
                    return nullptr;
                }

                auto childName = lirc_parse_include(val);
                if (childName.empty()) {
                    return nullptr;
                }

                auto fullPath = lirc_parse_relative(childName, name);
                if (fullPath.empty()) {
                    return nullptr;
                }

                FILE* childFile;
                auto const openResult = fopen_s(&childFile, fullPath.c_str(), "r");
                if (childFile != nullptr)
                {
                    int save_line = g_line;

                    if (!top_rem) {
                        /* create first remote */
                        top_rem = read_config_recursive(childFile, fullPath, depth + 1);
                        rem = top_rem.get();
                    }
                    else {
                        /* create new remote */

                        rem->next = read_config_recursive(childFile, fullPath, depth + 1);
                        rem = rem->next.get();
                    }
                    fclose(childFile);
                    g_line = save_line;
                }
            }
            else if ("begin" == key) {
                if ("codes" == val) {
                    /* init codes mode */

                    if (!checkMode(mode, ID_remote,
                        "begin codes")) break;
                    if (!rem->codes.empty()) {
                        return nullptr;
                    }

                    codes_list.clear();
                    codes_list.reserve(30);
                    mode = ID_codes;
                }
                else if ("raw_codes" == val) {
                    /* init raw_codes mode */

                    if (!checkMode(mode, ID_remote,
                        "begin raw_codes")) break;
                    if (!rem->codes.empty()) {

                        return nullptr;
                    }
                    set_protocol(rem, RAW_CODES);
                    raw_code.code = 0;
                    raw_codes.clear();
                    raw_codes.reserve(30);
                    mode = ID_raw_codes;
                }
                else if ("remote" == val) {
                    /* create new remote */

                    if (!checkMode(mode, ID_none, "begin remote"))
                        break;
                    mode = ID_remote;
                    if (!top_rem) {
                        /* create first remote */

                        top_rem = std::make_unique<ir_remote>();
                        rem = top_rem.get();
                    }
                    else {
                        /* create new remote */

                        rem->next = std::make_unique<ir_remote>();
                        rem = rem->next.get();
                    }
                }
                else if (mode == ID_codes) {
                    auto c = defineCode(key, val, &name_code);
                    if (!c)
                        return nullptr;
                    code = *c;
                    while (!val2.empty())
                    {
                        if (val2[0] == '#') break; /* comment */
                        if (!defineNode(code, val2))
                            break;
                        val2 = strtok(buf, whitespace);
                    }
                    code->current = nullptr;
                    codes_list.push_back(std::move(*code));
                }
                else {
                    return nullptr;
                }
            }
            else if ("end" == key) {

                if ("codes" == val) {
                    /* end Codes mode */
                    if (!checkMode(mode, ID_codes, "end codes"))
                        break;
                    rem->codes = std::move(codes_list);
                    codes_list.clear();
                    mode = ID_remote;     /* switch back */

                }
                else if ("raw_codes" == val) {
                    /* end raw codes mode */


                    if (mode == ID_raw_name) {
                        raw_code.signals = signals;
                        if (raw_code.length() % 2 == 0)
                            return nullptr;
                        raw_codes.push_back(std::move(raw_code));
                        mode = ID_raw_codes;
                    }
                    if (!checkMode(mode, ID_raw_codes, "end raw_codes"))
                        break;
                    rem->codes = std::move(raw_codes);
                    raw_codes.clear();
                    mode = ID_remote;     /* switch back */
                }
                else if ("remote" == val) {
                    /* end remote mode */
                    /* print_remote(rem); */
                    if (!checkMode(mode, ID_remote, "end remote"))
                        break;
                    if (!sanityChecks(rem))
                        return nullptr;

                    /* not really necessary because we
                    clear the alloced memory */
                    rem->next = nullptr;
                    rem->last_code = nullptr;
                    mode = ID_none;     /* switch back */
                }
                else if (mode == ID_codes) {
                    auto c = defineCode(key, val, &name_code);
                    if (!c)
                        return nullptr;
                    code = *c;
                    while (!val2.empty())
                    {
                        if (val2[0] == '#') break; /* comment */
                        auto node = defineNode(code, val2);
                        if (!node)
                            break;
                        val2 = strtok(buf, whitespace);
                    }
                    code->current = nullptr;
                    codes_list.push_back(std::move(*code));
                }
                else {
                    return nullptr;
                }
            }
            else {
                switch (mode) {
                case ID_remote:
                    defineRemote(key, val, val2, rem);
                    break;
                case ID_codes: {
                    auto c = defineCode(key, val, &name_code);
                    if (!c)
                        return nullptr;
                    code = *c;
                    while (!val2.empty())
                    {
                        if (val2[0] == '#') break; /* comment */
                        auto node = defineNode(code, val2);
                        if (!node)
                            break;
                        val2 = strtok(buf, whitespace);
                    }
                    code->current = nullptr;
                    codes_list.push_back(std::move(*code));
                    break;
                }
                case ID_raw_codes:
                case ID_raw_name:
                    if ("name" == key) {
                        if (mode == ID_raw_name)
                        {
                            raw_code.signals = std::move(signals);
                            if (raw_code.length() % 2 == 0)
                                return nullptr;
                            raw_codes.push_back(std::move(raw_code));
                        }
                        raw_code.name = std::string{ begin(val), end(val) };
                        raw_code.code++;
                        signals.clear();
                        signals.reserve(50);
                        mode = ID_raw_name;
                    }
                    else {
                        if (mode == ID_raw_codes)
                        {
                            return nullptr;
                        }
                        if (!addSignal(signals, key)) break;
                        if (!addSignal(signals, val)) break;
                        if (!val2.empty()) {
                            if (!addSignal(signals, val2)) {
                                break;
                            }
                        }
                        while (true) {
                            auto val = strtok(buf, whitespace);
                            if (val.empty() || !addSignal(signals, val)) break;
                        }
                    }
                    break;
                }
            }
        }
        else if (mode == ID_raw_name) {
            if (!addSignal(signals, key)) {
                break;
            }
        }
        else {
            return nullptr;
        }

    }
    if (mode != ID_none)
    {
        switch (mode)
        {
        case ID_raw_name:
            if (raw_code.name != std::nullopt)
            {
                raw_code.name->clear();
                signals.clear();
            }
        case ID_raw_codes:
            rem->codes = std::move(raw_codes);
            break;
        case ID_codes:
            rem->codes = std::move(codes_list);
            break;
        }
        //if (!g_parse_error)
        //{
        //    g_parse_error = 1;
        //}
    }

    /* kick reverse flag */
    /* handle RC6 flag to be backwards compatible: previous RC-6
    config files did not set rc6_mask */
    rem = top_rem.get();
    while (rem != nullptr)
    {
        if ((!is_raw(rem)) && rem->flags & REVERSE)
        {
            if (has_pre(rem))
            {
                rem->pre_data = reverse(rem->pre_data,
                    rem->pre_data_bits);
            }
            if (has_post(rem))
            {
                rem->post_data = reverse(rem->post_data,
                    rem->post_data_bits);
            }
            for (auto& c : rem->codes)
            {
                c.code = reverse(c.code, rem->bits);
            }
            rem->flags = rem->flags & (~REVERSE);
            rem->flags = rem->flags | COMPAT_REVERSE;
            /* don't delete the flag because we still need
            it to remain compatible with older versions
            */
        }
        if (rem->flags & RC6 && rem->rc6_mask == 0 && rem->toggle_bit > 0)
        {
            int all_bits = bit_count(rem);

            rem->rc6_mask = ((ir_code)1) << (all_bits - rem->toggle_bit);
        }
        if (rem->toggle_bit > 0)
        {
            int all_bits = bit_count(rem);

            if (!has_toggle_bit_mask(rem))
            {
                rem->toggle_bit_mask = ((ir_code)1) << (all_bits - rem->toggle_bit);
            }
            rem->toggle_bit = 0;
        }
        if (has_toggle_bit_mask(rem))
        {
            if (!is_raw(rem) && !rem->codes.empty())
            {
                rem->toggle_bit_mask_state = (rem->codes[0].code & rem->toggle_bit_mask);
                if (rem->toggle_bit_mask_state)
                {
                    /* start with state set to 0 for backwards compatibility */
                    rem->toggle_bit_mask_state ^= rem->toggle_bit_mask;
                }
            }
        }
        if (is_serial(rem))
        {
            if (rem->baud > 0)
            {
                lirc_t base = 1000000 / rem->baud;
                if (rem->pzero == 0 && rem->szero == 0)
                {
                    rem->pzero = base;
                }
                if (rem->pone == 0 && rem->sone == 0)
                {
                    rem->sone = base;
                }
            }
            if (rem->bits_in_byte == 0)
            {
                rem->bits_in_byte = 8;
            }
        }
        if (rem->min_code_repeat > 0)
        {
            if (!has_repeat(rem) ||
                rem->min_code_repeat > rem->min_repeat)
            {
                rem->min_code_repeat = 0;
            }
        }
        rem = rem->next.get();
    }

    return sort_by_bit_count(std::move(top_rem));
}
