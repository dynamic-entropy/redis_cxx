cmake_minimum_required(VERSION 3.21)

project(CppRedisClients VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "cmake")
include(AddGitSubmodule)

add_git_submodule(external/cpp_redis)

add_executable(Producer app/producer.cpp)
add_executable(Consumer app/consumer.cpp)
add_executable(BlockingConsumer app/blocking_consumer.cpp)

# add a loop to for each executable
foreach(executable Producer Consumer BlockingConsumer)
    target_link_libraries(${executable} cpp_redis)
    target_include_directories(${executable} SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/external/cpp_redis/includes")
    target_include_directories(${executable} SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/external/cpp_redis/tacopie/includes")
endforeach()
