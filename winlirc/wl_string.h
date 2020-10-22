#pragma once

#include <string>
#include <string_view>

namespace winlirc
{

struct ichar_traits : std::char_traits<char>
{
    static constexpr char toupper(char c) noexcept
    {
        constexpr auto diff = 'a' - 'A';
        if ('a' <= c && c <= 'z')
            return c - diff;
        return c;
    }

    static constexpr bool eq(char a, char b) noexcept
    {
        return a == b || toupper(a) == toupper(b);
    }

    static constexpr bool lt(char a, char b) noexcept
    {
        return toupper(a) < toupper(b);
    }
};

using istring_view = std::basic_string_view<char, ichar_traits>;
using istring      = std::basic_string<char, ichar_traits>;

istring_view strtok(istring_view& s, char const* separators);

template <typename C, typename CT>
std::basic_string_view<C, CT> rtrim(std::basic_string_view<C, CT> s, char const* spaces)
{
    using sv = std::basic_string_view<C, CT>;
    auto const pos = s.find_last_not_of(spaces);
    if (pos == sv::npos)
        // everything is in s is a space.
        return {};
    else
        return s.substr(0, pos+1);
}

bool operator==(istring_view const&, nullptr_t) = delete;
bool operator==(nullptr_t, istring_view const&) = delete;
bool operator!=(istring_view const&, nullptr_t) = delete;
bool operator!=(nullptr_t, istring_view const&) = delete;
} // namespace winlirc

char* strtok(nullptr_t, char const*) = delete;
