#include "request.h"


namespace siren {

namespace http {

const char *
GetMethodName(MethodType methodType) noexcept
{
    static const char *const methodNames[] = {
        "CONNECT",
        "DELETE",
        "GET",
        "HEAD",
        "OPTIONS",
        "PATCH",
        "POST",
        "PUT",
        "TRACE",
    };

    return methodNames[static_cast<int>(methodType)];
}

} // namespace http

} // namespace siren
