#include <boost/asio/co_spawn.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/redis/config.hpp>
#include <boost/redis/connection.hpp>
#include <boost/redis/src.hpp>
#include <iostream>

namespace asio = boost::asio;
namespace redis = boost::redis;

#if defined(BOOST_ASIO_HAS_CO_AWAIT)

auto read_input(asio::io_context &ioc) -> asio::awaitable<std::string>
{
    asio::posix::stream_descriptor input(ioc, ::dup(STDIN_FILENO));
    std::string message;

    for (;;)
    {
        std::array<char, 128> buffer;
        std::size_t n = co_await input.async_read_some(asio::buffer(buffer), asio::use_awaitable);

        for (std::size_t i = 0; i < n; ++i)
        {
            if (buffer[i] == '\n')
            {
                co_return message;
            }
            else
            {
                message += buffer[i];
            }
        }
    }
}

auto auto_input(asio::io_context &ioc) -> asio::awaitable<std::string>
{
    asio::steady_timer timer(ioc, asio::chrono::seconds(1));
    co_await timer.async_wait(asio::use_awaitable);
    if (std::rand() % 2)
    {
        co_return "abort:" + std::to_string(std::rand());
    }

    co_return std::to_string(std::rand());
}

// Perform work on the message - wait for 2 seconds to simulate work
// If the message starts with abort: return false (to abort) and true (to commit)
auto perform_operation(asio::io_context &ioc, std::shared_ptr<redis::connection> conn, std::string message)
    -> asio::awaitable<void>
{
    std::cout << "Performing operation on message: " << message << std::endl;

    int timeToWait = 3;
    asio::steady_timer timer(ioc, asio::chrono::seconds(timeToWait));
    co_await timer.async_wait(asio::use_awaitable);
    std::cout << "Operation completed on message: " << message << std::endl;
    bool decision = true;
    if (message.starts_with("abort:"))
    {
        decision = false;
    }
    redis::request req;
    if (!decision)
    {
        std::cout << "Aborting message: " << message << std::endl;
        req.push("RPOP", "myqueue:pending");
        co_await conn->async_exec(req, redis::ignore, asio::deferred);
    }
    else
    {
        std::cout << "Committing message: " << message << std::endl;
        req.push("LMOVE", "myqueue:pending", "myqueue:committed", "RIGHT", "LEFT");
        co_await conn->async_exec(req, redis::ignore, asio::deferred);
    }
}

auto co_main(redis::config cfg, asio::io_context &ioc) -> asio::awaitable<void>
{
    auto conn = std::make_shared<redis::connection>(co_await asio::this_coro::executor);
    conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

    redis::response<std::string> resp;
    redis::request req;

    // This ensures that previously unacknowledged messages are deleted
    try
    {
        req.push("DEL", "myqueue:pending");
        co_await conn->async_exec(req, redis::ignore, asio::deferred);
        req.clear();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        conn->cancel();
    }

    try
    {
        for (;;)
        {

            // Get a message from auto_input every second
            std::string message = co_await auto_input(ioc);
            if (message.empty())
            {
                break;
            }

            req.clear();
            req.push("LPUSH", "myqueue:pending", message);

            std::cout << "PUSHING MESSAGE: " << message << std::endl;
            co_await conn->async_exec(req, resp, asio::deferred);
            std::cout << "QUEUE LENGTH: " << std::get<0>(resp).value() << std::endl;

            asio::co_spawn(ioc, perform_operation(ioc, conn, message), asio::detached);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        conn->cancel();
    }
}

int main(int argc, char *argv[])
{
    try
    {
        redis::config cfg;

        asio::io_context ioc;

        std::cout << "PRODUCER STARTED" << std::endl;

        // Start the main coroutine.
        // Stop the io_context when the coroutine exits.
        asio::co_spawn(ioc, co_main(cfg, ioc), [&ioc](std::exception_ptr p) { ioc.stop(); });
        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

#endif // defined(BOOST_ASIO_HAS_CO_AWAIT)