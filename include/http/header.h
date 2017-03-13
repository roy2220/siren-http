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
};

} // namespace http

} // namespace siren


/*
 * #include "http/header-inl.h"
 */


#include <cstring>
#include <algorithm>
#include <utility>

#include <siren/assert.h>
#include <siren/utility.h>


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

            if (!callback(it - fields_.begin(), fieldName, fieldValue)) {
                break;
            }
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
    isSorted_ = false;
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
