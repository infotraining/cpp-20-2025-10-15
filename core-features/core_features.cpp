#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <source_location>

using namespace std::literals;

std::optional<std::vector<int>> load_data()
{
    return std::vector{1, 2, 3}; // return prvalue - RVO
}

enum class DownloadStatus {
    not_started,
    pending,
    done
};

[[nodiscard("never ignore error code")]] std::errc save_to_file(const std::string& path)
{
    return std::errc::bad_file_descriptor;
}

TEST_CASE("core features")
{
    SECTION("range-based for with initializer")
    {
        for (auto data = load_data(); int i : data.value())
        {
            std::cout << i << "\n";
        }
    }

    SECTION("using + enums")
    {
        DownloadStatus ds = DownloadStatus::done;
        ds = DownloadStatus::pending;

        switch (ds)
        {
            using enum DownloadStatus;
        case done:
            std::cout << "done\n";
            break;
        case pending:
            std::cout << "done\n";
            break;
        case not_started:
            std::cout << "not-started\n";
            break;
        }
    }

    SECTION("no_discard")
    {
        std::errc error_code = save_to_file("text.txt");
    }
}

struct Empty
{ };

struct Value
{
    int i;
};

struct EmptyAndValue
{
    [[no_unique_address]] Empty e;
    Value v;
};

TEST_CASE("no_unique_address")
{
    static_assert(sizeof(Empty) == 1);
    static_assert(sizeof(Value) == 4);
    //static_assert(sizeof(EmptyAndValue) == 4);
}

struct Data
{
    int id;
    double value;
    std::string name;

    void print() const
    {
        std::cout << "Data{" << id << ", " << value << ", " << name << "}\n";
    }
};

template <typename T1, typename T2>
struct Pair
{
    T1 fst;
    T2 snd;
};

// deduction guide
// template <typename T1, typename T2>
// Pair(T1, T2) -> Pair<T1, T2>;


TEST_CASE("aggregates")
{
    Data d1{42, 3.14, "pi"};
    Data d2{.id=665, .name = "zero"};
    // Data d3{.name = "zero", .id=1}; // Error
    // Data d3{999, .name = "zero"}; // Error

    Data d4(88, 3.4544, "float");   
    // Data d5(.id=1); // Error

    d4.print();

    auto ptr = std::make_unique<Data>(7, 77.7, "seven"); // OK since C++20
    ptr->print();

    Pair p1{42, "text"s}; // CTAD
    Pair p2(77, "seven"); // CTAD
}

template <typename T>
void foo_location(T value, std::source_location sl = std::source_location::current())
{ 
    std::cout << "file: " << sl.file_name() << "\n";
    std::cout << "function: " << sl.function_name() << "\n";
    std::cout << "line/col: " << sl.line() << "\n";
}

TEST_CASE("source_location")
{
    foo_location(42);
}