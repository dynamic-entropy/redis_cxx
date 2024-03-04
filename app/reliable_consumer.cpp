// Executable for redis consumer that pushes to a temporaray queue and acknowledges the message by deleting from the temporary queue

#include <iostream>
#include <vector>
#include <cpp_redis/cpp_redis>

std::string MYQUEUE = "myqueue";
std::string MYTEMPQUEUE = "mytempqueue";

int main()
{
    std::cout << "Hello from consumer" << std::endl;

    cpp_redis::client client;
    client.connect("127.0.0.1", 6379, [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
                   {
        if (status == cpp_redis::client::connect_state::dropped){
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
        } });

    // Get length of the temporary queue and move all its elements to the main queue
    auto rep = client.llen(MYTEMPQUEUE);
    client.sync_commit();
    int len = rep.get().as_integer();
    if (len > 0)
    {
        std::cout << "Found " << len << " unacknowledged messages on startup; moving them to main queue" << std::endl;
        for (int i = 0; i < len; i++)
            client.rpoplpush(MYTEMPQUEUE, MYQUEUE);
        client.sync_commit();
    }

    // Start consuming from the main queue and push to the temporary queue
    // Acknowledge the message by deleting from the temporary queue
    while (true)
    {
        auto rep = client.brpoplpush(MYQUEUE, MYTEMPQUEUE, 0);
        client.sync_commit();
        std::string message = rep.get().as_string();
        std::cout << "Working on message: " << message << std::endl;
        // sleep to simulate work
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // Acknowledge the message by deleting from the temporary queue
        client.lrem(MYTEMPQUEUE, 1, message);
        client.sync_commit();
    }
    return 0;
};