cmake_minimum_required(VERSION 3.16)
project(main CXX)

find_package(igraph REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PUBLIC igraph::igraph)
