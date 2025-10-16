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