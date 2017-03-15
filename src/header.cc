#include "header.h"

namespace siren {

namespace http {

void
Header::sort()
{
    if (!isSorted_) {
        std::stable_sort(
            fields_.begin(),
            fields_.end(),

            [&] (const Field &field1, const Field &field2) -> bool {
                const char *fieldName1 = base_.c_str() + field1.nameOffset;
                const char *fieldName2 = base_.c_str() + field2.nameOffset;
                return std::strcmp(fieldName1, fieldName2) < 0;
            }
        );

        isSorted_ = true;
    }
}

} // namespace http

} // namespace siren
