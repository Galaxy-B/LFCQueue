enable_testing()

# link google test library
find_package(GTest REQUIRED)
link_libraries(${GTEST_LIBRARIES} pthread)

include_directories(include)

# test case for SPSC queue
add_executable(spsc_test src/spsc_test.cpp)
add_test(NAME SPSC_basic_test COMMAND spsc_test)
