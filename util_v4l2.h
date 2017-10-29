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

#define UTIL_CLEAR(x) memset(&(x), 0, sizeof(x))

enum class pixel_format{
    V4L2CXX_PIX_FMT_RGB332 = V4L2_PIX_FMT_RGB332,
    V4L2CXX_PIX_FMT_YVYU = V4L2_PIX_FMT_YVYU
};
namespace util_v4l2{

    static int xioctl(int fd, int request, void *arg) {
        int r;
        do r = ioctl(fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
    }

    int open_device(std::string device_node) {

        int fd;

        fd = open(device_node.c_str(), O_RDWR);
        if (fd == -1) {
            // couldn't find capture device
            perror("Opening Video device");
            return -1;
        }

        return fd;

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

    int query_capabilites(int fd) {

        struct v4l2_capability caps;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            perror("Querying Capabilites");
            return 1;
        }

//        printf("cap 0x %x \n", caps.capabilities);
//
//        auto tmp = caps.capabilities;
//
//        if (caps.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
//            std::cout << "V4L2_CAP_VIDEO_CAPTURE" << std::endl;
//        }
//        if (caps.device_caps & V4L2_CAP_VIDEO_OUTPUT) {
//            std::cout << "V4L2_CAP_VIDEO_OUTPUT" << std::endl;
//        }
//        if (caps.device_caps & V4L2_CAP_STREAMING) {
//            std::cout << "V4L2_CAP_STREAMING" << std::endl;
//        }

        return 0;
    }

    std::vector<v4l2_fmtdesc> query_formats(int fd) {

        std::vector<v4l2_fmtdesc> formats;
        struct v4l2_fmtdesc fmt;
        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (-1 != util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {

            struct v4l2_fmtdesc fmt2;
            fmt2.index = 0;
            fmt2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt2);
            formats.push_back(fmt2);
            fmt.index++;
        }
        return formats;
    }

    static void start_capturing(int fd, int numOfBuffers) {
        unsigned int i;
        enum v4l2_buf_type type;


        for (i = 0; i < numOfBuffers; ++i) {
            struct v4l2_buffer buf;

            UTIL_CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf)) {
                printf("ERROR: start_capturing VIDIOC_QBUF");
            }

        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_STREAMON, &type))
            printf("ERROR: start_capturing VIDIOC_STREAMON");

    }



}// namespace