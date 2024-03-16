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

auto co_main(redis::config cfg, asio::io_context &ioc) -> asio::awaitable<void>
{

    auto conn = std::make_shared<redis::connection>(co_await asio::this_coro::executor);
    conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

    redis::response<std::vector<std::string>> resp;

    try
    {
        for (;;)
        {
            redis::request req;
            req.push("BRPOP", "myqueue", 0);

            co_await conn->async_exec(req, resp, asio::deferred);

            std::cout << std::get<0>(resp).value()[1] << std::endl;
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

        cfg.health_check_interval = std::chrono::seconds::zero();

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