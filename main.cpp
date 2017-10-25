#include <iostream>

#include <cstring>
#include "v4l2cxx.h"


int main() {
    //std::cout << "Hello, World!" << std::endl;

    int fd = util_v4l2::open_device("/dev/video1");

    util_v4l2::query_capabilites(fd);

    util_v4l2::query_formats(fd);

    util_v4l2::set_format(fd);

    util_v4l2::init_mmap(fd);

    util_v4l2::start_capturing(fd);

    util_v4l2::mainloop(fd);


    return 0;
}