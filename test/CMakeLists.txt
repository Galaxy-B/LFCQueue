enable_testing()

# link google test library
find_package(GTest REQUIRED)
link_libraries(${GTEST_LIBRARIES} pthread)

include_directories(include)

# enable debug
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message("*** current build type is DEBUG ***")
    add_compile_options(-g)
else()
    message("*** current build type is RELEASE ***")
    add_compile_definitions(NDEBUG)
endif()

# test case for SPSC queue
add_executable(spsc_test src/spsc_test.cpp)
add_test(NAME SPSC_basic_test COMMAND spsc_test)
