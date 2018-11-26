cmake_minimum_required(VERSION 3.12)
project(kv_node)

include_directories(include)
set(CMAKE_CXX_STANDARD 11)

add_executable(kv_node main.cpp parse.h parse.cpp message.cpp message.h)