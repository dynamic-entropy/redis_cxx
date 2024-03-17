// Consumer to consume messages from the queue

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

// 2. If the operation is successful, the consumer acknowledges the message by deleting the message from the list.
//     - This is done with the `RPOP` command.
// 3. If the operation fails, the message is left in the list and the consumer will try to process the message again in
// the next iteration.
auto perform_operation(asio::io_context &ioc, std::shared_ptr<redis::connection> conn, std::string message)
    -> asio::awaitable<void>
{
    std::cout << "Sending notification to topic endpoint for message: " << message << std::endl;
    asio::steady_timer timer(ioc, asio::chrono::seconds(1));
    co_await timer.async_wait(asio::use_awaitable);

    // Randomly fail the operation with 10% probability
    if (std::rand() % 10 == 0)
    {
        std::cout << "Operation failed." << std::endl;
        co_return;
    }

    std::cout << "Operation Acknowledged." << std::endl;

    redis::request req;
    req.push("RPOP", "myqueue:committed");
    co_await conn->async_exec(req, redis::ignore, asio::deferred);
}

auto co_main(redis::config cfg, asio::io_context &ioc) -> asio::awaitable<void>
{

    auto conn = std::make_shared<redis::connection>(co_await asio::this_coro::executor);
    conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

    redis::response<std::vector<std::string>> resp;
    redis::request req;

    try
    {
        for (;;)
        {
            req.clear();
            req.push("LRANGE", "myqueue:committed", -1, -1);
            co_await conn->async_exec(req, resp, asio::deferred);
            std::vector<std::string> messages{std::get<0>(resp).value()};
            if (messages.size() == 0)
            {
                std::cout << "No messages to consume" << std::endl;
                break;
            }

            co_await perform_operation(ioc, conn, messages[0]);
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

        std::cout << "CONSUMER STARTED" << std::endl;

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