
FILE(GLOB CppSources *.cc)
add_library(replacement ${CppSources})
target_include_directories(replacement PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
