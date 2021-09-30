
FILE(GLOB CppSources *.cc)
add_library(src ${CppSources})
target_include_directories(src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
