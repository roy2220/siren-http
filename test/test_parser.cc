#include <cstring>

#include <siren/stream.h>
#include <siren/test.h>

#include "parser.h"
#include "request.h"
#include "response.h"


namespace {

using namespace siren;
using namespace siren::http;


SIREN_TEST("Parse http requests/responses")
{
    Stream s;
    ParseOptions po;

    Parser p(po, &s, [f = 1] (Stream *s) mutable -> void {
        if (f == 1) {
            char m[] =
                "GET https://admin:guess@google.com:666/s?q=abc#def HTTP/1.1\r\n"
                "Host: google.com\r\n"
                "Content-Length: 6\r\n"
                "\r\n"
                "hello!"
            ;

            s->write(m, sizeof(m) - 1);
        } else if (f == 2) {
            char m[] =
                "GET / HTTP/1.0\r\n"
                "Host:test.com\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"
                "6\r\n"
                "hello!\r\n"
                "6\r\n"
                "world!\r\n"
                "0\r\n"
                "\r\n"
            ;

            s->write(m, sizeof(m) - 1);
        } else if (f == 3) {
            char m[] =
                "HTTP/1.1 200 Foo, Bar!\r\n"
                "Key:  Val ue \r\n"
                "\r\n"
            ;

            s->write(m, sizeof(m) - 1);
        } else {
            throw EndOfStream();
        }

        ++f;
    });

    Request req;
    p.getRequest(&req);
    SIREN_TEST_ASSERT(req.methodType == MethodType::Get);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getSchemeName(), "https") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getUserInfo(), "admin:guess") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getHostName(), "google.com") == 0);
    SIREN_TEST_ASSERT(req.uri.portNumber == 666);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getPathName(), "/s") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getQueryString(), "q=abc") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getFragmentID(), "def") == 0);
    SIREN_TEST_ASSERT(req.majorVersionNumber == 1);
    SIREN_TEST_ASSERT(req.minorVersionNumber == 1);
    SIREN_TEST_ASSERT(!p.bodyIsChunked());

    {
        std::size_t sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 6);
        char *pl = p.peekPayloadData(sz);
        SIREN_TEST_ASSERT(std::memcmp(pl, "hello!", sz) == 0);
        sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 6);
        p.discardPayloadData(sz);
        sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 0);
    }

    req.header.traverse([] (std::size_t, const char *fn, const char *fv) -> void {
        SIREN_TEST_ASSERT(std::strcmp(fn, "Host") == 0);
        SIREN_TEST_ASSERT(std::strcmp(fv, "google.com") == 0);
    });

    req.uri.reset();
    req.header.reset();
    p.getRequest(&req);
    SIREN_TEST_ASSERT(req.methodType == MethodType::Get);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getSchemeName(), "") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getUserInfo(), "") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getHostName(), "") == 0);
    SIREN_TEST_ASSERT(req.uri.portNumber < 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getPathName(), "/") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getQueryString(), "") == 0);
    SIREN_TEST_ASSERT(std::strcmp(req.uri.getFragmentID(), "") == 0);
    SIREN_TEST_ASSERT(req.majorVersionNumber == 1);
    SIREN_TEST_ASSERT(req.minorVersionNumber == 0);
    SIREN_TEST_ASSERT(p.bodyIsChunked());

    {
        std::size_t sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 6);
        char *pl = p.peekPayloadData(sz);
        SIREN_TEST_ASSERT(std::memcmp(pl, "hello!", sz) == 0);
        sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 6);
        p.discardPayloadData(sz);
        sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 6);
        pl = p.peekPayloadData(sz);
        SIREN_TEST_ASSERT(std::memcmp(pl, "world!", sz) == 0);
        p.discardPayloadData(sz);
        sz = p.getRemainingBodyOrChunkSize();
        SIREN_TEST_ASSERT(sz == 0);
        SIREN_TEST_ASSERT(p.bodyIsChunked());
        p.discardPayloadData(0);
        SIREN_TEST_ASSERT(!p.bodyIsChunked());
    }

    req.header.traverse([] (std::size_t, const char *fn, const char *fv) -> void {
        SIREN_TEST_ASSERT(std::strcmp(fn, "Host") == 0);
        SIREN_TEST_ASSERT(std::strcmp(fv, "test.com") == 0);
    });

    Response rsp;
    p.getResponse(&rsp);
    SIREN_TEST_ASSERT(rsp.majorVersionNumber == 1);
    SIREN_TEST_ASSERT(rsp.minorVersionNumber == 1);
    SIREN_TEST_ASSERT(rsp.statusCode == StatusCode::OK);
    SIREN_TEST_ASSERT(rsp.reasonPhrase == "Foo, Bar!");
    SIREN_TEST_ASSERT(p.getRemainingBodyOrChunkSize() == 0);

    rsp.header.traverse([] (std::size_t, const char *fn, const char *fv) -> void {
        SIREN_TEST_ASSERT(std::strcmp(fn, "Key") == 0);
        SIREN_TEST_ASSERT(std::strcmp(fv, "Val ue") == 0);
    });

    {
        bool t = false;

        try {
            p.getResponse(&rsp);
        } catch (const EndOfStream &) {
            t = true;
        }

        SIREN_TEST_ASSERT(t);
    }
}

}
