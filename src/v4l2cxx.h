//
// Created by dan on 10/23/17.
//

#pragma once

#include <iostream>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <vector>
#include <sys/mman.h>
#include <cassert>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cerrno>
#include <unistd.h>
#include "util_v4l2.h"
#include "capture.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))


#if 0
void raw_to_rgb(void* inBuff, int inBuffSize, void* outBuff, int outBuffSize, uint32_t numOfPixels, int bitPerPixel)
{
    auto dst = static_cast<uint8_t *>(outBuff);

    auto shift = bitPerPixel - 8;  //i.e. 10bit - 8bit(1 byte) = 2, 12bit - 8bit = 4

    auto tmp = static_cast<uint16_t *>(inBuff);
    for (auto i = 0u; i < numOfPixels; i++)
    {
        uint16_t temp = (*tmp++) >> shift; //12 bit shift 4, 10bit shift 2
        *dst++ = static_cast<uint8_t>(temp);
        *dst++ = static_cast<uint8_t>(temp);
        *dst++ = static_cast<uint8_t>(temp);
    }
}



namespace util_v4l2_b {


    static void process_image(const void *p, int size)
    {
        static uint32_t frame_number = 0;
        frame_number++;

        char filename[15];

        //std::cerr << size << "\n";
//        sprintf(filename, "frame-%d.ppm", frame_number);
//        FILE *fp=fopen(filename,"wb");

        uint8_t outBuff[921600];
        raw_to_rgb(const_cast<void *>(p), 0, outBuff, 921600, 640 * 480, 8);

//        fprintf(fp, "P6\n%d %d 255\n",
//                640, 480);

        bool enable_stdout = true;
        if (enable_stdout){
            //fwrite(outBuff, 921600, 1, fp);
            fwrite(outBuff, 921600, 1, stdout);
        }


//        fflush(fp);
//        fclose(fp);
    }

    static int read_frame(int fd, util_v4l2::buffer *pBuffer) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN:
                    return 0;

                case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                default:
                    printf("ERROR: read_frame VIDIOC_DQBUF\n");
                    return -1;
                break;

            }
        }

        assert(buf.index < 4);

        process_image(pBuffer[buf.index].start, buf.bytesused);

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf))
        {
            printf("ERROR: read_frame VIDIOC_DQBUF\n");
            return -1;
        }

        return 1;
    }

    static void mainloop(int fd, util_v4l2::buffer *pBuffer) {

            while (1){
                for (;;) {
                fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                /* Timeout. */
                tv.tv_sec = 5;
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);

                if (-1 == r) {
                    if (EINTR == errno)
                        continue;

                    printf("ERROR: select");
                    return;
                }

                if (0 == r) {
                    fprintf(stderr, "ERROR: select timeout\n");
                    exit(EXIT_FAILURE);
                }

                if (util_v4l2_b::read_frame(fd, pBuffer))
                    break;

                /* EAGAIN - continue select loop. */
            }
        }
    }

}

#endif