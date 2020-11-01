#pragma once

#include <concepts>
#include <utility>

namespace winlirc
{
    template <typename T>
    concept HandleTraits =
        requires(T x, T::HandleType h)
        {
            T::invalidValue();
            T::close(h);
        };

    /// UniqueHandle is wrapper around a handle value and manages the
    /// ownership of this handle. A handle could be file handle, window
    /// handle, event, ...
    ///
    /// @param HandleTraits - contains info about the handle being wrapped
    ///   HandleType - type of handle value
    ///   HandleType invalidValue() - function that returns the invalid
    ///    value for the handle
    ///   unspecified_type close(HandleType) - function to close a valid handle
    ///   
    template <HandleTraits T>
    class UniqueHandle
    {
    protected:
        using Traits = T;
        using HandleType = typename Traits::HandleType;

    public:
        UniqueHandle(UniqueHandle const&) = delete;
        void operator=(UniqueHandle const&) = delete;

        /// Construct UniqueHandle, assume ownerhip of \a h.
        explicit UniqueHandle(HandleType h = Traits::invalidValue()) noexcept
            : handle_(h)
        { }

        ~UniqueHandle() noexcept
        {
            reset();
        }

        UniqueHandle(UniqueHandle&& other) noexcept
            : handle_(other.release())
        { }

        UniqueHandle& operator=(UniqueHandle&& rhs) noexcept
        {
            reset(rhs.release());
            return *this;
        }

        HandleType get() const noexcept { return handle_; }

        explicit operator bool() const noexcept
        {
            return handle_ != Traits::invalidValue();
        }

        /// Give up ownership of the handle.
        /// @post get() == nullptr
        HandleType release() noexcept
        {
            return std::exchange(handle_, Traits::invalidValue());
        }

        /// Give up ownership of the current handle, assume ownership of replacement.
        /// @post get() == replacement
        void reset(HandleType replacement = Traits::invalidValue()) noexcept
        {
            if (handle_ != replacement)
            {
                if (handle_ != Traits::invalidValue())
                    Traits::close(handle_);
                handle_ = replacement;
            }
        }

    private:

        HandleType handle_;
    };
}
