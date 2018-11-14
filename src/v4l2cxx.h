
/*
MIT License

Copyright (c) 2018 Dan Yerushalmi dan.yerushalmi@gmail.com
https://github.com/narfster/v4l2cxx/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once


#include "util_v4l2.h"


namespace v4l2cxx {


    struct frm_size_t {
        frm_size_t(v4l2_frmsizeenum frmsize):
                frmsize_(frmsize) {
            if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                frmsize_discrete_ = frmsize.discrete;
            }
            else if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE){
                frmsize_stepwise_ = frmsize.stepwise;
            }
            frame_size_num = frmsize.index;
            type_= util_v4l2::frmtype2s(frmsize_.type);

            fourcc_format_ = util_v4l2::fcc2s(frmsize_.pixel_format);
        }

        // Original frame size enum data
        v4l2_frmsizeenum frmsize_;

        std::string type_;
        std::string fourcc_format_;
        int frame_size_num;
        v4l2_frmsize_discrete frmsize_discrete_;
        v4l2_frmsize_stepwise frmsize_stepwise_;

        // holds possiable frame intervals
        struct std::vector<v4l2_frmivalenum> frmival;
    };

    struct fmt_ext_t {

        fmt_ext_t(v4l2_fmtdesc fmt) :
                v4l2_fmt_desc_(fmt) {
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
        std::vector<frm_size_t> v4l2_frm_sizes_;

        int format_index;
        std::string description;
        std::string fourcc_format;
        std::string flags;
        std::string type;
    };

    static void print_fmt_ext(std::vector<fmt_ext_t> vid_formats){

        // Run on all format descriptions.
        for(int i = 0; i<vid_formats.size(); ++i){

            util_v4l2::print_fmt_desc(vid_formats[i].v4l2_fmt_desc_);

            // Run on all sizes
            for(int j = 0; j<vid_formats[i].v4l2_frm_sizes_.size(); ++j){

                auto e = vid_formats[i].v4l2_frm_sizes_[j].frmsize_;
                util_v4l2::print_frmsize(e, "\t");

                // Run on all intervals
                for(int k = 0; k< vid_formats[i].v4l2_frm_sizes_[j].frmival.size();++k){

                    auto e = vid_formats[i].v4l2_frm_sizes_[j].frmival[k];
                    util_v4l2::print_frmival(e, "\t\t");
                }
            }
        }
    }



    static std::vector<fmt_ext_t> get_video_formats_ext(int fd) {
        struct v4l2_fmtdesc fmt;
        struct v4l2_frmsizeenum frmsize;
        struct v4l2_frmivalenum frmival;

        std::vector<fmt_ext_t> capture_formats;   // results stored here.

        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while (util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {

            fmt_ext_t current_fmt(fmt);
            // Save format description
            //current_fmt.fmt = fmt;

            frmsize.pixel_format = fmt.pixelformat;
            frmsize.index = 0;
            while (util_v4l2::xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {

                frm_size_t vid_cap_frm_size(frmsize);

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
            capture_formats.push_back(current_fmt);
            fmt.index++;
        }

        return capture_formats;
    }

}//namespace




class capture {

public:

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    capture(std::string device, std::function<void(uint8_t *p_data, size_t len)> callback)
            : callback_(callback) {

        fd_ = util_v4l2::open_device(device, &err_);
        ASSERT_ERR_CODE(err_);

    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    capture(std::string device, uint32_t width, uint32_t height, pixel_format format,
            std::function<void(uint8_t *p_data, size_t len)> callback)
            : callback_(callback) {


        fd_ = util_v4l2::open_device(device, &err_);
        ASSERT_ERR_CODE(err_);

        util_v4l2::set_format(fd_, width, height, format, &err_);
        ASSERT_ERR_CODE(err_);

        util_v4l2::init_mmap(fd_, buffers, &err_);
        ASSERT_ERR_CODE(err_);

        util_v4l2::set_capture_steamon(fd_, &err_);
        ASSERT_ERR_CODE(err_);

        auto formats = util_v4l2::query_formats(fd_,nullptr);




        v4l2_format format1;
        format1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == util_v4l2::xioctl(fd_, VIDIOC_G_FMT, &format1)) {



        }
        int x = 5;

    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void run() {
        util_v4l2::queue_frames(fd_, NUM_OF_MAP_BUFFER, &err_);
        ASSERT_ERR_CODE(err_);

        // blocks indefintly.
        util_v4l2::mainloop(fd_, buffers, callback_, &err_);
        ASSERT_ERR_CODE(err_);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void set_format(int width, int height, pixel_format format) {
        util_v4l2::set_format(fd_, width, height, format, &err_);
        ASSERT_ERR_CODE(err_);

        util_v4l2::init_mmap(fd_, buffers, &err_);
        ASSERT_ERR_CODE(err_);

        util_v4l2::set_capture_steamon(fd_, &err_);
        ASSERT_ERR_CODE(err_);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    int read() {
        int numOfBytes = util_v4l2::read_one_frame(fd_, buffers, callback_, &err_);
        ASSERT_ERR_CODE(err_);
        return numOfBytes;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

private:

    int fd_{};
    constexpr static int NUM_OF_MAP_BUFFER = 4;
    struct util_v4l2::buffer buffers[NUM_OF_MAP_BUFFER];
    error_code err_ = error_code::ERR_NO_ERROR;
    std::function<void(uint8_t *p_data, size_t len)> callback_;
};








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
