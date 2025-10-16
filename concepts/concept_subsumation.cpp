#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <deque>
#include <list>

using namespace std::literals;

struct BoundingBox
{
    int w, h;
};

struct Color
{
    uint8_t r, g, b;
};

struct Rect
{
    int w, h;
    Color color;

    void draw() const
    {
        std::cout << "Rect::draw()\n";
    }

    BoundingBox box() const noexcept
    {
        return BoundingBox{w, h};
    }
};

struct ColorRect : Rect
{
    Color color;

    Color get_color() const noexcept
    {
        return color;
    }

    void set_color(Color new_color)
    {
        std::cout << "Setting new color\n";
        color = new_color;
    }
};

// clang-format off
template <typename T>
concept Shape = requires(const T& obj)
{
    { obj.box() } noexcept -> std::same_as<BoundingBox>;
    obj.draw();
};
// clang-format on

// TODO: Add concept ShapeWithColor that subsumes Shape and requires getters/setters for color
template <typename T>
concept ColorShape = Shape<T> && requires(T obj, Color color) {
    { obj.get_color() } -> std::same_as<Color>;
    obj.set_color(color);
};

static_assert(Shape<Rect>);
static_assert(Shape<ColorRect>);

template <Shape T>
void render(T& shp)
{
    std::cout << "render<Shape T>\n";
    shp.draw();
}

template <ColorShape T>
void render(T& shp)
{
    shp.set_color(Color{0, 0, 0});
    shp.draw();
}

template <std::bidirectional_iterator It>
void my_algorithm(It start, It end)
{
    std::cout << "algorithm for BidiIt\n";
}

template <std::random_access_iterator It>
void my_algorithm(It start, It end)
{
    std::cout << "algorithm for RandIt\n";
}

template <std::contiguous_iterator It>
void my_algorithm(It start, It end)
{
    std::cout << "algorithm for ContigIt\n";
}

TEST_CASE("concept subsumation")
{
    Rect r{10, 20, {255, 0, 0}};
    ColorRect cr{10, 20, {0, 255, 0}};

    render(r);
    std::cout << "---------\n";
    render(cr);

    std::list<int> lst;
    std::deque<int> dq;
    std::vector<int> vec;

    my_algorithm(lst.begin(), lst.end());
    my_algorithm(dq.begin(), dq.end());
    my_algorithm(vec.begin(), vec.end());
}

template <typename T>
concept Printable = requires(T& obj, std::ostream& out) {
    out << obj;
};

template <typename... TArgs>
auto sum(TArgs... args) // sum(1, 2, 3, 4)
{
    return (... + args); // (((1 + 2) + 3) + 4)
}

template <typename THead, typename... TTail>
constexpr bool AllSameAs_v = (... && std::is_same_v<THead, TTail>);

static_assert(AllSameAs_v<int, int, int>);
static_assert(not AllSameAs_v<int, int, char>);

template <typename F, std::ranges::range... TRng>
    requires AllSameAs_v<std::ranges::range_value_t<TRng>...> 
void for_all(F func, TRng&... rngs)
{
    auto iterate_over = [func = std::move(func)](auto& rng) {
        for(auto&& item : rng)
            func(item);
    };

    (..., iterate_over(rngs)); // fold-expression
}

// todo: is it correct?
// template <typename F, std::ranges::range... TRng>
// auto for_all_2(F func, TRng&... rngs) -> void
//     requires((... && std::is_same_v<std::ranges::range_value_t<TRng>, std::ranges::range_value_t<decltype(std::get<0>(std::tie(rngs...)))>>))
// {
//     auto iterate_over = [func = std::move(func)](auto& rng) {
//         for (auto&& item : rng)
//             func(item);
//     };
//
//     (..., iterate_over(rngs));
// }

TEST_CASE("for_all")
{
    std::vector<int> vec = {1, 2, 3};
    std::deque<int> dq = {4, 5, 6};
    std::list<int> lst = {7, 8, 9};
    int tab[3] = {10, 11, 12};


    auto printer = [](Printable auto item) {
        std::cout << "Item: " << item << "\n";
    };

    for_all(printer, vec, dq, lst, tab);
}