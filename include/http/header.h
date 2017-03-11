#pragma once


#include <cstddef>
#include <string>
#include <tuple>
#include <vector>


namespace siren {

namespace http {

namespace detail {

struct HeaderField
{
    std::size_t nameOffset;
    std::size_t valueOffset;
};

} // namespace detail


class Header
{
public:
    inline void reset() noexcept;
    inline std::size_t getNumberOfFields() const noexcept;
    inline std::tuple<const char *, const char *> getField(std::size_t) const noexcept;
    inline const char *getFieldName(std::size_t) const noexcept;
    inline const char *getFieldValue(std::size_t) const noexcept;

    template <class T, class U>
    inline void addField(T &&, U &&);

    void sortFields() noexcept;
    std::size_t searchFields(const char *) const noexcept;

private:
    typedef detail::HeaderField Field;

    std::string base_;
    std::vector<Field> fields_;
};

} // namespace http

} // namespace siren


/*
 * #include "header-inl.h"
 */


#include <siren/assert.h>
#include <siren/utility.h>


namespace siren {

namespace http {

void
Header::reset() noexcept
{
    base_.clear();
    fields_.clear();
}


std::size_t
Header::getNumberOfFields() const noexcept
{
    return fields_.size();
}


std::tuple<const char *, const char *>
Header::getField(std::size_t fieldIndex) const noexcept
{
    SIREN_ASSERT(fieldIndex < fields_.size());
    const Field *field = &fields_[fieldIndex];
    return std::make_tuple(base_.c_str() + field->nameOffset, base_.c_str() + field->valueOffset);
}


const char *
Header::getFieldName(std::size_t fieldIndex) const noexcept
{
    SIREN_ASSERT(fieldIndex < fields_.size());
    return base_.c_str() + fields_[fieldIndex].nameOffset;
}


const char *
Header::getFieldValue(std::size_t fieldIndex) const noexcept
{
    SIREN_ASSERT(fieldIndex < fields_.size());
    return base_.c_str() + fields_[fieldIndex].valueOffset;
}


template <class T, class U>
void
Header::addField(T &&arguments1, U &&arguments2)
{
    base_.push_back('\0');
    std::size_t offset1 = base_.size();

    ApplyFunction([&] (auto &&...argument) {
        base_.append(std::forward<decltype(argument)>(argument)...);
    }, std::forward<T>(arguments1));

    base_.push_back('\0');
    std::size_t offset2 = base_.size();

    ApplyFunction([&] (auto &&...argument) {
        base_.append(std::forward<decltype(argument)>(argument)...);
    }, std::forward<U>(arguments2));

    fields_.push_back({offset1, offset2});
}


} // namespace http

} // namespace siren
