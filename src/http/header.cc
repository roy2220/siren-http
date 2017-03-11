#include "http/header.h"

#include <cstring>
#include <algorithm>


namespace siren {

namespace http {

void
Header::sortFields() noexcept
{
    std::sort(
        fields_.begin(),
        fields_.end(),

        [&] (const Field &field1, const Field &field2) -> bool {
            return std::strcmp(base_.c_str() + field1.nameOffset
                               , base_.c_str() + field2.nameOffset) < 0;
        }
    );
}


std::size_t
Header::searchFields(const char *fieldName) const noexcept
{
    std::vector<Field>::const_iterator it = std::lower_bound(
        fields_.begin(),
        fields_.end(),
        fieldName,

        [&] (const Field &field, const char *fieldName) -> bool {
            return std::strcmp(base_.c_str() + field.nameOffset, fieldName) < 0;
        }
    );

    if (it == fields_.end() || std::strcmp(base_.c_str() + it->nameOffset, fieldName) != 0) {
        return fields_.size();
    } else {
        return it - fields_.begin();
    }
 ;
}


} // namespace http

} // namespace siren
