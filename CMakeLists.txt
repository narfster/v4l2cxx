cmake_minimum_required(VERSION 3.8)
project(libv4l2cxx)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra ")
endif()

set(SOURCE_FILES main.cpp v4l2cxx.h)
add_executable(libv4l2cxx ${SOURCE_FILES})