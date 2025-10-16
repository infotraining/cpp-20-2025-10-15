#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <numbers>

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