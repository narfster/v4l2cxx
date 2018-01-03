cmake_minimum_required(VERSION 3.8)
project(libv4l2cxx)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra ")
endif()

set(SOURCE_FILES src/main.cpp src/v4l2cxx.h src/util_v4l2.h)
add_executable(libv4l2cxx ${SOURCE_FILES})