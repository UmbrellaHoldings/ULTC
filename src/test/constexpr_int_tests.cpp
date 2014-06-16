#include <boost/test/unit_test.hpp>

#include "types/constexpr_int.h"

using namespace types;

using uint256 = ulongint<256>;

BOOST_AUTO_TEST_SUITE(constexpr_int_tests)

BOOST_AUTO_TEST_CASE(constexpr_int_ctrs)
{
    uint256 a(0xefffffff15223344);

#if 0
    uint256 num1 = 10;
    uint256 num2 = 11;
    BOOST_CHECK(num1+1 == num2);

    uint64 num3 = 10;
    BOOST_CHECK(num1 == num3);
    BOOST_CHECK(num1+num2 == num3+num2);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
