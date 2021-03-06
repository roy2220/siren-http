#pragma once


#include <cstddef>
#include <exception>
#include <tuple>

#include "input_stream.h"


namespace siren {

namespace http {

class Header;
class ParseException;
class URI;
enum class MethodType;
enum class StatusCode;
struct Request;
struct Response;


struct ParseOptions
{
    std::size_t maxStartLineSize = 4 * 1024;
    std::size_t maxHeaderSize = 16 * 1024;
    std::size_t maxBodySize = 64 * 1024;
};


class Parser final
{
public:
    typedef ParseOptions Options;

    inline bool isValid() const noexcept;
    inline bool bodyIsChunked() const noexcept;
    inline std::size_t getRemainingBodyOrChunkSize() const noexcept;

    template <class ...T>
    inline explicit Parser(const Options &, T &&...);

    Parser(Parser &&) noexcept;
    Parser &operator=(Parser &&) noexcept;

    void getRequest(Request *);
    void getResponse(Response *);
    char *peekPayloadData(std::size_t);
    void discardPayloadData(std::size_t);

private:
    ParseOptions options_;
    std::size_t maxChunkSize_;
    InputStream inputStream_;
    bool bodyIsChunked_;

    union {
        std::size_t remainingBodySize_;
        std::size_t remainingChunkSize_;
        std::size_t remainingBodyOrChunkSize_;
    };

    static MethodType ParseMethod(const char *);
    static void ParseURI(const char *, URI *);
    static std::tuple<unsigned short, unsigned short> ParseVersion(const char *);
    static StatusCode ParseStatusCode(const char *);
    static void ParseHeaderFields(const char *, Header *);
    static void ParseHeaderField(const char *, const char *, Header *);

    template <class T, std::size_t N = 10>
    static T ParseNumber(const char *);

    template <class T, std::size_t N = 10>
    static T ParseNumber(const char *, const char *);

    void initialize() noexcept;
    void move(Parser *) noexcept;
    void parseRequestStartLine(Request *);
    void parseResponseStartLine(Response *);
    void parseHeader(Header *);
    std::tuple<bool, std::size_t> parseBodyOrChunkSize(Header *);
    std::size_t parseFirstChunkSize();
    std::size_t parseChunkSize();

    template <ParseException F()>
    std::tuple<char *, std::size_t> peekCharsUntilCRLF(std::size_t);

    template <ParseException F()>
    std::tuple<char *, std::size_t> peekCharsUntilCRLFCRLF(std::size_t);
};


enum class ParseExceptionType
{
    InvalidMessage = 0,
    UnknownMethod,
    UnknownStatus,
    StartLineTooLong,
    HeaderTooLarge,
    BodyTooLarge,
};


class ParseException final
  : public std::exception
{
public:
    typedef ParseExceptionType Type;

    inline Type getType() const noexcept;

    explicit ParseException(Type) noexcept;

    const char *what() const noexcept override;

private:
    Type type_;
};


inline ParseException InvalidMessage();
inline ParseException UnknownMethod();
inline ParseException UnknownStatus();
inline ParseException StartLineTooLong();
inline ParseException HeaderTooLarge();
inline ParseException BodyTooLarge();

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
Parser::Parser(const Options &options, T &&...inputStream)
  : options_(options),
    inputStream_(std::forward<T>(inputStream)...)
{
    initialize();
}


bool
Parser::isValid() const noexcept
{
    return inputStream_.isValid();
}


bool
Parser::bodyIsChunked() const noexcept
{
    SIREN_ASSERT(isValid());
    return bodyIsChunked_;
}


std::size_t
Parser::getRemainingBodyOrChunkSize() const noexcept
{
    SIREN_ASSERT(isValid());
    return remainingBodyOrChunkSize_;
}


ParseExceptionType
ParseException::getType() const noexcept
{
    return type_;
}


ParseException
InvalidMessage()
{
    return ParseException(ParseExceptionType::InvalidMessage);
}


ParseException
UnknownMethod()
{
    return ParseException(ParseExceptionType::UnknownMethod);
}


ParseException
UnknownStatus()
{
    return ParseException(ParseExceptionType::UnknownStatus);
}


ParseException
StartLineTooLong()
{
    return ParseException(ParseExceptionType::StartLineTooLong);
}


ParseException
HeaderTooLarge()
{
    return ParseException(ParseExceptionType::HeaderTooLarge);
}


ParseException
BodyTooLarge()
{
    return ParseException(ParseExceptionType::BodyTooLarge);
}

} // namespace http

} // namespace siren
