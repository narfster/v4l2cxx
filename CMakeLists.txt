cmake_minimum_required(VERSION 3.5.1)
project(libv4l2cxx)

set(CMAKE_CXX_STANDARD 17)


set(HEADER_FILES src/util_v4l2.h src/v4l2cxx.h)

set(SOURCE_FILES examples/main.cpp )
add_executable(libv4l2cxx ${SOURCE_FILES} ${HEADER_FILES})

set(EXAMPLE1_FILES examples/example1.cpp )
add_executable(example1 ${EXAMPLE1_FILES} ${HEADER_FILES})

