cmake_minimum_required(VERSION 3.21)

project(CppRedisClients VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.84.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)


add_executable(PingRedis app/ping.cpp)
add_executable(Producer app/producer.cpp)

# add a loop to for each executable
foreach(executable PingRedis Producer)
target_link_libraries(${executable} OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
endforeach()
