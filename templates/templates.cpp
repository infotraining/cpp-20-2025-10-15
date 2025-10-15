#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

auto foo() -> int
{
    return 42;
}

auto add(auto a, auto b)
{
    return a + b;
}

template <typename TItem>
void add_to(auto& container, TItem&& item)
{
    container.push_back(std::forward<TItem>(item));
}

namespace Explain
{
    namespace Cpp11
    {
        template <typename T1, typename T2>
        auto add(T1 a, T2 b) -> decltype(a + b)
        {
            return a + b;
        }
    } // namespace Cpp11

    namespace Cpp14
    {
        template <typename T1, typename T2>
        auto add(T1 a, T2 b)
        {
            return a + b;
        }
    } // namespace Cpp14

    template <typename T1, typename T2>
    auto add(T1 a, T2 b)
    {
        return a + b;
    }

} // namespace Explain

TEST_CASE("abbreviated function templates")
{
    auto describe = [](auto n) -> std::string {
        if (n % 2 == 0)
            return "even"s;
        return "odd";
    };

    int x = 42;
    unsigned int y = 6;

    CHECK(add(x, y) == 48);
}

TEST_CASE("templates & lambda expressions")
{
    REQUIRE(true);
}