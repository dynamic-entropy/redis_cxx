// Executable for redis producer

#include <iostream>
#include <sstream>
#include <cpp_redis/cpp_redis>

int main()
{
    std::cout << "Hello from producer" << std::endl;

    cpp_redis::client client;
    client.connect("127.0.0.1", 6379, [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
                   {
        if (status == cpp_redis::client::connect_state::dropped)
        {
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
        } });


    while (true)
    {
        std::string input;
        std::vector<std::string> values;
        
        std::string value;

        std::cout << "Enter space separated values; press enter to submit; and use Ctrl+C to exit: " << std::endl;
        std::getline(std::cin, input);
        std::istringstream iss(input);
        while (iss >> value)
        {
            values.push_back(value);
        }
        client.lpush("myqueue", values, [](cpp_redis::reply &reply)
                     { std::cout << "lpush: " << reply << std::endl; });
        client.sync_commit();
    }
    return 0;
}