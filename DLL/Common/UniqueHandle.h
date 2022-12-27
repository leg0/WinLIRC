#pragma once

#include <winrt/base.h>
#include <concepts>
#include <utility>

namespace winlirc
{
    template <typename HandleType, auto Close, HandleType InvalidValue = HandleType{}>
    struct GenericTraits {
        using type = HandleType;
        static inline constexpr auto close = Close;
        static inline constexpr type invalid() noexcept { return InvalidValue; }
    };

    template <typename HandleType, auto Close, HandleType InvalidValue = HandleType{} >
    using UniqueHandle = winrt::handle_type<GenericTraits<HandleType, Close, InvalidValue>>;
}
