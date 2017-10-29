//
// Created by dan on 10/25/17.
//

#pragma once


#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <vector>
#include <sys/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>

enum class pixel_format{
    V4L2CXX_PIX_FMT_RGB332 = V4L2_PIX_FMT_RGB332,
    V4L2CXX_PIX_FMT_YVYU = V4L2_PIX_FMT_YVYU
};
namespace util_v4l2_a{

    static int xioctl(int fd, int request, void *arg) {
        int r;
        do r = ioctl(fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
    }


    //V4L2_PIX_FMT_*
    void set_format(int fd , uint32_t width, uint32_t height,pixel_format pixel_format ) {
        struct v4l2_format fmt;
        unsigned int min;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        //std::cout << "set_format()\n";
        fmt.fmt.pix.width = width; //replace
        fmt.fmt.pix.height = height; //replace
        fmt.fmt.pix.pixelformat = static_cast<__u32>(pixel_format); //replace
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            std::cout << "ERROR set_format\n";
        }
        // Note VIDIOC_S_FMT may change width and height.

        // Buggy driver paranoia.
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;

        //printfmt(fmt);

    }
}