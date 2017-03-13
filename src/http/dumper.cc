#include "http/dumper.h"

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <limits>

#include <siren/utility.h>

#include "http/request.h"
#include "http/response.h"


namespace siren {

namespace http {

Dumper::Dumper(Dumper &&other) noexcept
  : outputStream_(std::move(other.outputStream_))
{
    other.move(this);
}


Dumper &
Dumper::operator=(Dumper &&other) noexcept
{
    if (&other != this) {
        outputStream_ = std::move(other.outputStream_);
        other.move(this);
    }

    return *this;
}


void
Dumper::initialize() noexcept
{
    bodyIsChunked_ = false;
    remainingBodySize_ = 0;
}


void
Dumper::move(Dumper *other) noexcept
{
    other->bodyIsChunked_ = bodyIsChunked_;

    if (!bodyIsChunked_) {
        other->remainingBodySize_ = remainingBodySize_;
    }
}


void
Dumper::putRequest(const Request &request, bool bodyIsChunked, std::size_t bodySize)
{
    SIREN_ASSERT(isValid());
    SIREN_ASSERT(!bodyIsChunked_ && remainingBodySize_ == 0);
    dumpRequestStartLine(request);
    dumpHeader(request.header, bodyIsChunked, bodySize);
    bodyIsChunked_ = bodyIsChunked;

    if (!bodyIsChunked) {
        remainingBodySize_ = bodySize;
    }
}


void
Dumper::putResponse(const Response &response, bool bodyIsChunked, std::size_t bodySize)
{
    SIREN_ASSERT(isValid());
    SIREN_ASSERT(!bodyIsChunked_ && remainingBodySize_ == 0);
    dumpResponseStartLine(response);
    dumpHeader(response.header, bodyIsChunked, bodySize);
    bodyIsChunked_ = bodyIsChunked;

    if (!bodyIsChunked) {
        remainingBodySize_ = bodySize;
    }
}


char *
Dumper::reservePayloadBuffer(std::size_t payloadBufferSize)
{
    SIREN_ASSERT(isValid());

    if (bodyIsChunked_) {
        constexpr unsigned int k = (std::numeric_limits<std::size_t>::digits + 3) / 4;

        return outputStream_.reserveBuffer(k + SIREN_STRLEN("\r\n") + payloadBufferSize
                                           + SIREN_STRLEN("\r\n")) + k + SIREN_STRLEN("\r\n");
    } else {
        SIREN_ASSERT(payloadBufferSize <= remainingBodySize_);
        return outputStream_.reserveBuffer(payloadBufferSize);
    }
}


void
Dumper::flushPayloadBuffer(std::size_t payloadBufferSize)
{
    SIREN_ASSERT(isValid());

    if (bodyIsChunked_) {
        constexpr unsigned int k = (std::numeric_limits<std::size_t>::digits + 3) / 4;

        char *s1 = outputStream_.reserveBuffer(k + SIREN_STRLEN("\r\n") + payloadBufferSize
                                               + SIREN_STRLEN("\r\n"));
        char *s2 = s1;
        s2 += std::sprintf(s2, "%0*zX", k, payloadBufferSize);
        *s2++ = '\r';
        *s2++ = '\n';
        s2 += payloadBufferSize;
        *s2++ = '\r';
        *s2++ = '\n';
        outputStream_.flushBuffer(s2 - s1);

        if (payloadBufferSize == 0) {
            bodyIsChunked_ = false;
            remainingBodySize_ = 0;
        }
    } else {
        SIREN_ASSERT(payloadBufferSize <= remainingBodySize_);
        outputStream_.flushBuffer(payloadBufferSize);
        remainingBodySize_ -= payloadBufferSize;
    }
}


void
Dumper::dumpRequestStartLine(const Request &request)
{
    const char *methodName = GetMethodName(request.methodType);
    const char *schemeName = request.uri.getSchemeName();
    const char *userInfo = request.uri.getUserInfo();
    const char *hostName = request.uri.getHostName();
    const char *pathName = request.uri.getPathName();
    const char *queryString = request.uri.getQueryString();
    const char *fragmentID = request.uri.getFragmentID();

    char *s1 = outputStream_.reserveBuffer(
        std::strlen(methodName) +
        SIREN_STRLEN(" ") +
        std::strlen(schemeName) +
        SIREN_STRLEN("://") +
        std::strlen(userInfo) +
        SIREN_STRLEN("@") +
        std::strlen(hostName) +
        SIREN_STRLEN(":") +
        std::numeric_limits<std::uint16_t>::digits10 +
        std::strlen(pathName) +
        SIREN_STRLEN("?") +
        std::strlen(queryString) +
        SIREN_STRLEN("#") +
        std::strlen(fragmentID) +
        SIREN_STRLEN(" ") +
        SIREN_STRLEN("HTTP/") +
        std::numeric_limits<unsigned short>::digits10 +
        SIREN_STRLEN(".") +
        std::numeric_limits<unsigned short>::digits10 +
        SIREN_STRLEN("\r\n")
    );

    char *s2 = s1;
    s2 += std::sprintf(s2, "%s ", methodName);

    if (*pathName == '*') {
        *s2++ = '*';
    } else {
        if (*schemeName != '\0') {
            s2 += std::sprintf(s2, "%s://", schemeName);

            if (*userInfo != '\0') {
                s2 += std::sprintf(s2, "%s@", userInfo);
            }

            s2 += std::sprintf(s2, "%s", hostName);

            if (request.uri.portNumber >= 0) {
                s2 += std::sprintf(s2, ":%" PRIu16, request.uri.portNumber);
            }
        }

        s2 += std::sprintf(s2, "%s", pathName);

        if (*queryString != '\0') {
            s2 += std::sprintf(s2, "?%s", queryString);
        }

        if (*fragmentID != '\0') {
            s2 += std::sprintf(s2, "#%s", fragmentID);
        }
    }

    s2 += std::sprintf(s2, " HTTP/%hu.%hu", request.majorVersionNumber, request.minorVersionNumber);
    *s2++ = '\r';
    *s2++ = '\n';
    outputStream_.flushBuffer(s2 - s1);
}


void
Dumper::dumpResponseStartLine(const Response &response)
{
    char *s1 = outputStream_.reserveBuffer(
        SIREN_STRLEN("HTTP/") +
        std::numeric_limits<unsigned short>::digits10 +
        SIREN_STRLEN(".") +
        std::numeric_limits<unsigned short>::digits10 +
        SIREN_STRLEN(" ") +
        std::numeric_limits<int>::digits10 +
        SIREN_STRLEN(" ") +
        response.reasonPhrase.size() +
        SIREN_STRLEN("\r\n")
    );

    char *s2 = s1;
    s2 += std::sprintf(s2, "HTTP/%hu.%hu %d %s", response.majorVersionNumber
                       , response.minorVersionNumber, static_cast<int>(response.statusCode)
                       , response.reasonPhrase.c_str());
    *s2++ = '\r';
    *s2++ = '\n';
    outputStream_.flushBuffer(s2 - s1);
}


void
Dumper::dumpHeader(const Header &header, bool bodyIsChunked, std::size_t bodySize)
{
    std::size_t n = 0;

    if (bodyIsChunked) {
        char *s1 = outputStream_.reserveBuffer(
            n +
            SIREN_STRLEN("Transfer-Encoding: chunked\r\n")
        ) + n;

        char *s2 = s1;
        s2 += std::sprintf(s2, "Transfer-Encoding: chunked");
        *s2++ = '\r';
        *s2++ = '\n';
        n += s2 - s1;
    } else {
        constexpr unsigned int k = (std::numeric_limits<std::size_t>::digits + 2) / 3;

        char *s1 = outputStream_.reserveBuffer(
            n +
            SIREN_STRLEN("Content-Length: ") +
            k +
            SIREN_STRLEN("\r\n")
        ) + n;

        char *s2 = s1;
        s2 += std::sprintf(s2, "Content-Length: %zo", bodySize);
        *s2++ = '\r';
        *s2++ = '\n';
        n += s2 - s1;
    }

    header.traverse([&] (std::size_t, const char *headerFieldName , const char *headerFieldValue)
                    -> bool {
        char *s1 = outputStream_.reserveBuffer(
            n +
            std::strlen(headerFieldName) +
            SIREN_STRLEN(": ") +
            std::strlen(headerFieldValue) +
            SIREN_STRLEN("\r\n")
        ) + n;

        char *s2 = s1;
        s2 += std::sprintf(s2, "%s: %s", headerFieldName, headerFieldValue);
        *s2++ = '\r';
        *s2++ = '\n';
        n += s2 - s1;
        return true;
    });

    {
        char *s1 = outputStream_.reserveBuffer(
            n +
            SIREN_STRLEN("\r\n")
        ) + n;

        char *s2 = s1;
        *s2++ = '\r';
        *s2++ = '\n';
        n += s2 - s1;
    }

    outputStream_.flushBuffer(n);
}

} // namespace http

} // namespace siren
