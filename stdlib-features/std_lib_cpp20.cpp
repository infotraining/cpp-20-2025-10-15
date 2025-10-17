#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <numbers>
#include <format>

using namespace std::literals;

void print(std::span<const int> data, std::string_view desc)
{
    std::cout << desc << ": [ ";
    for (const auto& item : data)
        std::cout << item << " ";
    std::cout << "]\n";
}

void zero(std::span<int> data, int default_value = 0)
{
    for (auto& item : data)
        item = default_value;
}

std::span<int> get_head(std::span<int> data)
{
    return data.subspan(0, 3);
}

void use(int* tab, size_t size)
{
    delete[] tab;
}

TEST_CASE("std::span")
{
    SECTION("fixed extent")
    {
        int native_array[5] = {1, 2, 3, 4, 5};
        print(native_array, "native_array");

        std::span sp_1{native_array}; // CTAD -> std::span<int, 5>
        print(sp_1, "sp_1");

        std::array<int, 10> arr = {1, 2, 3};
        std::span sp_2{arr}; // CTAD -> std::span<int, 10>

        std::span<int, 3> sp_head{arr.data(), 3};
        print(sp_head, "sp_head");
    }

    SECTION("dynamic extent")
    {
        std::span<int> sp_1;
        CHECK(sp_1.data() == nullptr);
        CHECK(sp_1.size() == 0);

        std::vector<int> vec = {1, 2, 3, 4, 5, 6};
        sp_1 = vec;
        print(sp_1, "sp_1");

        sp_1 = std::span{vec.data(), 3};
        print(sp_1, "sp_1");

        zero(std::span{vec.data(), 4});
        sp_1[2] = 42;
        print(vec, "vec");
    }

    SECTION("beware dangling pointers!!!")
    {
        SECTION("case 1")
        {
            std::vector<int> vec = {1, 2, 3, 4, 5};
            std::span<int> head_of_vec = get_head(vec);

            vec.push_back(6); // invalidating the span

            print(head_of_vec, "head of vec");
        }

        SECTION("case 2")
        {
            int* tab = new int[1024];
            use(tab, 1024);
            std::span<int> head_of_tab = get_head(std::span{tab, 3});
            print(head_of_tab, "head of tab");
        }
    }
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
#ifdef __cpp_lib_format
    std::cout << std::format("{:+6}", f) << " - { ";

    for (std::byte b : bytes)
    {
        std::cout << std::format("{:02X} ", std::to_integer<int>(b));
    }

    std::cout << "}\n";
#endif
}

TEST_CASE("float as span of bytes")
{
    float data[] = {std::numbers::pi_v<float>};

    std::span<const std::byte> const_bytes = std::as_bytes(std::span{data});
    print_as_bytes(data[0], const_bytes);

    std::span<std::byte> writeble_bytes = std::as_writable_bytes(std::span{data});
    writeble_bytes[3] |= std::byte{0b1000'0000};
    print_as_bytes(data[0], const_bytes);
}

//////////////////////////////////////
// formatting with std::format

TEST_CASE("format")
{
    std::string text = "Text";
    std::string formatted_str = std::format("{} has {} chars", text, text.size());
    std::cout << formatted_str << "\n";

    std::cout << std::format("Price:{:_>8.2f} PLN\n", 665.9);
}

TEST_CASE("format specifiers")
{
    SECTION("width & fill character")
    {
        // width & alignment
        CHECK(std::format("{:8}", 42) == "      42"sv);
        CHECK(std::format("{:<8}", 42) == "42      "sv);
        CHECK(std::format("{:8}", "text") == "text    "sv);
        CHECK(std::format("{:^8}", "text") == "  text  "sv);
        CHECK(std::format("{:>8}", "text") == "    text"sv);

        // padding zeros
        CHECK(std::format("{:08}", 42) == "00000042"sv);  // padding zeros
        CHECK(std::format("{:^08}", 42) == "   42   "sv); // padding zeros ignored when alignment is specified
        CHECK(std::format("{:>08}", -42) == "     -42"sv);

        // fill character
        CHECK(std::format("{:0<8}", 42) == "42000000"sv);
        CHECK(std::format("{:0^8}", 42) == "00042000"sv);
        CHECK(std::format("{:0>8}", 42) == "00000042"sv);
    }

    SECTION("precision for floating-points")
    {
        CHECK(std::format("{}", 0.12345678) == "0.12345678"sv); // default precision: 6
        CHECK(std::format("{:.5}", 0.12345678) == "0.12346"sv);
        CHECK(std::format("{:10.5}", 0.12345678) == "   0.12346"sv);
        CHECK(std::format("{:_^10.5}", 0.12345678) == "_0.12346__"sv);
    }

    SECTION("precision for strings")
    {
        CHECK(std::format("{}", "helloworld") == "helloworld"sv);
        CHECK(std::format("{:15}", "helloworld") == "helloworld     "sv);
        CHECK(std::format("{:.5}", "helloworld") == "hello"sv);
        CHECK(std::format("{:15.5}", "helloworld") == "hello          "sv);
    }
}

TEST_CASE("formatting to buffers")
{
    std::string txt = "Hello";

    SECTION("add null at the end")
    {
        char buffer[128];

        auto result = std::format_to_n(buffer, std::size(buffer) - 1, "String '{}' has {} chars\n", txt, txt.size());
        *(result.out) = '\0'; // add null at the end

        std::cout << buffer;
    }

    SECTION("use array that is zeroed")
    {
        std::array<char, 128> buffer{};

        auto result = std::format_to_n(buffer.data(), buffer.size() - 1, "String '{}' has {} chars\n", txt, txt.size());

        std::cout << buffer.data();
    }
}

TEST_CASE("std::format_to - unlimited number of chars")
{
    SECTION("works with streams")
    {
        std::format_to(std::ostreambuf_iterator<char>{std::cout}, "{} has value {}\n", "Pi", std::numbers::pi);
    }

    SECTION("works with back inserters")
    {
        std::string str;
        std::format_to(std::back_inserter(str), "{} has value {}\n", "Pi", std::numbers::pi);

        std::cout << str;
    }
}

TEST_CASE("format string must be compile-time value")
{
    SECTION("for std::format")
    {
        const char* fmt1 = "{}\n"; // runtime format string
        // std::cout << std::format(fmt1, 42); // compile-time ERROR: runtime format

        constexpr const char* fmt2 = "{}\n"; // compile-time format string
        std::cout << std::format(fmt2, 42);  // OK
    }

    SECTION("runtime version - std::vformat")
    {
        const char* fmt = "{} value is {:7.2f}\n";
        std::cout << std::vformat(fmt, std::make_format_args("Pi", std::numbers::pi));
    }
}

