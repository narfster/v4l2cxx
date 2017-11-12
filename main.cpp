#include <iostream>

#include <cstring>
#include "v4l2cxx.h"
#include "util_v4l2.h"





int main() {
    //std::cout << "Hello, World!" << std::endl;

    struct util_v4l2::buffer buffers[4];
    static unsigned int n_buffers;



    int fd = util_v4l2::open_device("/dev/video0");

    util_v4l2::query_capabilites(fd);

    util_v4l2::query_formats(fd);

    util_v4l2::set_format(fd,640,480,pixel_format::V4L2CXX_PIX_FMT_YVYU);

    util_v4l2::init_mmap(fd, buffers);

    util_v4l2::start_capturing(fd, 4, buffers);

    util_v4l2_b::mainloop(fd,buffers);


    return 0;
}