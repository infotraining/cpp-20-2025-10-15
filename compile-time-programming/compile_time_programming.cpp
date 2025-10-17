#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ranges>
#include <numeric>
#include <algorithm>

using namespace std::literals;

int runtime_function(int n)
{
    return n * n;
}

constexpr int constexpr_function(int n)
{
    return n * n;
}

consteval int consteval_function(int n)
{
    return n * n;
}

TEST_CASE("runtime vs. constexpr vs. consteval")
{
    int n = 4;
    CHECK(runtime_function(n) == 16);

    constexpr int contexpr_n = 4;
    static_assert(constexpr_function(contexpr_n) == 16); // constexpr
    CHECK(constexpr_function(n) == 16); // runtime

    //CHECK(consteval_function(n) == 16); // ERROR
    static_assert(consteval_function(contexpr_n) == 16);
    static_assert(consteval_function(5) == 25);
}

consteval int square(int n)
{
    return n * n;
}

void compile_time_error() // runtime-only
{}
 
consteval int next_two_digit_value(int value)
{
    if (value < 9 || value >= 99)
    {
        compile_time_error(); // illegal
    }
 
    return ++value;
}

constexpr int len(const char* s)
{
    if (std::is_constant_evaluated())
    { 
        // compile-time friendly code
        int idx = 0;
        while (s[idx] != '\0')
            ++idx;
        return idx;        
    }
    else
    {
        return std::strlen(s); // function called at runtime
    }
}

TEST_CASE("consteval")
{
    std::array squares = { square(1), square(2), square(3), square(4) };
    squares[0] = 0;    

    static_assert(next_two_digit_value(42) == 43);
    //static_assert(next_two_digit_value(99) == 43);

    const char* text = "Hello Consteval";
    CHECK(len(text) == 15);

    constexpr size_t text_size = len("Hello");
    static_assert(text_size == 5);
}

constexpr uintmax_t factorial(uintmax_t n)
{   
    if (n <= 1)
        return 1;
    
    return n * factorial(n - 1);
}

template <size_t N>
consteval auto create_factorial_lookup_table()
{
    std::array<uintmax_t, N> values{};

    auto factorial_values = 
        std::views::iota(0)
        | std::views::take(N)
        | std::views::transform(factorial);

    std::ranges::copy(factorial_values, values.begin());

    return values;
}

template <size_t N, typename F>
consteval auto create_lookup_table(F func)
{
    std::array<uintmax_t, N> values{};
    
    auto vw = std::views::iota(0)
        | std::views::take(N)
        | std::views::transform(func);

    std::ranges::copy(vw, values.begin());

    return values;
}

template <std::ranges::input_range... TRng_>
constexpr auto avg_for_unique(const TRng_&... rng)
{
    using TElement = std::common_type_t<std::ranges::range_value_t<TRng_>...>;

    std::vector<TElement> vec;                            // empty vector
    vec.reserve((rng.size() + ...));                      // reserve a buffer - fold expression C++17
    (vec.insert(vec.end(), rng.begin(), rng.end()), ...); // fold expression C++17

    // sort items
    std::ranges::sort(vec); // std::sort(vec.begin(), vec.end());

    // create span of unique_items
    auto new_end = std::unique(vec.begin(), vec.end());
    std::span unique_items{vec.begin(), new_end};

    // calculate sum of unique items
    auto sum = std::accumulate(unique_items.begin(), unique_items.end(), TElement{});

    return sum / static_cast<double>(unique_items.size());
}

TEST_CASE("avg for unique")
{
    constexpr std::array lst1 = {1, 2, 3, 4, 5};
    constexpr std::array lst2 = {5, 6, 7, 8, 9};

    constexpr auto avg = avg_for_unique(lst1, lst2);

    std::cout << "AVG: " << avg << "\n";
}

constexpr int fibonacci(int n)
{
    if (n <= 1)
        return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

TEST_CASE("constexpr extensions")
{
    constexpr std::array factorial_lookup_table = create_factorial_lookup_table<20>();
    constexpr std::array fibonacci_lookup_table = create_lookup_table<20>(fibonacci);
}

constexpr int safety_during_compiletime()
{
    int* ptr = new int[1024];

    ptr[665] = 0;

    // int* ptr_2 = nullptr;
    // int n = ptr[1];

    // std::vector vec = {1, 2, 3};
    // vec[6] = 8;

    delete[] ptr;

    return 42;
}

TEST_CASE("safety net for constexpr function")
{
    constexpr auto safe_test = safety_during_compiletime();
}