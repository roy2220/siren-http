#include <cstdio>
#include <cstdlib>

#include <siren/test.h>


int main()
{
    using namespace siren;

    std::size_t m = RunTests();
    std::size_t n = GetNumberOfTests();
    std::printf("%zu/%zu passed\n", m, n);
    return m < n ? EXIT_FAILURE : EXIT_SUCCESS;
}
