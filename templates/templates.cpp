#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <array>
#include <map>

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

//////////////////////////////////////////////////////
// Lambda features in C++20

TEST_CASE("explicit params for generic lambdas")
{
    auto add_42 = []<std::integral T>(std::vector<T>& container) 
        requires (sizeof(T) > 1)
    {
        container.push_back(42);
    };

    std::vector<int> vec;
    add_42(vec);

    CHECK(vec == std::vector{42});

    // std::vector<uint8_t> vec_bytes;
    // add_42(vec_bytes);

    auto add_value = []<typename TItem>(auto& container, TItem&& item) {
        container.push_back(std::forward<TItem>(item));
    };

    std::vector<std::string> words;
    add_value(words, "text"s);
}

// closure class - klasa domknięcia
class Lambda_472357645762
{
public:
    auto operator()(int n) { return n * n; }
};

TEST_CASE("default init")
{
    auto closure = [](int n) { return n * n; };
    auto closure_explain = Lambda_472357645762{};

    CHECK(closure(4) == 16);
    CHECK(closure_explain(5) == 25);

    decltype(closure) another_closure{}; // since C++20

    std::array data = {7, 6, 1, 0, 5};

    auto cmp_by_value = [](int* a, int* b) { return *a < *b; };

    std::set<int*, decltype(cmp_by_value)> mset = { &data[2], &data[1], &data[3], &data[0] };

    for(auto* ptr : mset)
    {
        std::cout << *ptr << " ";
    }
    std::cout << "\n";
}

TEST_CASE("capturing sb")
{
    std::map<int, std::string> dict = { {0, "zero"}, {1, "one"} };

    for(const auto& [key, value] : dict)
    {
        auto printer = [key, value] { 
            std::cout << key << " - " << value << "\n";
        };

        printer();
    }
}

auto create_caller(auto f, auto... args)
{
   return [f, ...args = std::move(args)]() -> decltype(auto) {
        return f(args...);
   }; 
}

int sum_of_three(int a, int b, int c)
{
    return a + b + c;
}

TEST_CASE("capturing variadic pack")
{
    auto f = create_caller(sum_of_three, 1, 2, 3);
    CHECK(f() == 6);
}