#pragma once

namespace winlirc
{
    namespace impl
    {
        struct S { int i; };
    }

    /// SafeBool is is safe because unlike built-in bool it does not
    /// implicitly convert to integer, and therefore is safer to use
    /// as return type for type-conversion operator than bool is.
    typedef int impl::S::* SafeBool;

    inline SafeBool safeTrue()  { return &impl::S::i; }
    inline SafeBool safeFalse() { return nullptr; }
}
