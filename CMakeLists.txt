cmake_minimum_required(VERSION 3.22)

project(CppRedisClients VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(Producer app/producer.cpp)
add_executable(Consumer app/consumer.cpp)