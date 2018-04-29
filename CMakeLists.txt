cmake_minimum_required(VERSION 3.5.1)
project(libv4l2cxx)

set(CMAKE_CXX_STANDARD 17)



set(SOURCE_FILES src/main.cpp src/v4l2cxx.h src/util_v4l2.h src/capture.h)
add_executable(libv4l2cxx ${SOURCE_FILES})

set(EXAMPLE1_FILES examples/example1.cpp )
add_executable(example1 ${EXAMPLE1_FILES})