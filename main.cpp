#include <iostream>

#include <cstring>
#include "v4l2cxx.h"


int main() {
    //std::cout << "Hello, World!" << std::endl;

    int fd = util_v4l2::open_device("/dev/video0");

    util_v4l2::query_capabilites(fd);

    util_v4l2::query_formats(fd);

    util_v4l2_a::set_format(fd,640,480,pixel_format::V4L2CXX_PIX_FMT_YVYU);

    util_v4l2::init_mmap(fd);

    util_v4l2::start_capturing(fd);

    util_v4l2::mainloop(fd);


    return 0;
}