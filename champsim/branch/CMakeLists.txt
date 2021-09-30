
FILE(GLOB CppSources *.cc)
add_library(branch ${CppSources})
target_include_directories(branch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
