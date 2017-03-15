#pragma once


#include <cstddef>
#include <functional>


namespace siren {

class Stream;


namespace http {

class OutputStream final
{
public:
    inline OutputStream(OutputStream &&) noexcept;
    inline OutputStream &operator=(OutputStream &&) noexcept;

    template <class T, class = std::enable_if_t<!std::is_same<T, nullptr_t>::value>>
    inline explicit OutputStream(Stream *, T &&);

    inline bool isValid() const noexcept;
    inline char *reserveBuffer(std::size_t);
    inline void flushBuffer(std::size_t);

private:
    Stream *base_;
    std::function<void (Stream *)> reader_;

    inline void initialize(Stream *) noexcept;
    inline void move(OutputStream *) noexcept;
};

} // namespace http

} // namespace siren


/*
 * #include "output_stream-inl.h"
 */


#include <utility>

#include <siren/assert.h>
#include <siren/stream.h>


namespace siren {

namespace http {

template <class T, class>
OutputStream::OutputStream(Stream *base, T &&writer)
  : reader_(std::forward<T>(writer))
{
    SIREN_ASSERT(base != nullptr);
    initialize(base);
}


OutputStream::OutputStream(OutputStream &&other) noexcept
  : reader_(std::move(other.reader_))
{
    other.move(this);
}


OutputStream &
OutputStream::operator=(OutputStream &&other) noexcept
{
    if (&other != this) {
        reader_ = std::move(other.reader_);
        other.move(this);
    }

    return *this;
}


void
OutputStream::initialize(Stream *base) noexcept
{
    base_ = base;
}


void
OutputStream::move(OutputStream *other) noexcept
{
    other->base_ = base_;
    base_ = nullptr;
}


bool
OutputStream::isValid() const noexcept
{
    return base_ != nullptr && reader_ != nullptr;
}


char *
OutputStream::reserveBuffer(std::size_t bufferSize)
{
    SIREN_ASSERT(isValid());
    base_->reserveBuffer(bufferSize);
    return static_cast<char *>(base_->getBuffer());
}


void
OutputStream::flushBuffer(std::size_t bufferSize)
{
    SIREN_ASSERT(isValid());
    base_->commitBuffer(bufferSize);

    while (base_->getDataSize() >= 1) {
        reader_(base_);
    }
}

} // namespace http

} // namespace siren
