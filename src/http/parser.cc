#include "http/parser.h"

#include <cstring>
#include <limits>

#include <siren/utility.h>

#include "http/request.h"


namespace siren {

namespace http {

#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHANUM ALPHA DIGIT
#define DIGIT "0123456789"
#define HEXDIGIT DIGIT "abcdefABCDEF"
#define OCTDIGIT "01234567"
#define SPACE " \t\n\v\f\r"


namespace {

struct {
    unsigned char alpha: 1;
    unsigned char alphanum: 1;
    unsigned char digit: 1;
    unsigned char hexdigit: 1;
    unsigned char octdigit: 1;
    unsigned char space: 1;
} CharFlags[256];


char tolower(char) noexcept;
bool isalpha(char c) noexcept;
bool isalphanum(char c) noexcept;
bool isspace(char c) noexcept;
void InitializeCharFlags() noexcept;

template <std::size_t N = 10>
bool isdigit(char) noexcept;

template <>
bool isdigit<8>(char) noexcept;

template <>
bool isdigit<10>(char) noexcept;

template <>
bool isdigit<16>(char) noexcept;

template <std::size_t N = 10>
int digit2int(char) noexcept;

template <>
int digit2int<8>(char) noexcept;

template <>
int digit2int<10>(char) noexcept;

template <>
int digit2int<16>(char) noexcept;

} // namespace


MethodType
Parser::ParseMethod(const char *s)
{
    switch (*s) {
    case 'C':
        if (std::strcmp(s + 1, "ONNECT") == 0) {
            return MethodType::Connect;
        } else {
            break;
        }

    case 'D':
        if (std::strcmp(s + 1, "ELETE") == 0) {
            return MethodType::Delete;
        } else {
            break;
        }

    case 'G':
        if (std::strcmp(s + 1, "ET") == 0) {
            return MethodType::Get;
        } else {
            break;
        }

    case 'H':
        if (std::strcmp(s + 1, "EAD") == 0) {
            return MethodType::Head;
        } else {
            break;
        }

    case 'O':
        if (std::strcmp(s + 1, "PTIONS") == 0) {
            return MethodType::Options;
        } else {
            break;
        }

    case 'P':
        switch (s[1]) {
        case 'A':
            if (std::strcmp(s + 2, "TCH") == 0) {
                return MethodType::Patch;
            } else {
                break;
            }

        case 'O':
            if (std::strcmp(s + 2, "ST") == 0) {
                return MethodType::Post;
            } else {
                break;
            }

        case 'U':
            if (std::strcmp(s + 2, "T") == 0) {
                return MethodType::Put;
            } else {
                break;
            }
        }

        break;

    case 'T':
        if (std::strcmp(s + 1, "RACE") == 0) {
            return MethodType::Trace;
        } else {
            break;
        }
    }

    throw InvalidMessage();
}


void
Parser::ParseURI(const char *s, URI *uri)
{
    if (*s == '*') {
        if (s[1] != '\0') {
            throw InvalidMessage();
        }

        return;
    } else {
        const char *schemeNameStart;
        const char *schemeNameEnd;
        const char *userInfoStart;
        const char *userInfoEnd;
        const char *hostNameStart;
        const char *hostNameEnd;
        const char *portNumberStart;
        const char *portNumberEnd;
        const char *pathNameStart;

        if (*s == '/') {
            schemeNameStart = schemeNameEnd = nullptr;
            userInfoStart = userInfoEnd = nullptr;
            hostNameStart = hostNameEnd = nullptr;
            portNumberStart = portNumberEnd = nullptr;
            pathNameStart = s;
        } else {
            schemeNameStart = s;

            for (schemeNameEnd = schemeNameStart; *schemeNameEnd != '\0'; ++schemeNameEnd) {
                if (*schemeNameEnd == ':') {
                    break;
                }
            }

            if (*schemeNameEnd == '\0') {
                throw InvalidMessage();
            }

            if (!(schemeNameEnd[1] == '/' && schemeNameEnd[2] == '/')) {
                throw InvalidMessage();
            }

            hostNameStart = schemeNameEnd + 3;

            for (pathNameStart = hostNameStart; *pathNameStart != '\0'; ++pathNameStart) {
                if (*pathNameStart == '/') {
                    break;
                }
            }

            if (*pathNameStart == '\0') {
                throw InvalidMessage();
            }

            hostNameEnd = pathNameStart;

            for (userInfoEnd = hostNameStart; userInfoEnd < hostNameEnd; ++userInfoEnd) {
                if (*userInfoEnd == '@') {
                    break;
                }
            }

            if (userInfoEnd < hostNameEnd) {
                userInfoStart = hostNameStart;
                hostNameStart = userInfoEnd + 1;
            } else {
                userInfoStart = userInfoEnd = nullptr;
            }

            for (portNumberStart = hostNameEnd; portNumberStart > hostNameStart
                 ; --portNumberStart) {
                if (portNumberStart[-1] == ':') {
                    break;
                }
            }

            if (portNumberStart > hostNameStart) {
                portNumberEnd = hostNameEnd;
                hostNameEnd = portNumberStart - 1;
            } else {
                portNumberStart = portNumberEnd = nullptr;
            }
        }

        const char *pathNameEnd;

        for (pathNameEnd = pathNameStart; *pathNameEnd != '\0'; ++pathNameEnd) {
            if (*pathNameEnd == '?' || *pathNameEnd == '#') {
                break;
            }
        }

        const char *queryStringStart;
        const char *queryStringEnd;
        const char *fragmentIDStart;
        const char *fragmentIDEnd;

        if (*pathNameEnd == '?') {
            queryStringStart = pathNameEnd + 1;

            for (queryStringEnd = queryStringStart; *queryStringEnd != '\0'; ++queryStringEnd) {
                if (*queryStringEnd == '#') {
                    break;
                }
            }

            if (*queryStringEnd == '#') {
                fragmentIDStart = queryStringEnd + 1;
                for (fragmentIDEnd = fragmentIDStart; *fragmentIDEnd != '\0'; ++fragmentIDEnd);
            } else {
                fragmentIDStart = fragmentIDEnd = nullptr;
            }
        } else if (*pathNameEnd == '#') {
            queryStringStart = queryStringEnd = nullptr;
            fragmentIDStart = pathNameEnd + 1;
            for (fragmentIDEnd = fragmentIDStart; *fragmentIDEnd != '\0'; ++fragmentIDEnd);
        } else {
            queryStringStart = queryStringEnd = nullptr;
            fragmentIDStart = fragmentIDEnd = nullptr;
        }

        uri->setSchemeName(schemeNameStart, schemeNameEnd);
        uri->setUserInfo(userInfoStart, userInfoEnd);
        uri->setHostName(hostNameStart, hostNameEnd);

        if (portNumberStart < portNumberEnd) {
            uri->portNumber = ParseNumber<std::uint16_t>(portNumberStart, portNumberEnd);
        } else {
            uri->portNumber = -1;
        }

        uri->setPathName(pathNameStart, pathNameEnd);
        uri->setQueryString(queryStringStart, queryStringEnd);
        uri->setFragmentID(fragmentIDStart, fragmentIDEnd);
    }
}


std::tuple<unsigned short, unsigned short>
Parser::ParseVersion(const char *s)
{
    if (std::strncmp(s, "HTTP/", sizeof("HTTP/") - 1) != 0) {
        throw InvalidMessage();
    }

    const char *majorVersionNumberStart = s + sizeof("HTTP/") - 1;
    const char *majorVersionNumberEnd;

    for (majorVersionNumberEnd = majorVersionNumberStart; *majorVersionNumberEnd != '\0'
         ; ++majorVersionNumberEnd) {
        if (*majorVersionNumberEnd == '.') {
            break;
        }
    }

    if (*majorVersionNumberEnd == '\0') {
        throw InvalidMessage();
    }

    if (majorVersionNumberStart == majorVersionNumberEnd) {
        throw InvalidMessage();
    }

    unsigned short majorVersionNumber = ParseNumber<unsigned short>(majorVersionNumberStart
                                                                    , majorVersionNumberEnd);
    const char *minorVersionNumberStart = majorVersionNumberEnd + 1;

    if (*minorVersionNumberStart == '\0') {
        throw InvalidMessage();
    }

    unsigned short minorVersionNumber = ParseNumber<unsigned short>(minorVersionNumberStart);
    return std::make_tuple(majorVersionNumber, minorVersionNumber);
}


void
Parser::ParseHeaderFields(const char *s, Header *header)
{
    const char *headerFieldStart = s;

    do {
        const char *headerFieldEnd = headerFieldStart;

        for (;;) {
            int c1 = *headerFieldEnd;
            int c2 = headerFieldEnd[1];

            if (c2 == '\n') {
                if (c1 == '\r') {
                    break;
                } else {
                    headerFieldEnd += 2;
                }
            } else {
                if (c2 == '\r') {
                    headerFieldEnd += 1;
                } else {
                    headerFieldEnd += 2;
                }
            }
        }

        ParseHeaderField(headerFieldStart, headerFieldEnd, header);
        headerFieldStart = headerFieldEnd + 2;
    } while (*headerFieldStart != '\0');
}


template <class T, std::size_t N>
T
Parser::ParseNumber(const char *s)
{
    constexpr T k1 = std::numeric_limits<T>::max() / N;
    constexpr T k2 = std::numeric_limits<T>::max() % N;

    T number = 0;

    for (; *s != '\0'; ++s) {
        if (!isdigit<N>(*s)) {
            throw InvalidMessage();
        }

        if (number > k1 || (number == k1 && digit2int<N>(*s) > k2)) {
            throw InvalidMessage();
        }

        number = N * number + digit2int<N>(*s);
    }

    return number;
}


template <class T, std::size_t N>
T
Parser::ParseNumber(const char *s1, const char *s2)
{
    constexpr T k1 = std::numeric_limits<T>::max() / N;
    constexpr T k2 = std::numeric_limits<T>::max() % N;

    T number = 0;

    for (; s1 < s2; ++s1) {
        if (!isdigit<N>(*s1)) {
            throw InvalidMessage();
        }

        if (number > k1 || (number == k1 && digit2int<N>(*s1) > k2)) {
            throw InvalidMessage();
        }

        number = N * number + digit2int<N>(*s1);
    }

    return number;
}


void
Parser::ParseHeaderField(const char *s1, const char *s2, Header *header)
{
    const char *headerFieldNameStart = s1;
    const char *headerFieldNameEnd;

    for (headerFieldNameEnd = headerFieldNameStart; headerFieldNameEnd < s2; ++headerFieldNameEnd) {
        if (*headerFieldNameEnd == ':') {
            break;
        }
    }

    if (headerFieldNameEnd == s2) {
        throw InvalidMessage();
    }

    if (headerFieldNameStart == headerFieldNameEnd) {
        throw InvalidMessage();
    }

    const char *headerFieldValueStart;

    for (headerFieldValueStart = headerFieldNameEnd + 1; headerFieldValueStart < s2
         ; ++headerFieldValueStart) {
        if (!isspace(*headerFieldValueStart)) {
            break;
        }
    }

    const char *headerFieldValueEnd;

    for (headerFieldValueEnd = s2; headerFieldValueEnd > headerFieldValueStart
         ; --headerFieldValueEnd) {
        if (!isspace(headerFieldValueEnd[-1])) {
            break;
        }
    }

    header->addField(std::make_tuple(headerFieldNameStart, headerFieldNameEnd)
                     , std::make_tuple(headerFieldValueStart, headerFieldValueEnd));
}


Parser::Parser(Parser &&other) noexcept
  : options_(other.options_),
    inputStream_(std::move(other.inputStream_))
{
    other.move(this);
}


Parser &
Parser::operator=(Parser &&other) noexcept
{
    if (&other != this) {
        options_ = other.options_;
        inputStream_ = std::move(other.inputStream_);
        other.move(this);
    }

    return *this;
}


void
Parser::initialize() noexcept
{
    bodyIsChunked_ = false;
    bodySize_ = 0;
    InitializeCharFlags();
}


void
Parser::move(Parser *other) noexcept
{
    if (bodyIsChunked_) {
        other->maxChunkSize_ = maxChunkSize_;
    }

    other->bodyIsChunked_ = bodyIsChunked_;
    other->bodyOrChunkSize_ = bodyOrChunkSize_;
}


void
Parser::getRequest(Request *request)
{
    SIREN_ASSERT(isValid());
    SIREN_ASSERT(!bodyIsChunked_ && bodySize_ == 0);
    SIREN_ASSERT(request != nullptr);
    parseRequestStartLine(request);
    parseHeader(&request->header);
    std::tie(bodyIsChunked_, bodyOrChunkSize_) = parseBodyOrChunkSize(&request->header);
}


void
Parser::parseRequestStartLine(Request *request)
{
    char *s;
    std::size_t n;
    std::tie(s, n) = peekCharsUntilCRLF(options_.maxStartLineSize);
    s[n - 2] = '\0';
    char *methodNameStart = s;
    char *methodNameEnd;

    for (methodNameEnd = methodNameStart; *methodNameEnd != '\0'; ++methodNameEnd) {
        if (isspace(*methodNameEnd)) {
            break;
        }
    }

    if (*methodNameEnd == '\0') {
        throw InvalidMessage();
    }

    *methodNameEnd = '\0';
    char *uriStart;

    for (uriStart = methodNameEnd + 1; *uriStart != '\0'; ++uriStart) {
        if (!isspace(*uriStart)) {
            break;
        }
    }

    if (*uriStart == '\0') {
        throw InvalidMessage();
    }

    char *uriEnd;

    for (uriEnd = uriStart; *uriEnd != '\0'; ++uriEnd) {
        if (isspace(*uriEnd)) {
            break;
        }
    }

    if (*uriEnd == '\0') {
        throw InvalidMessage();
    }

    *uriEnd = '\0';
    char *versionStart;

    for (versionStart = uriEnd + 1; *versionStart != '\0'; ++versionStart) {
        if (!isspace(*versionStart)) {
            break;
        }
    }

    if (*versionStart == '\0') {
        throw InvalidMessage();
    }

    request->methodType = ParseMethod(methodNameStart);
    ParseURI(uriStart, &request->uri);
    std::tie(request->majorVersionNumber, request->minorVersionNumber) = ParseVersion(versionStart);
    inputStream_.discardChars(n);
}


void
Parser::parseHeader(Header *header)
{
    std::size_t n = 2;
    char *s = inputStream_.peekChars(n);
    bool headerHasFields = !(*s == '\r' && s[1] == '\n');

    if (headerHasFields) {
        std::tie(s, n) = peekCharsUntilCRLFCRLF(options_.maxHeaderSize);
        s[n - 2] = '\0';
        const char *headerFieldsStart = s;
        ParseHeaderFields(headerFieldsStart, header);
    }

    inputStream_.discardChars(n);
}


std::tuple<bool, std::size_t>
Parser::parseBodyOrChunkSize(Header *header)
{
    header->sortFields();
    bool bodyIsChunked;

    union {
        std::size_t bodySize;
        std::size_t chunkSize;
        std::size_t bodyOrChunkSize;
    };

    bool bodySizeIsDefined = false;

    {
        std::size_t fieldIndex = header->searchFields("Transfer-Encoding");

        if (fieldIndex == header->getNumberOfFields()) {
            bodyIsChunked = false;
        } else {
            const char *fieldValue = header->getFieldValue(fieldIndex);
            bodyIsChunked = std::strcmp(fieldValue, "chunked") == 0;
        }
    }

    {
        std::size_t fieldIndex = header->searchFields("Content-Length");

        if (fieldIndex != header->getNumberOfFields()) {
            const char *fieldValue = header->getFieldValue(fieldIndex);

            if (*fieldValue != '\0') {
                bodySize = ParseNumber<std::size_t, 8>(fieldValue);
                bodySizeIsDefined = true;
            }
        }
    }

    if (bodyIsChunked) {
        if (bodySizeIsDefined) {
            throw InvalidMessage();
        }

        chunkSize = parseFirstChunkSize();
    } else {
        if (bodySizeIsDefined) {
            if (bodySize > options_.maxBodySize) {
                throw InvalidMessage();
            }
        } else {
            bodySize = 0;
        }
    }

    return std::make_tuple(bodyIsChunked, bodyOrChunkSize);
}


std::size_t
Parser::parseFirstChunkSize()
{
    maxChunkSize_ = options_.maxBodySize;
    return parseChunkSize();
}


std::size_t
Parser::parseChunkSize()
{
    char *s;
    std::size_t n;
    std::tie(s, n) = peekCharsUntilCRLF((std::numeric_limits<std::size_t>::digits + 3) / 4 + 2);
    char *chunkSizeStart = s;
    char *chunkSizeEnd = s + n - 2;

    if (chunkSizeStart == chunkSizeEnd) {
        throw InvalidMessage();
    }

    std::size_t chunkSize = ParseNumber<std::size_t, 16>(chunkSizeStart, chunkSizeEnd);

    if (chunkSize > maxChunkSize_) {
        throw InvalidMessage();
    }

    maxChunkSize_ -= chunkSize;
    inputStream_.discardChars(n);
    return chunkSize;
}


char *
Parser::peekContent(std::size_t *contentSize)
{
    char *content;

    if (*contentSize < bodyOrChunkSize_) {
        content = inputStream_.peekChars(*contentSize);
    } else {
        if (bodyIsChunked_) {
            content = inputStream_.peekChars(chunkSize_ + 2);

            if (!(content[chunkSize_] == '\r' && content[chunkSize_ + 1] == '\n')) {
                throw InvalidMessage();
            }
        } else {
            content = inputStream_.peekChars(bodySize_);
        }

        *contentSize = bodyOrChunkSize_;
    }

    return content;
}


void
Parser::discardContent(std::size_t contentSize)
{
    if (contentSize < bodyOrChunkSize_) {
        inputStream_.discardChars(contentSize);
        bodySize_ -= contentSize;
    } else {
        if (bodyIsChunked_) {
            inputStream_.discardChars(chunkSize_ + 2);

            if (chunkSize_ == 0) {
                bodyIsChunked_ = false;
            } else {
                chunkSize_ = parseChunkSize();
            }
        } else {
            inputStream_.discardChars(bodySize_);
            bodySize_ = 0;
        }
    }
}


std::tuple<char *, std::size_t>
Parser::peekCharsUntilCRLF(std::size_t maxNumberOfChars)
{
    std::size_t charCount = 2;

    for (;;) {
        if (charCount > maxNumberOfChars) {
            throw InvalidMessage();
        }

        char *chars = inputStream_.peekChars(charCount);
        int c1 = chars[charCount - 2];
        int c2 = chars[charCount - 1];

        if (c2 == '\n') {
            if (c1 == '\r') {
                return std::make_tuple(chars, charCount);
            } else {
                charCount += 2;
            }
        } else {
            if (c2 == '\r') {
                charCount += 1;
            } else {
                charCount += 2;
            }
        }
    }
}


std::tuple<char *, std::size_t>
Parser::peekCharsUntilCRLFCRLF(std::size_t maxNumberOfChars)
{
    std::size_t charCount = 4;

    for (;;) {
        if (charCount > maxNumberOfChars) {
            throw InvalidMessage();
        }

        char *chars = inputStream_.peekChars(charCount);
        int c1 = chars[charCount - 4];
        int c2 = chars[charCount - 3];
        int c3 = chars[charCount - 2];
        int c4 = chars[charCount - 1];

        if (c4 == '\n') {
            if (c3 == '\r') {
                if (c2 == '\n' && c1 == '\r') {
                    return std::make_tuple(chars, charCount);
                } else {
                    charCount += 2;
                }
            } else {
                charCount += 4;
            }
        } else {
            if (c4 == '\r') {
                if (c3 == '\n' && c2 == '\r') {
                    charCount += 1;
                } else {
                    charCount += 3;
                }
            } else {
                charCount += 4;
            }
        }
    }
}


InvalidMessage::InvalidMessage() noexcept
{
}


const char *InvalidMessage::what() const noexcept
{
    return "Siren: Invalid HTTP message";
}


namespace {

char
tolower(char c) noexcept
{
    return UnsignedToSigned<unsigned char>(c | 0x20);
}


bool
isalpha(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].alpha;
}


bool
isalphanum(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].alphanum;
}


bool
isspace(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].space;
}


template <>
bool
isdigit<8>(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].octdigit;
}


template <>
bool
isdigit<10>(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].digit;
}


template <>
bool
isdigit<16>(char c) noexcept
{
    return CharFlags[static_cast<unsigned char>(c)].hexdigit;
}


template <>
int
digit2int<8>(char c) noexcept
{
    return c - '0';
}


template <>
int
digit2int<10>(char c) noexcept
{
    return c - '0';
}


template <>
int
digit2int<16>(char c) noexcept
{
    return isdigit<10>(c) ? digit2int<10>(c) : 10 + (tolower(c) - 'a');
}


void
InitializeCharFlags() noexcept
{
    static bool charFlagsAreInitialized = false;

    if (!charFlagsAreInitialized) {
        std::memset(CharFlags, '\0', sizeof(CharFlags));

        for (char c : ALPHA) {
            CharFlags[static_cast<unsigned char>(c)].alpha = 1;
        }

        for (char c : ALPHANUM) {
            CharFlags[static_cast<unsigned char>(c)].alphanum = 1;
        }

        for (char c : DIGIT) {
            CharFlags[static_cast<unsigned char>(c)].digit = 1;
        }

        for (char c : HEXDIGIT) {
            CharFlags[static_cast<unsigned char>(c)].hexdigit = 1;
        }

        for (char c : OCTDIGIT) {
            CharFlags[static_cast<unsigned char>(c)].octdigit = 1;
        }

        for (char c : SPACE) {
            CharFlags[static_cast<unsigned char>(c)].space = 1;
        }

        charFlagsAreInitialized = true;
    }
}

} // namespace

} // namespace http

} // namespace siren
