#pragma once


#include <cstddef>
#include <exception>
#include <tuple>

#include "http/input_stream.h"


namespace siren {

namespace http {

class Header;
class URI;
enum class MethodType;
struct Request;


struct ParserOptions
{
    std::size_t maxStartLineSize = 4 * 1024;
    std::size_t maxHeaderSize = 16 * 1024;
    std::size_t maxBodySize = 64 * 1024;
};


class Parser final
{
public:
    typedef ParserOptions Options;

    inline bool isValid() const noexcept;

    template <class ...T>
    inline explicit Parser(const Options &, T &&...);

    template <class T>
    inline void getContent(std::size_t, T &&);

    Parser(Parser &&) noexcept;
    Parser &operator=(Parser &&) noexcept;

    void getRequest(Request *);

private:
    ParserOptions options_;
    std::size_t maxChunkSize_;
    InputStream inputStream_;
    bool bodyIsChunked_;

    union {
        std::size_t bodySize_;
        std::size_t chunkSize_;
        std::size_t bodyOrChunkSize_;
    };

    static MethodType ParseMethod(const char *);
    static void ParseURI(const char *, URI *);
    static std::tuple<unsigned short, unsigned short> ParseVersion(const char *);
    static void ParseHeaderFields(const char *, Header *);
    static void ParseHeaderField(const char *, const char *, Header *);

    template <class T, std::size_t N = 10>
    static T ParseNumber(const char *);

    template <class T, std::size_t N = 10>
    static T ParseNumber(const char *, const char *);

    void initialize() noexcept;
    void move(Parser *) noexcept;
    void parseRequestStartLine(Request *);
    void parseHeader(Header *);
    std::tuple<bool, std::size_t> parseBodyOrChunkSize(Header *);
    std::size_t parseFirstChunkSize();
    std::size_t parseChunkSize();
    char *peekContent(std::size_t *);
    void discardContent(std::size_t);
    std::tuple<char *, std::size_t> peekCharsUntilCRLF(std::size_t);
    std::tuple<char *, std::size_t> peekCharsUntilCRLFCRLF(std::size_t);
};


class InvalidMessage
  : public std::exception
{
public:
    explicit InvalidMessage() noexcept;

    const char *what() const noexcept override;
};

} // namespace http

} // namespace siren


/*
 * #include "parser-inl.h"
 */


#include <utility>

#include <siren/assert.h>


namespace siren {

namespace http {

template <class ...T>
Parser::Parser(const Options &options, T &&...argument)
  : options_(options),
    inputStream_(std::forward<T>(argument)...)
{
    initialize();
}


bool
Parser::isValid() const noexcept
{
    return inputStream_.isValid();
}


template <class T>
void
Parser::getContent(std::size_t contentSize, T &&callback)
{
    SIREN_ASSERT(isValid());
    char *content = peekContent(&contentSize);
    callback(content, contentSize);
    discardContent(contentSize);
}

} // namespace http

} // namespace siren