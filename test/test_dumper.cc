#include <cstring>
#include <tuple>

#include <siren/stream.h>
#include <siren/test.h>

#include "dumper.h"
#include "request.h"
#include "response.h"


namespace {

using namespace siren;
using namespace siren::http;


SIREN_TEST("Dumper http requests/responses")
{
    Stream s;

    Dumper d(&s, [f = 1] (Stream *s) mutable -> void {
        if (f <= 3) {
            if (f == 1) {
                char m[] = "GET https://admin:guess@google.com:666/s?q=abc#def HTTP/1.1\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else if (f == 2) {
                char m[] =
                    "Content-Length: 6\r\n"
                    "Host: google.com\r\n"
                    "\r\n"
                ;

                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else {
                char m[] = "hello!";
                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                s->discardData(s->getDataSize());
            }
        } else if (f <= 8) {
            if (f == 4) {
                char m[] = "GET / HTTP/1.0\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else if (f == 5) {
                char m[] =
                    "Transfer-Encoding: chunked\r\n"
                    "Host: test.com\r\n"
                    "\r\n"
                ;

                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else if (f == 6) {
                char m[] = "6\r\nhello!\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() >= sizeof(m) - 1);
                auto p = static_cast<char *>(s->getData()) + s->getDataSize() - (sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(p, m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else if (f == 7) {
                char m[] = "6\r\nworld!\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() >= sizeof(m) - 1);
                auto p = static_cast<char *>(s->getData()) + s->getDataSize() - (sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(p, m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else {
                char m[] = "0\r\n\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() >= sizeof(m) - 1);
                auto p = static_cast<char *>(s->getData()) + s->getDataSize() - (sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(p, m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            }
        } else if (f <= 10) {
            if (f == 9) {
                char m[] = "HTTP/1.1 200 Foo, Bar!\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else if (f == 10) {
                char m[] = "Key: Val ue\r\n\r\n";
                SIREN_TEST_ASSERT(s->getDataSize() == sizeof(m) - 1);
                SIREN_TEST_ASSERT(std::memcmp(s->getData(), m, sizeof(m) - 1) == 0);
                s->discardData(s->getDataSize());
            } else {
                SIREN_TEST_ASSERT(false);
            }
        }
        ++f;
    });

    Request req;
    req.methodType = MethodType::Get;
    req.uri.setSchemeName("https");
    req.uri.setUserInfo("admin:guess");
    req.uri.setHostName("google.com");
    req.uri.portNumber = 666;
    req.uri.setPathName("/s");
    req.uri.setQueryString("q=abc");
    req.uri.setFragmentID("def");
    req.majorVersionNumber = 1;
    req.minorVersionNumber = 1;
    req.header.addField(std::make_tuple("Host"), std::make_tuple("google.com"));
    d.putRequest(req, 6);

    {
        std::size_t sz = d.getRemainingBodySize();
        SIREN_TEST_ASSERT(sz == 6);
        SIREN_TEST_ASSERT(!d.bodyIsChunked());
        char *pl = d.reservePayloadBuffer(sz);
        std::memcpy(pl, "hello!", sz);
        d.flushPayloadBuffer(sz);
    }

    req.uri.reset();
    req.header.reset();
    req.methodType = MethodType::Get;
    req.uri.setPathName("/");
    req.majorVersionNumber = 1;
    req.minorVersionNumber = 0;
    req.header.addField(std::make_tuple("Host"), std::make_tuple("test.com"));
    d.putRequest(req);

    {
        SIREN_TEST_ASSERT(d.bodyIsChunked());
        char *pl = d.reservePayloadBuffer(6);
        std::memcpy(pl, "hello!", 6);
        d.flushPayloadBuffer(6);
        pl = d.reservePayloadBuffer(6);
        std::memcpy(pl, "world!", 6);
        d.flushPayloadBuffer(6);
        SIREN_TEST_ASSERT(d.bodyIsChunked());
        d.flushPayloadBuffer(0);
        SIREN_TEST_ASSERT(!d.bodyIsChunked());
    }

    Response rsp;
    rsp.majorVersionNumber = 1;
    rsp.minorVersionNumber = 1;
    rsp.statusCode = StatusCode::OK;
    rsp.reasonPhrase = "Foo, Bar!";
    rsp.header.addField(std::make_tuple("Key"), std::make_tuple("Val ue"));
    d.putResponse(rsp, 0);
}

}

