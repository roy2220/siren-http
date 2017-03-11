#pragma once


#include <cstddef>
#include <functional>


namespace siren {

class Stream;


namespace http {

class InputStream final
{
public:
    inline InputStream(InputStream &&) noexcept;
    inline InputStream &operator=(InputStream &&) noexcept;

    template <class T, class = std::enable_if_t<!std::is_same<T, nullptr_t>::value, void>>
    inline explicit InputStream(Stream *, T &&);

    inline bool isValid() const noexcept;
    inline char *peekChars(std::size_t);
    inline void discardChars(std::size_t) noexcept;

private:
    Stream *base_;
    std::function<void (Stream *, std::size_t)> writer_;

    inline void initialize(Stream *) noexcept;
    inline void move(InputStream *) noexcept;
};

} // namespace http

} // namespace siren


/*
 * #include "input_stream-inl.h"
 */


#include <utility>

#include <siren/assert.h>
#include <siren/stream.h>


namespace siren {

namespace http {

template <class T, class>
InputStream::InputStream(Stream *base, T &&writer)
  : writer_(std::forward<T>(writer))
{
    SIREN_ASSERT(base != nullptr);
    initialize(base);
}


InputStream::InputStream(InputStream &&other) noexcept
  : writer_(std::move(other.writer_))
{
    other.move(this);
}


InputStream &
InputStream::operator=(InputStream &&other) noexcept
{
    if (&other != this) {
        writer_ = std::move(other.writer_);
        other.move(this);
    }

    return *this;
}


void
InputStream::initialize(Stream *base) noexcept
{
    base_ = base;
}


void
InputStream::move(InputStream *other) noexcept
{
    other->base_ = base_;
    base_ = nullptr;
}


bool
InputStream::isValid() const noexcept
{
    return base_ != nullptr && writer_ != nullptr;
}


char *
InputStream::peekChars(std::size_t numberOfChars)
{
    SIREN_ASSERT(isValid());
    std::size_t charCount = base_->getDataSize();

    if (charCount < numberOfChars) {
        writer_(base_, numberOfChars - charCount);
        charCount = base_->getDataSize();

        if (charCount < numberOfChars) {
            throw EndOfStream();
        }
    }

    auto chars = static_cast<char *>(base_->getData());
    return chars;
}


void
InputStream::discardChars(std::size_t numberOfChars) noexcept
{
    SIREN_ASSERT(isValid());
    base_->discardData(numberOfChars);
}

} // namespace http

} // namespace siren