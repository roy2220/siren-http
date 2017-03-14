#include "http/connection.h"

#include <functional>
#include <utility>


namespace siren {

namespace http {

Connection::Connection(const ConnectionOptions &options, TCPSocket &&tcpSocket)
  : options_(options),
    tcpSocket_(std::move(tcpSocket)),
    parser_(options, &streams_[0], std::bind(&Connection::writeStream, this
                                             , std::placeholders::_1)),
    dumper_(&streams_[1], std::bind(&Connection::readStream, this, std::placeholders::_1))
{
}


void
Connection::readStream(Stream *stream)
{
    tcpSocket_.write(stream);
}


void
Connection::writeStream(Stream *stream)
{
    stream->reserveBuffer(options_.minReadBufferSize);

    if (tcpSocket_.read(stream) == 0) {
        throw EndOfStream();
    }
}

} // namespace http

} // namespace siren
