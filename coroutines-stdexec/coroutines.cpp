#include <catch2/catch_test_macros.hpp>
#include <exec/task.hpp>
#include <iostream>
#include <stdexec/execution.hpp>
#include <string>
#include <vector>
#include <future>

using namespace std::literals;

exec::task<int> calculate_square(int n)
{
    std::cout << "Starting calculation for " << n << std::endl;
    std::this_thread::sleep_for(1s);
    std::cout << "Calculation done for " << n << std::endl;

    co_return n * n;
}

TEST_CASE("calculate_square")
{
    auto task = calculate_square(6);

    auto [result] = stdexec::sync_wait(std::move(task)).value();

    CHECK(result == 36);
}

exec::task<std::string> processing_value(int value)
{
    std::cout << "Processing started for " << value << std::endl;
    std::this_thread::sleep_for(1s);
    int square = co_await calculate_square(value);
    std::this_thread::sleep_for(1s);https://prod.liveshare.vsengsaas.visualstudio.com/join?270162BABDA56C12B14B95E958E5BE972B92
    std::cout << "Processing done for " << value << std::endl;
    co_return "Square of " + std::to_string(value) + " is " + std::to_string(square);
}

TEST_CASE("processing_value")
{
    auto task_1 = processing_value(7);
    auto task_2 = processing_value(13);

    SECTION("synchronous wait")
    {
        auto [result] = stdexec::sync_wait(std::move(task_1)).value();
        auto [result_2] = stdexec::sync_wait(std::move(task_2)).value();

        CHECK(result == "Square of 7 is 49");
        CHECK(result_2 == "Square of 13 is 169");
    }

    SECTION("concurrent execution")
    {
        std::cout << "--------------------" << std::endl;
        auto [result, result_2] = stdexec::sync_wait(
            stdexec::when_all(std::move(task_1), std::move(task_2))).value();

        CHECK(result == "Square of 7 is 49");
        CHECK(result_2 == "Square of 13 is 169");
    }
}

//////////////////////////////////////////////////////
// Awaiter for std::future

template <typename T>
class FutureAwaiter
{
    std::future<T> future_;
public:
    FutureAwaiter(std::future<T>&& ftr) : future_(std::move(ftr)) {}

    bool await_ready() const noexcept
    {
        using namespace std::literals;
        return future_.wait_for(0s) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coro_handle)
    {
        std::thread([this, coro_handle]() mutable {
            future_.wait();
            coro_handle.resume();
        }).detach();
    }

    T await_resume()
    {
        return future_.get();
    }
};

template <typename F, typename... Args>
auto async_task(F&& func, Args&&... args) -> FutureAwaiter<decltype(func(std::forward<Args>(args)...))>
{
    using ReturnType = decltype(func(std::forward<Args>(args)...));
    return FutureAwaiter<ReturnType>(std::async(std::launch::async, std::forward<F>(func), std::forward<Args>(args)...));
}

int slow_add(int a, int b)
{
    std::cout << "Starting slow_add(" << a << ", " << b << ") on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(2s);
    return a + b;
}

std::string demo_future(int a, int b)
{
    std::future<int> f = std::async(std::launch::async, slow_add, 1, 2);
    //..
    int result = f.get();
    return "Result is " + std::to_string(result);
}

exec::task<std::string> demo_async_task(int a, int b)
{
    std::cout << "Starting async task on thread " << std::this_thread::get_id() << std::endl;
    int result = co_await async_task(slow_add, a, b);
    std::cout << "slow_add completed with result: " << result << " on thread " << std::this_thread::get_id() << std::endl;
    co_return "Result is " + std::to_string(result);
}

TEST_CASE("demo_async_task")
{
    auto task_1 = demo_async_task(5, 10);
    auto task_2 = demo_async_task(15, 20);
    auto task = stdexec::when_all(std::move(task_1), std::move(task_2));

    auto [result_1, result_2] = stdexec::sync_wait(std::move(task)).value();

    CHECK(result_1 == "Result is 15");
    CHECK(result_2 == "Result is 35");
}