// Executable for redis consumer

#include <iostream>
#include <cpp_redis/cpp_redis>

int main()
{
    std::cout << "Hello from consumer" << std::endl;

    cpp_redis::client client;
    client.connect("127.0.0.1", 6379, [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
                   {
        if (status == cpp_redis::client::connect_state::dropped)
        {
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
        } });

    // Continously poll and consumer the queue
    while (true)
    {
        client.rpop("myqueue", [](cpp_redis::reply &reply)
                    { std::cout << "rpop: " << reply << std::endl; });
        client.sync_commit();

        // sleep for 5 second
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}