#pragma once


#include "output_stream.h"


namespace siren {

namespace http {

class Header;
struct Request;
struct Response;


class Dumper final
{
public:
    inline bool isValid() const noexcept;
    inline bool bodyIsChunked() const noexcept;
    inline std::size_t getRemainingBodySize() const noexcept;

    template <class ...T>
    inline explicit Dumper(T &&...);

    Dumper(Dumper &&) noexcept;
    Dumper &operator=(Dumper &&) noexcept;

    void putRequest(const Request &);
    void putRequest(const Request &, std::size_t);
    void putResponse(const Response &);
    void putResponse(const Response &, std::size_t);
    char *reservePayloadBuffer(std::size_t);
    void flushPayloadBuffer(std::size_t);

private:
    OutputStream outputStream_;
    bool bodyIsChunked_;
    std::size_t remainingBodySize_;

    void initialize() noexcept;
    void move(Dumper *) noexcept;
    void dumpRequestStartLine(const Request &);
    void dumpResponseStartLine(const Response &);
    void dumpHeader(const Header &, bool, std::size_t);
};

} // namespace http

} // namespace siren


/*
 * #include "dumper-inl.h"
 */


#include <utility>

#include <siren/assert.h>


namespace siren {

namespace http {

template <class ...T>
Dumper::Dumper(T &&...outputStream)
  : outputStream_(std::forward<T>(outputStream)...)
{
    initialize();
}


bool
Dumper::isValid() const noexcept
{
    return outputStream_.isValid();
}


bool
Dumper::bodyIsChunked() const noexcept
{
    SIREN_ASSERT(isValid());
    return bodyIsChunked_;
}


std::size_t
Dumper::getRemainingBodySize() const noexcept
{
    SIREN_ASSERT(isValid());
    SIREN_ASSERT(!bodyIsChunked_);
    return remainingBodySize_;
}

} // namespace http

} // namespace siren
