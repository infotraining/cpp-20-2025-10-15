#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <numbers>

using namespace std::literals;

TEST_CASE("safe comparing integral numbers")
{
    int x = -42;
    unsigned int y = 665;

    SECTION("cmp_* comparers")
    {
        CHECK(std::cmp_less(x, y));
        CHECK(std::cmp_less_equal(x, y));
        CHECK(std::cmp_greater(y, x));
        CHECK(std::cmp_greater_equal(y, x));

        CHECK(std::in_range<uint8_t>(255));
        CHECK(not std::in_range<uint8_t>(257));
    }

    SECTION("with generic functions")
    {
        auto my_less_comparer = [](auto a, auto b) {
            if constexpr (std::is_integral_v<decltype(a)> 
                && std::is_integral_v<decltype(b)>)
                return std::cmp_less(a, b);
            return a < b;
        };

        CHECK(my_less_comparer(x, y) == true);
        CHECK(my_less_comparer("abc"s, "def"s) == true);
        CHECK(not my_less_comparer(std::numbers::pi_v<float>, std::numbers::e_v<float>));
    }
}