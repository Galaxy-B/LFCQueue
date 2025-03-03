cmake_minimum_required(VERSION 3.15)

project(lfcq LANGUAGES CXX)

# require C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# generate json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(GNUInstallDirs)

# enable debug
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message("*** current build type is DEBUG ***")
    add_compile_options(-g)
else()
    message("*** current build type is RELEASE ***")
    add_compile_definitions(NDEBUG)
endif()

# install header files
install(DIRECTORY lfcq/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# build test case
if(BUILD_TESTING)
    message(STATUS "*** enable test building ***")
    include_directories(lfcq)
    add_subdirectory(test)
endif()
