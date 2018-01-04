//
// Created by dan on 10/23/17.
//

#pragma once


#include "util_v4l2.h"


namespace v4l2cxx {


    struct v4l2_frm_size_t {
        // Original frame size enum
        v4l2_frmsizeenum frmsize;

        // holds possiable frame intervals
        struct std::vector<v4l2_frmivalenum> frmival;
    };

    struct vid_cap_fmt_ext {

        vid_cap_fmt_ext(v4l2_fmtdesc fmt) : v4l2_fmt_desc_(fmt) {
            // convert to string repesnation.
            description = std::string((char *) fmt.description);
            fourcc_format = util_v4l2::fcc2s(fmt.pixelformat);
            if (fmt.flags) {
                flags = util_v4l2::fmtdesc2s(fmt.flags);
            }
            type = util_v4l2::buftype2s(fmt.type);

            format_index = fmt.index;
        }

        // Original v4l2 format description.
        struct v4l2_fmtdesc v4l2_fmt_desc_;

        // possiable sizes for format.
        std::vector<v4l2_frm_size_t> v4l2_frm_sizes_;

        int format_index;
        std::string description;
        std::string fourcc_format;
        std::string flags;
        std::string type;
    };

    static void print_fmt_ext(std::vector<vid_cap_fmt_ext> vid_formats){

        // Run on all format descriptions.
        for(int i = 0; i<vid_formats.size(); ++i){

            util_v4l2::print_fmt_desc(vid_formats[i].v4l2_fmt_desc_);

            // Run on all sizes
            for(int j = 0; j<vid_formats[i].v4l2_frm_sizes_.size(); ++j){

                auto e = vid_formats[i].v4l2_frm_sizes_[j].frmsize;
                util_v4l2::print_frmsize(e, "\t");

                // Run on all intervals
                for(int k = 0; k< vid_formats[i].v4l2_frm_sizes_[j].frmival.size();++k){

                    auto e = vid_formats[i].v4l2_frm_sizes_[j].frmival[k];
                    util_v4l2::print_frmival(e, "\t\t");
                }
            }
        }
    }



    static void get_video_formats_ext(int fd) {
        struct v4l2_fmtdesc fmt;
        struct v4l2_frmsizeenum frmsize;
        struct v4l2_frmivalenum frmival;

        std::vector<vid_cap_fmt_ext> vid_formats;   // results stored here.

        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while (util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {

            vid_cap_fmt_ext current_fmt(fmt);
            // Save format description
            //current_fmt.fmt = fmt;

            frmsize.pixel_format = fmt.pixelformat;
            frmsize.index = 0;
            while (util_v4l2::xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {

                v4l2_frm_size_t vid_cap_frm_size;
                vid_cap_frm_size.frmsize = frmsize;

                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                    frmival.index = 0;
                    frmival.pixel_format = fmt.pixelformat;
                    frmival.width = frmsize.discrete.width;
                    frmival.height = frmsize.discrete.height;
                    while (util_v4l2::xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0) {

                        // Save frame interval data
                        vid_cap_frm_size.frmival.push_back(frmival);
                        frmival.index++;
                    }
                }
                current_fmt.v4l2_frm_sizes_.push_back(vid_cap_frm_size);
                frmsize.index++;
            }
            // Save current video format
            vid_formats.push_back(current_fmt);
            fmt.index++;
        }

        //test print
        print_fmt_ext(vid_formats);
    }

}










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