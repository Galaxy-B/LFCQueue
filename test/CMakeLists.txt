enable_testing()

# link google test library
find_package(GTest REQUIRED)
link_libraries(${GTEST_LIBRARIES} pthread)

include_directories(include)

# test case for basic features
add_executable(basic_test src/basic_test.cpp)
add_test(NAME LFCQ_basic_test COMMAND basic_test)

# test case for SPSC queue
add_executable(spsc_test src/spsc_test.cpp)
add_test(NAME SPSC_basic_test COMMAND spsc_test)
