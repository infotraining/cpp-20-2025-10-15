#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

using namespace std::literals;

struct Point
{
    int x;
    int y;

    friend std::ostream& operator<<(std::ostream& out, const Point& p)
    {
        return out << std::format("Point({},{})", p.x, p.y);
    }

    bool operator==(const Point& other) const = default;
    // friend bool operator==(const Point& lhs, const Point& rhs) = default;

    bool operator==(const std::pair<int, int>& other) const
    {
        return x == other.first && y == other.second;
    }
};

struct Point3D : Point
{
    int z;

    constexpr Point3D(int x, int y, int z)
        : Point{x, y}
        , z{z}
    { }

    bool operator==(const Point3D& other) const = default;
};

TEST_CASE("Point - operator ==")
{
    SECTION("Point")
    {
        Point p1{1, 2};
        Point p2{1, 2};
        Point p3{2, 1};

        CHECK(p1 == p2);
        CHECK(p1 != p3); // expression rewritten to: !(p1 == p2)

        std::pair pt1{1, 2};
        CHECK(p1 == pt1);

        CHECK(pt1 == p1); // expression rewritten to: p1 == pt1
    }

    SECTION("Point3D")
    {
        constexpr Point3D p1{1, 2, 3};
        constexpr Point3D p2{1, 2, 3};
        Point3D p3{1, 2, 4};

        CHECK(p1 == p2);
        CHECK(p1 != p3);

        static_assert(p1 == p2);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{
    struct Money
    {
        int dollars;
        int cents;

        constexpr Money(int dollars, int cents)
            : dollars(dollars)
            , cents(cents)
        {
            if (cents < 0 || cents > 99)
            {
                throw std::invalid_argument("cents must be between 0 and 99");
            }
        }

        constexpr Money(double amount)
            : dollars(static_cast<int>(amount))
            , cents(static_cast<int>(amount * 100) % 100)
        { }

        friend std::ostream& operator<<(std::ostream& out, const Money& m)
        {
            return out << std::format("${}.{}", m.dollars, m.cents);
        }

        auto operator<=>(const Money& other) const = default;
    };

    namespace Literals
    {
        // clang-format off
        constexpr Money operator""_USD(long double amount)
        {
            return Money(amount);
        }
        // clang-format on
    } // namespace Literals
} // namespace Comparisons

TEST_CASE("Money - operator <=>")
{
    using Comparisons::Money;
    using namespace Comparisons::Literals;

    Money m1{42, 50};
    Money m2{42, 50};

    SECTION("comparison operators are synthetized")
    {
        CHECK(m1 == m2);
        CHECK(m1 == Money(42.50));
        CHECK(m1 == 42.50_USD);
        CHECK(m1 != 42.51_USD);
        CHECK(m1 < 42.51_USD);
        CHECK(m1 <= 42.51_USD);
        CHECK(m1 > 0.99_USD);
        CHECK(m1 >= 0.99_USD);

        SECTION("it should never appear in clients code")
        {
            CHECK((m1 <=> m2 == 0));
        }

        static_assert(Money{42, 50} == 42.50_USD);
    }

    SECTION("sorting")
    {
        std::vector<Money> wallet{42.50_USD, 13.37_USD, 0.99_USD, 100.00_USD, 0.01_USD};
        std::ranges::sort(wallet);
        CHECK(std::ranges::is_sorted(wallet));
    }
}

struct Temperature
{
    double value;

    bool operator==(const Temperature& other) const
    {
        return std::strong_order(value, other.value) == 0;
    }

    auto operator<=>(const Temperature& other) const
    {
        return std::strong_order(value, other.value);
    }
};

TEST_CASE("operator <=>")
{
    SECTION("primitive types")
    {
        int x = 42;
    }

    SECTION("custom types")
    {
        Temperature t1{23.5};
        Temperature t2{23.5};
        Temperature t3{24.5};
        Temperature t4{std::numeric_limits<double>::quiet_NaN()};

        SECTION("result is a comparison category")
        {
            CHECK(t1 == t2);
            auto result_1 = t1 <=> t2;
            static_assert(std::is_same_v<decltype(result_1), std::strong_ordering>);
        }

        SECTION("operators <, >, <=, >= are synthetized")
        {
            CHECK(t4 == t4);
            CHECK(t1 < t3);
            CHECK(t1 < t4);
        }
    }
}

struct PreCpp20
{
    int value;

    bool operator==(const PreCpp20& other) const
    {
        return value == other.value;
    }

    bool operator<(const PreCpp20& other) const
    {
        return value < other.value;
    }
};

struct PostCpp20
{
    int id;
    PreCpp20 member;

    std::strong_ordering operator<=>(const PostCpp20& other) const = default;
};

struct DifferentOrderOfMembers
{
    std::string name;
    int id;

    auto tied() const
    {
        return std::tie(id, name);
    }

    bool operator==(const DifferentOrderOfMembers& other) const
    {
        return tied() == other.tied();
    }

    auto operator<=>(const DifferentOrderOfMembers& other) const 
    {
        // if (auto cmp_id = id <=> other.id; cmp_id != 0)
        //     return cmp_id;
        // return name <=> other.name;

        return tied() <=> other.tied();
    }
};

TEST_CASE("working with legacy code")
{
    PostCpp20 pc1{42, PreCpp20{665}};
    PostCpp20 pc2{42, PreCpp20{42}};

    CHECK(pc1 == pc1);
    CHECK(pc1 != pc2);
    CHECK(pc1 > pc2);
    CHECK(pc1 >= pc2);
    CHECK(pc2 < pc1);
    CHECK(pc2 <= pc1);
}

class Car
{
    std::string licence_plate_;
    int milage_;
public:
    Car(const std::string& licence_plate, int milage) : licence_plate_{licence_plate}, milage_{milage}
    {}

    std::string get_licence_plate() const
    {
        return licence_plate_;
    }

    int get_milage()
    {
        return milage_;
    }

    void drive(int distance)
    {
        milage_ += distance;
    }

    std::weak_ordering operator<=>(const Car& other) const
    {
        return licence_plate_ <=> other.licence_plate_;
    }

    bool operator==(const Car& other) const
    {
        return licence_plate_ == other.licence_plate_;
    }
};

TEST_CASE("comparison categories")
{
    SECTION("strong ordering")
    {
        auto result = 42 <=> 665;
        static_assert(std::is_same_v<decltype(result), std::strong_ordering>);
        CHECK(result == std::strong_ordering::less);

        int x = -42;
        unsigned y = 665;
        // CHECK((x <=> y < 0));
    }

    SECTION("partial ordering")
    {
        double pi = 3.14;
        auto result = pi <=> std::numbers::e;
        static_assert(std::is_same_v<decltype(result), std::partial_ordering>);
        CHECK(result == std::partial_ordering::greater);

        auto result_with_nan = pi <=> std::numeric_limits<double>::quiet_NaN();
        CHECK(result_with_nan == std::partial_ordering::unordered);
    }

    SECTION("weak ordering")
    {
        Car c1{"KR1111", 2'000};
        Car c2{"KR1111", 4'000};

        CHECK((c1 <=> c2 == 0));
        CHECK((c1.get_milage() <=> c2.get_milage() != 0));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{
    class Data
    {
        int* buffer_;
        size_t size_;

    public:
        Data(std::initializer_list<int> values)
            : buffer_(new int[values.size()])
            , size_(values.size())
        {
            std::copy(values.begin(), values.end(), buffer_);
        }

        Data(const Data&) = delete;
        Data& operator=(const Data&) = delete;

        ~Data()
        {
            delete[] buffer_;
        }

        auto operator<=>(const Data& other) const
        {
            return std::lexicographical_compare_three_way(
                buffer_, buffer_ + size_, 
                other.buffer_, other.buffer_ + other.size_
            );
        }

        bool operator==(const Data& other) const
        {
            if (size_ != other.size_)
                return false;

            return std::equal(buffer_, buffer_ + size_, other.buffer_);
        }
    };
} // namespace Comparisons

TEST_CASE("lexicographical_compare_three_way")
{
    using Comparisons::Data;

    Data data1{1, 2, 3};
    Data data2{1, 2, 3};
    Data data3{1, 2, 4};
}