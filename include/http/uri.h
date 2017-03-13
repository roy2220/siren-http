#pragma once


#include <cstddef>
#include <cstdint>
#include <string>


namespace siren {

namespace http {

class URI final
{
public:
    std::int32_t portNumber;

    inline explicit URI() noexcept;
    inline URI(URI &&) noexcept;
    inline URI &operator=(URI &&) noexcept;

    inline void reset() noexcept;
    inline const char *getSchemeName() const noexcept;
    inline const char *getUserInfo() const noexcept;
    inline const char *getHostName() const noexcept;
    inline const char *getPathName() const noexcept;
    inline const char *getQueryString() const noexcept;
    inline const char *getFragmentID() const noexcept;

    template <class ...T>
    inline void setSchemeName(T &&...);

    template <class ...T>
    inline void setUserInfo(T &&...);

    template <class ...T>
    inline void setHostName(T &&...);

    template <class ...T>
    inline void setPathName(T &&...);

    template <class ...T>
    inline void setQueryString(T &&...);

    template <class ...T>
    inline void setFragmentID(T &&...);

private:
    std::string base_;
    std::size_t schemeNameOffset_;
    std::size_t userInfoOffset_;
    std::size_t hostNameOffset_;
    std::size_t pathNameOffset_;
    std::size_t queryStringOffset_;
    std::size_t fragmentIDOffset_;

    inline void initialize() noexcept;
    inline void move(URI *) noexcept;
};

} // namespace http

} // namespace siren


/*
 * #include "http/uri-inl.h"
 */


#include <utility>


namespace siren {

namespace http {

URI::URI() noexcept
{
    initialize();
}


URI::URI(URI &&other) noexcept
  : base_(std::move(other.base_))
{
    other.move(this);
}


URI &
URI::operator=(URI &&other) noexcept
{
    if (&other != this) {
        base_ = std::move(other.base_);
        other.move(this);
    }

    return *this;
}


void
URI::initialize() noexcept
{
    schemeNameOffset_ = 0;
    userInfoOffset_ = 0;
    hostNameOffset_ = 0;
    pathNameOffset_ = 0;
    queryStringOffset_ = 0;
    fragmentIDOffset_ = 0;
}


void
URI::move(URI *other) noexcept
{
    other->schemeNameOffset_ = schemeNameOffset_;
    other->userInfoOffset_ = userInfoOffset_;
    other->hostNameOffset_ = hostNameOffset_;
    other->pathNameOffset_ = pathNameOffset_;
    other->queryStringOffset_ = queryStringOffset_;
    other->fragmentIDOffset_ = fragmentIDOffset_;
    initialize();
}


void
URI::reset() noexcept
{
    base_.clear();
    initialize();
}


const char *
URI::getSchemeName() const noexcept
{
    return base_.c_str() + schemeNameOffset_;
}


const char *
URI::getUserInfo() const noexcept
{
    return base_.c_str() + userInfoOffset_;
}


const char *
URI::getHostName() const noexcept
{
    return base_.c_str() + hostNameOffset_;
}


const char *
URI::getPathName() const noexcept
{
    return base_.c_str() + pathNameOffset_;
}


const char *
URI::getQueryString() const noexcept
{
    return base_.c_str() + queryStringOffset_;
}


const char *
URI::getFragmentID() const noexcept
{
    return base_.c_str() + fragmentIDOffset_;
}


template <class ...T>
void
URI::setSchemeName(T &&...argument)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(argument)...);
    schemeNameOffset_ = offset;
}


template <class ...T>
void
URI::setUserInfo(T &&...argument)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(argument)...);
    userInfoOffset_ = offset;
}


template <class ...T>
void
URI::setHostName(T &&...argument)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(argument)...);
    hostNameOffset_ = offset;
}


template <class ...T>
void
URI::setPathName(T &&...argument)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(argument)...);
    pathNameOffset_ = offset;
}


template <class ...T>
void
URI::setQueryString(T &&...argument)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(argument)...);
    queryStringOffset_ = offset;
}


template <class ...T>
void
URI::setFragmentID(T &&...argument)
{
    std::size_t offset = base_.size();
    base_.push_back('\0');
    base_.append(std::forward<T>(argument)...);
    fragmentIDOffset_ = offset;
}


} // namespace http

} // namespace siren
