#pragma once


#include <cstddef>

#include <siren/stream.h>
#include <siren/tcp_socket.h>

#include "http/dumper.h"
#include "http/parser.h"


namespace siren {

namespace http {

class PayloadReader;
class PayloadWriter;
struct Request;
struct Response;


namespace detail {

struct ConnectionOptions
{
    std::size_t minReadBufferSize = 4096;
};

} // namespace detail


struct ConnectionOptions
  : detail::ConnectionOptions,
    ParseOptions
{
};


class Connection final
{
public:
    inline bool isValid() const noexcept;
    inline PayloadReader parseRequest(Request *);
    inline PayloadReader parseResponse(Response *);
    inline PayloadWriter dumpRequest(const Request &);
    inline PayloadWriter dumpRequest(const Request &, std::size_t);
    inline PayloadWriter dumpResponse(const Response &);
    inline PayloadWriter dumpResponse(const Response &, std::size_t);

    explicit Connection(const ConnectionOptions &, TCPSocket &&);

private:
    typedef detail::ConnectionOptions Options;

    Options options_;
    TCPSocket tcpSocket_;
    Stream streams_[2];
    Parser parser_;
    Dumper dumper_;

    void readStream(Stream *);
    void writeStream(Stream *);
};


class PayloadReader final
{
public:
    inline PayloadReader(PayloadReader &&) noexcept;
    inline PayloadReader &operator=(PayloadReader &&) noexcept;

    inline bool isValid() const noexcept;
    inline bool bodyIsChunked() const noexcept;
    inline std::size_t getRemainingBodyOrChunkSize() const noexcept;
    inline char *peekData(std::size_t *);
    inline void discardData(std::size_t);

protected:
    inline explicit PayloadReader(Parser *) noexcept;

private:
    Parser *parser_;

    inline void initialize(Parser *) noexcept;
    inline void move(PayloadReader *) noexcept;

    friend Connection;
};


class PayloadWriter final
{
public:
    inline PayloadWriter(PayloadWriter &&) noexcept;
    inline PayloadWriter &operator=(PayloadWriter &&) noexcept;

    inline bool isValid() const noexcept;
    inline bool bodyIsChunked() const noexcept;
    inline std::size_t getRemainingBodySize() const noexcept;
    inline char *reserveBuffer(std::size_t);
    inline void flushBuffer(std::size_t);

protected:
    inline explicit PayloadWriter(Dumper *) noexcept;

private:
    Dumper *dumper_;

    inline void initialize(Dumper *) noexcept;
    inline void move(PayloadWriter *) noexcept;

    friend Connection;
};

} // namespace http

} // namespace siren


/*
 * #include "http/connection-inl.h"
 */


#include <siren/assert.h>


namespace siren {

namespace http {

bool
Connection::isValid() const noexcept
{
    return tcpSocket_.isValid() && parser_.isValid() && dumper_.isValid();
}


PayloadReader
Connection::parseRequest(Request *request)
{
    SIREN_ASSERT(isValid());
    parser_.getRequest(request);
    return PayloadReader(&parser_);
}


PayloadReader
Connection::parseResponse(Response *response)
{
    SIREN_ASSERT(isValid());
    parser_.getResponse(response);
    return PayloadReader(&parser_);
}


PayloadWriter
Connection::dumpRequest(const Request &request)
{
    SIREN_ASSERT(isValid());
    dumper_.putRequest(request);
    return PayloadWriter(&dumper_);
}


PayloadWriter
Connection::dumpRequest(const Request &request, std::size_t bodySize)
{
    SIREN_ASSERT(isValid());
    dumper_.putRequest(request, bodySize);
    return PayloadWriter(&dumper_);
}


PayloadWriter
Connection::dumpResponse(const Response &response)
{
    SIREN_ASSERT(isValid());
    dumper_.putResponse(response);
    return PayloadWriter(&dumper_);
}


PayloadWriter
Connection::dumpResponse(const Response &response, std::size_t bodySize)
{
    SIREN_ASSERT(isValid());
    dumper_.putResponse(response, bodySize);
    return PayloadWriter(&dumper_);
}


PayloadReader::PayloadReader(Parser *parser) noexcept
{
    initialize(parser);
}


PayloadReader::PayloadReader(PayloadReader &&other) noexcept
{
    other.move(this);
}


PayloadReader &
PayloadReader::operator=(PayloadReader &&other) noexcept
{
    if (&other != this) {
        other.move(this);
    }

    return *this;
}


void
PayloadReader::initialize(Parser *parser) noexcept
{
    parser_ = parser;
}


void
PayloadReader::move(PayloadReader *other) noexcept
{
    other->parser_ = parser_;
    parser_ = nullptr;
}


bool
PayloadReader::isValid() const noexcept
{
    return parser_ != nullptr;
}


bool
PayloadReader::bodyIsChunked() const noexcept
{
    SIREN_ASSERT(isValid());
    return parser_->bodyIsChunked();
}


std::size_t
PayloadReader::getRemainingBodyOrChunkSize() const noexcept
{
    SIREN_ASSERT(isValid());
    return parser_->getRemainingBodyOrChunkSize();
}


char *
PayloadReader::peekData(std::size_t *dataSize)
{
    SIREN_ASSERT(isValid());
    return parser_->peekPayloadData(dataSize);
}


void
PayloadReader::discardData(std::size_t dataSize)
{
    SIREN_ASSERT(isValid());
    return parser_->discardPayloadData(dataSize);
}


PayloadWriter::PayloadWriter(Dumper *dumper) noexcept
{
    initialize(dumper);
}


PayloadWriter::PayloadWriter(PayloadWriter &&other) noexcept
{
    other.move(this);
}


PayloadWriter &
PayloadWriter::operator=(PayloadWriter &&other) noexcept
{
    if (&other != this) {
        other.move(this);
    }

    return *this;
}


void
PayloadWriter::initialize(Dumper *dumper) noexcept
{
    dumper_ = dumper;
}


void
PayloadWriter::move(PayloadWriter *other) noexcept
{
    other->dumper_ = dumper_;
    dumper_ = nullptr;
}


bool
PayloadWriter::isValid() const noexcept
{
    return dumper_ != nullptr;
}


bool
PayloadWriter::bodyIsChunked() const noexcept
{
    SIREN_ASSERT(isValid());
    return dumper_->bodyIsChunked();
}


std::size_t
PayloadWriter::getRemainingBodySize() const noexcept
{
    SIREN_ASSERT(isValid());
    return dumper_->getRemainingBodySize();
}


char *
PayloadWriter::reserveBuffer(std::size_t bufferSize)
{
    SIREN_ASSERT(isValid());
    return dumper_->reservePayloadBuffer(bufferSize);
}


void
PayloadWriter::flushBuffer(std::size_t bufferSize)
{
    SIREN_ASSERT(isValid());
    return dumper_->flushPayloadBuffer(bufferSize);
}

} // namespace http

} // namespace siren
