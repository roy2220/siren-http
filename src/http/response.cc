#include "http/response.h"

#include <algorithm>
#include <array>


namespace siren {

namespace http {

namespace {

static const std::array<std::pair<StatusCode, const char *>, 45> StatusCodesAndDescriptions = {{
    {StatusCode::Continue, "Continue"},
    {StatusCode::SwitchingProtocol, "Switching Protocol"},
    {StatusCode::OK, "OK"},
    {StatusCode::Created, "Created"},
    {StatusCode::Accepted, "Accepted"},
    {StatusCode::NonAuthoritativeInformation, "Non-Authoritative Information"},
    {StatusCode::NoContent, "No Content"},
    {StatusCode::ResetContent, "Reset Content"},
    {StatusCode::PartialContent, "Partial Content"},
    {StatusCode::MultipleChoices, "Multiple Choices"},
    {StatusCode::MovedPermanently, "Moved Permanently"},
    {StatusCode::Found, "Found"},
    {StatusCode::SeeOther, "See Other"},
    {StatusCode::NotModified, "Not Modified"},
    {StatusCode::TemporaryRedirect, "Temporary Redirect"},
    {StatusCode::PermanentRedirect, "Permanent Redirect"},
    {StatusCode::BadRequest, "Bad Request"},
    {StatusCode::Unauthorized, "Unauthorized"},
    {StatusCode::Forbidden, "Forbidden"},
    {StatusCode::NotFound, "Not Found"},
    {StatusCode::MethodNotAllowed, "Method Not Allowed"},
    {StatusCode::NotAcceptable, "Not Acceptable"},
    {StatusCode::ProxyAuthenticationRequired, "Proxy Authentication Required"},
    {StatusCode::RequestTimeout, "Request Timeout"},
    {StatusCode::Conflict, "Conflict"},
    {StatusCode::Gone, "Gone"},
    {StatusCode::LengthRequired, "Length Required"},
    {StatusCode::PreconditionFailed, "Precondition Failed"},
    {StatusCode::PayloadTooLarge, "Payload Too Large"},
    {StatusCode::URITooLong, "URI Too Long"},
    {StatusCode::UnsupportedMediaType, "Unsupported Media Type"},
    {StatusCode::RangeNotSatisfiable, "Range Not Satisfiable"},
    {StatusCode::ExpectationFailed, "Expectation Failed"},
    {StatusCode::UpgradeRequired, "Upgrade Required"},
    {StatusCode::PreconditionRequired, "Precondition Required"},
    {StatusCode::TooManyRequests, "Too Many Requests"},
    {StatusCode::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large"},
    {StatusCode::UnavailableForLegalReasons, "Unavailable For Legal Reasons"},
    {StatusCode::InternalServerError, "Internal Server Error"},
    {StatusCode::NotImplemented, "Not Implemented"},
    {StatusCode::BadGateway, "Bad Gateway"},
    {StatusCode::ServiceUnavailable, "Service Unavailable"},
    {StatusCode::GatewayTimeout, "Gateway Timeout"},
    {StatusCode::HTTPVersionNotSupported, "HTTP Version Not Supported"},
    {StatusCode::NetworkAuthenticationRequired, "Network Authentication Required"},
}};

} // namespace


const char *
DescribeStatus(StatusCode statusCode) noexcept
{
    auto it = std::lower_bound(
        StatusCodesAndDescriptions.begin(),
        StatusCodesAndDescriptions.end(),
        statusCode,

        [&] (const std::pair<StatusCode, const char *> &StatusCodeAndDescription
             , StatusCode statusCode) -> bool {
            return StatusCodeAndDescription.first < statusCode;
        }
    );

    return it->second;
}


bool
TestRawStatusCode(int rawStatusCode) noexcept
{
    auto it = std::lower_bound(
        StatusCodesAndDescriptions.begin(),
        StatusCodesAndDescriptions.end(),
        rawStatusCode,

        [&] (const std::pair<StatusCode, const char *> &StatusCodeAndDescription
             , int rawStatusCode) -> bool {
            return static_cast<int>(StatusCodeAndDescription.first) < rawStatusCode;
        }
    );

    return it < StatusCodesAndDescriptions.end() && static_cast<int>(it->first) == rawStatusCode;
}

} // namespace http

} // namespace siren
