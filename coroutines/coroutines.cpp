#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

class TaskResumer
{
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        auto initial_suspend()
        {
            std::cout << "Initial suspension point..." << std::endl;
            return std::suspend_always{};
        }

        auto final_suspend() noexcept
        {
            std::cout << "Final suspension point..." << std::endl;
            return std::suspend_always{};
        }

        void unhandled_exception()
        {
            std::terminate();
        }

        TaskResumer get_return_object()
        {
            return CoroHandle::from_promise(*this);
        }

        void return_void()
        {}
    };

    TaskResumer(CoroHandle coro_hndl)
        : coro_hndl_{coro_hndl}
    { }

    TaskResumer(const TaskResumer&) = delete;
    TaskResumer& operator=(const TaskResumer&) = delete;

    ~TaskResumer()
    {
        if (coro_hndl_)
            coro_hndl_.destroy();
    }

    bool resume() const
    {
        if (!coro_hndl_ || coro_hndl_.done())
            return false;

        coro_hndl_.resume();

        return !coro_hndl_.done();
    }

private:
    CoroHandle coro_hndl_;
};

TaskResumer simple_coroutine()
{
    int i = 0;

    std::cout << "Simplest_coroutine_started..." << std::endl;
    ++i;

    co_await std::suspend_always();

    std::cout << "Step: " << i << std::endl;
    ++i;

    co_await std::suspend_always();

    std::cout << "Step: " << i << std::endl;

    co_await std::suspend_always();

    std::cout << "End of simplest_coroutine\n";
}

TEST_CASE("first coroutine")
{
    TaskResumer task = simple_coroutine();

    while (task.resume())
        std::cout << "Caller!!!" << std::endl;
}