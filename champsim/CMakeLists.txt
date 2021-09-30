
cmake_minimum_required(VERSION 3.16)

project(champsim CXX) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(inc)

add_subdirectory(src)
add_subdirectory(branch)
add_subdirectory(replacement)
add_subdirectory(prefetcher)

add_executable(champsim src/main.cc)
target_link_libraries(champsim src branch replacement prefetcher)
