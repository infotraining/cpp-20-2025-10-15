#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <array>

using namespace std::literals;

auto foo() -> int
{
    return 42;
}

auto add(auto a, auto b)
{
    return a + b;
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

template <typename TItem>
void add_to(auto& container, TItem&& item)
{
    if constexpr(requires { container.push_back(std::forward<TItem>(item)); }) // requires expression
        container.push_back(std::forward<TItem>(item));
    else
        container.insert(std::forward<TItem>(item));
}

TEST_CASE("add_to")
{
    std::vector<int> vec = {1, 2};
    add_to(vec, 3);
    CHECK(vec == std::vector{1, 2, 3});

    std::set<int> mset = {5, 4};
    add_to(mset, 1);

}

TEST_CASE("templates & lambda expressions")
{
    REQUIRE(true);
}

template <size_t N>
auto create_buffer()
{
    if constexpr(N < 256)
    {
        return std::array<std::byte, N>{};
    }
    else
    {
        return std::vector<std::byte>(N);
    }
}

TEST_CASE("create_buffer")
{
    auto small_buffer = create_buffer<128>();  // std::array<std::byte, 128>
    //auto large_buffer = create_buffer<1024>(); // std::vector<std::byte>
}