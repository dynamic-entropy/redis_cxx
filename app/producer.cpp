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

auto co_main(redis::config cfg, asio::io_context &ioc) -> asio::awaitable<void>
{
    auto conn = std::make_shared<redis::connection>(co_await asio::this_coro::executor);
    conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

    redis::response<std::string> resp;

    try
    {
        for (;;)
        {

            // Read a message from the standard input.
            // Exit if empty message.
            std::string message = co_await read_input(ioc);
            if (message.empty())
            {
                break;
            }

            redis::request req;
            req.push("LPUSH", "myqueue", message);

            co_await conn->async_exec(req, resp, asio::deferred);

            std::cout << "QUEUE LENGTH: " << std::get<0>(resp).value() << std::endl;
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

        std::cout << "Enter messages to push to the queue and press Enter." << std::endl;
        std::cout << "Press Enter wihout any message to exit." << std::endl;

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