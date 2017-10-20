#pragma once

namespace winlirc
{
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
    template <typename HandleTraits>
    class UniqueHandle
    {
    protected:
        typedef HandleTraits Traits;
        typedef typename Traits::HandleType HandleType;

    public:
        UniqueHandle(UniqueHandle const&) = delete;
        void operator=(UniqueHandle const&) = delete;

        /// Construct UniqueHandle, assume ownerhip of \a h.
        explicit UniqueHandle(HandleType h = Traits::invalidValue())
            : handle_(h)
        { }

        ~UniqueHandle()
        {
            reset();
        }

        UniqueHandle(UniqueHandle&& other)
            : handle_(other.release())
        { }

        UniqueHandle& operator=(UniqueHandle&& rhs)
        {
            reset(rhs.release());
            return *this;
        }

        HandleType get() const { return handle_; }

        explicit operator bool() const
        {
            return handle_ != Traits::invalidValue();
        }

        /// Give up ownership of the handle.
        /// @post get() == nullptr
        HandleType release()
        {
            HandleType const res = handle_;
            handle_ = Traits::invalidValue();
            return res;
        }

        /// Give up ownership of the current handle, assume ownership of replacement.
        /// @post get() == replacement
        void reset(HandleType replacement = Traits::invalidValue())
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
