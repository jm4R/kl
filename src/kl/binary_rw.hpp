#pragma once

#include "kl/byte.hpp"
#include "kl/type_traits.hpp"

#include <gsl/span>
#include <cstdint>
#include <cstring>

namespace kl {

namespace detail {

template <typename T>
class cursor_base
{
    // Expect T to differ only by CV-qualifier
    static_assert(std::is_same<std::remove_cv_t<T>, byte>::value, "!!!");

public:
    explicit cursor_base(gsl::span<T> buffer) noexcept
        : buffer_(std::move(buffer))
    {
    }

    template <typename U, std::ptrdiff_t Extent>
    explicit cursor_base(gsl::span<U, Extent> buffer) noexcept
        : buffer_{reinterpret_cast<T*>(buffer.data()), buffer.size_bytes()}
    {
    }

    void skip(std::ptrdiff_t off) noexcept
    {
        // Make sure pos_ does not escape of buffer range
        if (off > 0 && off > left())
            err_ = true;
        if (off < 0 && (-off) > pos_)
            err_ = true;

        if (!err_)
            pos_ += off;
    }

    bool empty() const noexcept { return left() <= 0; }
    // Returns how many bytes are left in the internal buffer
    std::ptrdiff_t left() const noexcept
    {
        return buffer_.size_bytes() - pos();
    }
    // Returns how many bytes we've already read
    std::ptrdiff_t pos() const noexcept { return pos_; }

    bool err() const noexcept { return err_; }

    // Useful in user-provided operator>> for composite types to fail fast
    void notify_error() noexcept { err_ = true; }

protected:
    T* cursor() const noexcept { return buffer_.data() + pos_; }

protected:
    gsl::span<T> buffer_;
    std::ptrdiff_t pos_{0};
    bool err_{false};
};
} // namespace detail

class binary_reader : public detail::cursor_base<const byte>
{
public:
    using detail::cursor_base<const byte>::cursor_base;

    template <typename T>
    bool read(T& value)
    {
        if (err_)
            return false;
        (*this) >> value;
        return !err_;
    }

    template <typename T,
              typename = enable_if<std::is_default_constructible<T>>>
    T read()
    {
        T value{};
        read(value);
        return value;
    }

    template <typename T>
    bool peek(T& value) noexcept
    {
        return peek_basic(value);
    }

    template <typename T,
              typename = enable_if<std::is_default_constructible<T>>>
    T peek()
    {
        T value{};
        peek(value);
        return value;
    }

    gsl::span<const byte> view(std::size_t count, bool move_cursor = true)
    {
        if (count > static_cast<std::size_t>(left()))
            err_ = true;
        if (err_)
            return {};

        gsl::span<const byte> ret(cursor(), count);
        if (move_cursor)
            pos_ += count;

        return ret;
    }

    // Default Stream Op implementation for all trivially copyable types
    template <typename T>
    friend binary_reader& operator>>(binary_reader& r, T& value) noexcept
    {
        if (!r.err_ && r.peek_basic(value))
            r.pos_ += sizeof(value);
        else
            r.err_ = true;

        return r;
    }

    template <typename T, std::ptrdiff_t Extent>
    friend binary_reader& operator>>(binary_reader& r,
                                     gsl::span<T, Extent> span)
    {
        if (!r.err_ && r.peek_span(span))
            r.pos_ += span.size_bytes();
        else
            r.err_ = true;

        return r;
    }

private:
    template <typename T>
    bool peek_basic(T& value) noexcept
    {
        // If you get compilation error here it means your type T does not
        // provide operator>>(kl::binary_reader&, T&) function and does not
        // satisfy TriviallyCopyable concept.

        // If the objects are not TriviallyCopyable, the behavior of memcpy is
        // not specified and may be undefined.
        static_assert(std::is_trivially_copyable<T>::value,
                      "T must be a trivially copyable type");

        if (err_ || static_cast<std::size_t>(left()) < sizeof(T))
            return false;

        std::memcpy(&value, cursor(), sizeof(T));
        return true;
    }

    template <typename T, std::ptrdiff_t Extent>
    bool peek_span(gsl::span<T, Extent> span)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "T must be a trivially copyable type");

        auto repr = gsl::make_span(reinterpret_cast<byte*>(span.data()),
                                   span.size_bytes());

        if (err_ || left() < repr.size_bytes())
            return false;

        std::memcpy(repr.data(), cursor(), repr.size_bytes());
        return true;
    }
};

class binary_writer : public detail::cursor_base<byte>
{
public:
    using detail::cursor_base<byte>::cursor_base;

    // Default Stream Op implementation for all trivially copyable types
    template <typename T>
    friend binary_writer& operator<<(binary_writer& w, const T& value) noexcept
    {
        if (w.write_basic(value))
            w.pos_ += sizeof(value);
        else
            w.err_ = true;

        return w;
    }

    template <typename T, std::ptrdiff_t Extent>
    friend binary_writer& operator<<(binary_writer& w,
                                     gsl::span<const T, Extent> span)
    {
        if (!w.err_ && w.write_span(span))
            w.pos_ += span.size_bytes();
        else
            w.err_ = true;

        return w;
    }

private:
    template <typename T>
    bool write_basic(const T& value) noexcept
    {
        // If you get compilation error here it means your type T does not
        // provide operator<<(kl::binary_writer&, const T&) function and does
        // not satisfy TriviallyCopyable concept.

        // If the objects are not TriviallyCopyable, the behavior of memcpy is
        // not specified and may be undefined.
        static_assert(std::is_trivially_copyable<T>::value,
                      "T must be a trivially copyable type");

        if (err_ || static_cast<std::size_t>(left()) < sizeof(T))
            return false;

        std::memcpy(cursor(), &value, sizeof(T));
        return true;
    }

    template <typename T, std::ptrdiff_t Extent>
    bool write_span(gsl::span<const T, Extent> span)
    {
        static_assert(std::is_trivially_copyable<T>::value,
                      "T must be a trivially copyable type");

        auto repr = gsl::make_span(reinterpret_cast<const byte*>(span.data()),
                                   span.size_bytes());

        if (err_ || left() < repr.size_bytes())
            return false;

        std::memcpy(cursor(), repr.data(), repr.size_bytes());
        return true;
    }
};
} // namespace kl
