#include <catch2/catch_test_macros.hpp>
#include <deque>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std::literals;

struct Person
{
    int id;
    std::string name;
};

TEST_CASE("ranges", "[ranges]")
{
    auto data = helpers::create_numeric_dataset<20>(42);
    helpers::print(data, "data");

    std::vector words = {"one"s, "two"s, "three"s, "four"s, "five"s, "six"s, "seven"s, "eight"s, "nine"s, "ten"s,
        "eleven"s, "twelve"s, "thirteen"s, "fourteen"s, "fifteen"s, "sixteen"s, "seventeen"s, "eighteen"s, "nineteen"s, "twenty"s};
    helpers::print(words, "words");

    SECTION("algorithms")
    {
        // std::sort(words.begin(), words.end());
        std::ranges::sort(words, std::greater{});
        CHECK(std::ranges::is_sorted(words, std::greater{}));
    }

    SECTION("projections")
    {
        std::ranges::sort(words, std::less{}, /* projection */ &std::string::size);
        helpers::print(words, "words after sort");

        std::vector<Person> people = {Person{42, "Jan"}, {55, "Adam"}, {88, "Zenon"}};

        // std::sort(people.begin(), people.end(),
        //           [](const Person& p1, const Person& p2) { return p1.name <  p2.name; });
        std::ranges::sort(people, std::less{}, &Person::name);
        CHECK(std::ranges::is_sorted(people, std::less{}, &Person::name));
    }

    SECTION("concepts & tools")
    {
        int vec[] = {1, 2, 3};
        static_assert(std::ranges::range<decltype(vec)>);
        static_assert(std::ranges::contiguous_range<decltype(vec)>);

        using Iterator = std::ranges::iterator_t<decltype(vec)>;
        using ValueType = std::ranges::range_value_t<decltype(vec)>;
    }
}

namespace Check
{
    template <std::input_iterator It, typename Value>
    struct EndValue
    {
        It end_;
        Value value_;

        EndValue(It end, const Value& value)
            : end_{end}
            , value_{value}
        { }

        bool operator==(std::input_iterator auto it) const
        {
            return it == end_ || *it == value_;
        }
    };
} // namespace Check

template <auto Value>
struct EndValue
{
    bool operator==(auto it) const
    {
        return *it == Value;
    }
};

TEST_CASE("sentinels", "[ranges]")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    // EndValue sentinel_42_or_end(data.end(), 42);
    // CHECK((sentinel_42_or_end == data.begin()) == false);
    // CHECK((sentinel_42_or_end == data.end()) == true);
    // CHECK((sentinel_42_or_end == data.begin() + 5) == true);

    std::ranges::sort(data.begin(), EndValue<42>{});
    helpers::print(data, "data");

    char txt[] = {'a', 'b', 'c', '\0', 'e', 'f'};
    std::ranges::sort(std::ranges::begin(txt), EndValue<'\0'>{}, std::greater{});
    helpers::print(txt, "txt");
}

TEST_CASE("views")
{
    std::deque data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    SECTION("all")
    {
        auto all_view = std::views::all(data);
        auto target_view = all_view;

        helpers::print(target_view, "target_view");
    }

    SECTION("subrange - iterator & sentinel as a view")
    {
        auto head = std::ranges::subrange{data.begin(), EndValue<42>{}};
        std::ranges::sort(head, std::greater{});
        helpers::print(data, "data");

        for (auto& item : head)
            item = 0;

        head[0] = 665;
        helpers::print(data, "data");
    }

    SECTION("counted")
    {
        auto tail = std::views::counted(data.rbegin(), 3);
        helpers::print(tail, "tail");
    }

    SECTION("iota")
    {
        auto numbers = std::views::iota(1, 10);
        helpers::print(numbers, "numbers");
    }

    SECTION("single")
    {
        double pi = 3.14;
        auto view_pi = std::views::single(pi);
        helpers::print(view_pi, "pi");
    }

    SECTION("pipes |")
    {
        auto numbers_by_compostion = std::views::take(std::views::iota(1), 10);

        auto numbers = std::views::iota(1)
            | std::views::drop(10)
            | std::views::filter([](int n) { return n % 2 == 0; })
            | std::views::transform([](int n) { return n * n; })
            | std::views::take(10)
            | std::views::reverse;

        for (const auto& item : numbers)
        {
            std::cout << item << "; ";
        }
        std::cout << "\n";

        for (auto it = numbers.begin(); it != numbers.end(); ++it)
        {
            const auto& item = *it;
        }
    }

    SECTION("tuples & pairs")
    {
        std::vector<std::tuple<std::string, double>> vec = {{"pi", 3.14}, {"e", 2.71}};

        // auto keys = vec | std::views::elements<0>;
        auto keys = vec | std::views::keys;
        // auto values = vec | std::views::elements<1>;
        auto values = vec | std::views::keys;

        helpers::print(keys, "keys");
        helpers::print(values, "values");
    }
}

TEST_CASE("split")
{
    std::string text = "abc def ghi";

    auto tokens_view = std::views::split(text, " ")
        | std::views::transform([](auto token) { return std::string_view(token.data(), token.size()); });

    helpers::print(tokens_view, "tokens");
}

TEST_CASE("views - reference semantics")
{
    std::vector data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto evens_view = data | std::views::filter([](int i) { return i % 2 == 0; });
    helpers::print(data, "data");

    for (auto& item : evens_view)
    {
        item = 0;
    }

    helpers::print(data, "data");
}

TEST_CASE("storing to container")
{
    std::vector data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    SECTION("C++20")
    {
        auto evens_view = data
            | std::views::filter([](int i) { return i % 2 == 0; })
                        | std::views::take(3)
                                    | std::views::common;

        std::vector<int> evens(evens_view.begin(), evens_view.end());

        CHECK(evens == std::vector{2, 4, 6});
    }

    SECTION("C++23")
    {
        auto evens = data
            | std::views::filter([](int i) { return i % 2 == 0; })
            | std::views::take(3)
            | std::ranges::to<std::vector>();

        helpers::print(evens, "evens");

        auto odds = data | std::views::filter([](int i) { return i % 2 != 0; });

        std::vector vec_odds(std::from_range, odds); // constructs from range
        helpers::print(vec_odds, "vec_odds");
    }
}

TEST_CASE("range-based for")
{
    for (const int n : std::views::iota(1) | std::views::take(10))
    {
        std::cout << n << " ";
    }
}

TEST_CASE("C++23 - new adaptors")
{
    std::vector words_en = {"zero", "one", "two"};
    std::vector words_pl = {"zero", "jeden", "dwa"};

    for (const auto& [index, value] : words_en | std::views::enumerate)
    {
        std::cout << index << " = " << value << "\n";
    }

    std::cout << "\n";

    for (const auto& [en, pl] : std::views::zip(words_en, words_pl))
    {
        std::cout << en << " - " << pl << "\n";
    }
}

template <std::ranges::range TRng>
void print_container(TRng&& container)
{
    for(const auto& item : container)
        std::cout << item << " ";
    std::cout << "\n";
}

// template <std::ranges::view TView>
// void print_container(TView vw)
// {
//     for(const auto& item : vw)
//         std::cout << item << " ";
//     std::cout << "\n";
// }

TEST_CASE("")
{
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    print_container(vec);

    auto evens = vec | std::views::filter([](int i) { return i % 2 == 0; });
    print_container(evens);

    print_container(vec | std::views::take(5));
}
