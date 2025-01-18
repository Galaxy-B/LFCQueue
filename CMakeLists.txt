cmake_minimum_required(VERSION 3.15)

project(lfcq LANGUAGES CXX)

# require C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(GNUInstallDirs)

# install header files
install(DIRECTORY lfcq/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# build test case
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
