// Executable for redis consumer that blocks and waits for a message to be published to the queue

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

    while (true)
    {
        client.brpop({"myqueue"}, 0, [](cpp_redis::reply &reply)
                     {
            if (reply.is_array())
            {
                std::cout << "Received message: " << reply.as_array().at(1).as_string() << std::endl;
            } });
        client.sync_commit();
    }

    return 0;
}