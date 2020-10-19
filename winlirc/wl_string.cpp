#include "wl_string.h"

winlirc::istring_view winlirc::strtok(winlirc::istring_view& s, char const* separators)
{
    auto pos = s.find_first_not_of(separators);
    if (pos == istring_view::npos)
        return s = {};
    else if (pos > 0)
        s = s.substr(pos);

    pos = s.find_first_of(separators);
    if (pos == istring_view::npos)
        return std::exchange(s, istring_view{});

    auto res = s.substr(0, pos);
    s.remove_prefix(pos);
    pos = s.find_first_not_of(separators);
    if (pos == istring_view::npos)
        s = {};
    else
        s.remove_prefix(pos);
    return res;
}
