cmake_minimum_required(VERSION 3.10)
project(epoll_echo)

set(CMAKE_CXX_STANDARD)

add_executable(server Server.cpp Main.cpp IO.cpp)

find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
