include_directories(include)

# enable debug
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-g)
endif()

# test case for SPSC queue
add_executable(spsc_test src/spsc_test.cpp)
