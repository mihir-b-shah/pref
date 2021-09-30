
FILE(GLOB CppSources *.cc)
add_library(prefetcher ${CppSources})
target_include_directories(prefetcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
