#pragma once


#include "http/header.h"
#include "http/uri.h"


namespace siren {

namespace http {

enum class MethodType
{
    Connect = 0,
    Delete,
    Get,
    Head,
    Options,
    Patch,
    Post,
    Put,
    Trace,
};


struct Request
{
    MethodType methodType;
    URI uri;
    unsigned short majorVersionNumber;
    unsigned short minorVersionNumber;
    Header header;
};


const char *GetMethodName(MethodType) noexcept;

} // namespace http

} // namespace siren
