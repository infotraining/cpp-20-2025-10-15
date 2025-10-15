#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

/////////////////////////////////////////////////
// trait

int foo(int arg)
{
    return arg * 2;
}

template <size_t Arg>
struct Foo
{
    constexpr static auto value = Arg * 2;
};

template <size_t Arg>
constexpr inline static auto Foo_v = Foo<Arg>::value;

/////////////////////////////////////////////////////////
template <typename T>
struct IsPointer
{
    constexpr static bool value = false;
};

template <typename T>
struct IsPointer<T*>
{
    constexpr static bool value = true;
};

template <typename T>
constexpr static inline bool IsPointer_v = IsPointer<T>::value;

//////////////////////////////////////////////////////////
template <typename T>
struct Identity
{
    using type = T;
};

template <typename T>
using Identity_t = typename Identity<T>::type;

static_assert(std::is_same_v<Identity<int>::type, int>);

///////////////////////////////////////////////////////////
template <typename T>
struct RemoveRef
{
    using type = T;
};

template <typename T>
struct RemoveRef<T&>
{
    using type = T;
};

template <typename T>
struct RemoveRef<T&&>
{
    using type = T;
};

template <typename T>
using RemoveRef_t = typename RemoveRef<T>::type;

TEST_CASE("traits")
{
    CHECK(foo(42) == 84);
    static_assert(Foo<42>::value == 84);
    static_assert(Foo_v<42> == 84);

    using TypeA = int;
    using TypeB = int*;

    static_assert(not IsPointer_v<TypeA>);
    static_assert(IsPointer_v<TypeB>);

    static_assert(std::is_same_v<RemoveRef_t<int>, int>);
    static_assert(std::is_same_v<RemoveRef_t<int&>, int>);
}

namespace ver_1
{
    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires IsPointer_v<T> // requires clause
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace ver_1

namespace ver_2
{
    template <typename T>
    concept Pointer = IsPointer_v<T>;

    static_assert(Pointer<int*>);
    static_assert(not Pointer<int>);

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace ver_2

inline namespace ver_3
{
    template <typename T>
    concept Pointer = requires(T ptr) // requires expression
    {
        *ptr;
        ptr == nullptr;
        ptr != nullptr;
    };

    static_assert(Pointer<int*>);
    static_assert(not Pointer<int>);

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }

    namespace Alternative
    {
        auto max_value(Pointer auto a, Pointer auto b) 
            requires std::same_as<decltype(a), decltype(b)> 
        {
            assert(a != nullptr);
            assert(b != nullptr);
            return *a < *b ? *b : *a;
        }
    }
} // namespace ver_3


std::integral auto add(std::integral auto a, std::integral auto b)
{
    return a + b;
}

TEST_CASE("constraints")
{
    int x = 10;
    int y = 20;

    CHECK(max_value(x, y) == 20);
    CHECK(max_value("abc"s, "def"s) == "def"s);

    CHECK(max_value(&x, &y) == 20);

    auto sp1 = std::make_shared<int>(42);
    auto sp2 = std::make_shared<int>(665);
    CHECK(max_value(sp1, sp2) == 665);

    std::integral auto result = add(42, 667UL);
}

template <typename T>
concept Printable = requires(T& obj, std::ostream& out) {
    out << obj;
};

template <typename T>
struct Wrapper
{
    T value;

    void print() const requires Printable<T>
    {
        std::cout << "value: " << value << "\n";
    }

    void print() const
    {
        std::cout << "values: [ ";
        for (const auto& item : value)
            std::cout << item << " ";
        std::cout << "]\n";
    }
};

TEST_CASE("concepts")
{
    Wrapper<int> w_int{42};
    w_int.print();

    // Wrapper<std::vector<int>> w_vec{std::vector{1, 2, 3}};
    // w_vec.print();
}