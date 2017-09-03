#pragma once


#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <siren/utility.h>


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
    inline explicit Header() noexcept;
    inline Header(Header &&) noexcept;
    inline Header &operator=(Header &&) noexcept;

    inline void reset() noexcept;
    inline void removeField(std::size_t) noexcept;

    template <class T>
    inline void traverse(T &&) const;

    template <class T>
    inline void search(const char *, T &&) const;

    template <class T, class U>
    inline void addField(T &&, U &&);

    void sort();

private:
    typedef detail::HeaderField Field;

    std::string base_;
    std::vector<Field> fields_;
    bool isSorted_;

    inline void initialize() noexcept;
    inline void move(Header *) noexcept;

    template <class T>
    inline std::enable_if_t<SIREN_TEST_INSTANTIATION(T, std::tuple)
                            , std::size_t> addFieldNameOrValue(T &&);

    template <class T>
    inline std::enable_if_t<!SIREN_TEST_INSTANTIATION(T, std::tuple)
                            , std::size_t> addFieldNameOrValue(T &&);
};

} // namespace http

} // namespace siren


/*
 * #include "header-inl.h"
 */


#include <cstring>
#include <algorithm>
#include <utility>

#include <siren/assert.h>


namespace siren {

namespace http {

Header::Header() noexcept
{
    initialize();
}


Header::Header(Header &&other) noexcept
  : base_(std::move(other.base_)),
    fields_(std::move(other.fields_))
{
    other.move(this);
}


Header &
Header::operator=(Header &&other) noexcept
{
    if (&other != this) {
        base_ = std::move(other.base_);
        fields_ = std::move(other.fields_);
        other.move(this);
    }

    return *this;
}


void
Header::initialize() noexcept
{
    isSorted_ = true;
}


void
Header::move(Header *other) noexcept
{
    other->isSorted_ = isSorted_;
    initialize();
}


void
Header::reset() noexcept
{
    base_.clear();
    fields_.clear();
    initialize();
}


template <class T>
void
Header::traverse(T &&callback) const
{
    for (auto it = fields_.begin(); it < fields_.end(); ++it) {
        if (it->valueOffset >= 1) {
            const char *fieldName = base_.c_str() + it->nameOffset;
            const char *fieldValue = base_.c_str() + it->valueOffset;
            callback(it - fields_.begin(), fieldName, fieldValue);
        }
    }
}


template <class T>
void
Header::search(const char *fieldName, T &&callback) const
{
    SIREN_ASSERT(isSorted_);

    for (
        auto it = std::lower_bound(
            fields_.begin(),
            fields_.end(),
            fieldName,

            [&] (const Field &field, const char *fieldName) -> bool {
                return std::strcmp(base_.c_str() + field.nameOffset, fieldName) < 0;
            }
        );

        it < fields_.end() && std::strcmp(base_.c_str() + it->nameOffset, fieldName) == 0;
        ++it
    ) {
        if (it->valueOffset >= 1) {
            const char *fieldValue = base_.c_str() + it->valueOffset;

            if (!callback(it - fields_.begin(), fieldValue)) {
                break;
            }
        }
    }
}


template <class T, class U>
void
Header::addField(T &&fieldName, U &&fieldValue)
{
    fields_.push_back({addFieldNameOrValue(std::forward<T>(fieldName))
                       , addFieldNameOrValue(std::forward<U>(fieldValue))});
    isSorted_ = false;
}


template <class T>
std::enable_if_t<SIREN_TEST_INSTANTIATION(T, std::tuple), std::size_t>
Header::addFieldNameOrValue(T &&fieldNameOrValue)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();

    ApplyFunction([&] (auto &&...fieldNameOrValue) {
        base_.append(std::forward<decltype(fieldNameOrValue)>(fieldNameOrValue)...);
    }, std::forward<T>(fieldNameOrValue));

    return offset;
}


template <class T>
std::enable_if_t<!SIREN_TEST_INSTANTIATION(T, std::tuple), std::size_t>
Header::addFieldNameOrValue(T &&fieldNameOrValue)
{
    base_.push_back('\0');
    std::size_t offset = base_.size();
    base_.append(std::forward<T>(fieldNameOrValue));
    return offset;
}


void
Header::removeField(std::size_t fieldIndex) noexcept
{
    SIREN_ASSERT(fieldIndex < fields_.size());
    Field *field = &fields_[fieldIndex];
    field->valueOffset = 0;
}

} // namespace http

} // namespace siren
