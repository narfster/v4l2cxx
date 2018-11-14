
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
#include <cassert>
#include <functional>


#define UTIL_CLEAR(x) memset(&(x), 0, sizeof(x))
#define SET_ERR_CODE(err, code) if(err !=nullptr) *err = code
#define ASSERT_ERR_CODE(x)  if(x != error_code::ERR_NO_ERROR)\
                                {\
                                    print_err_code(x) ;\
                                    std::cerr << "\n"<< __FILE__ << ":" << __LINE__ << "  ";\
                                    exit(1);\
                                }


enum class error_code {
    ERR_NO_ERROR = 0,
    ERR_CANNOT_OPEN_DEVICE,
    ERR_CANNOT_SET_FORMAT,
    ERR_QUERYING_CAP,
    ERR_QUERYING_FORMAT,
    ERR_VIDIOC_QBUF,
    ERR_VIDIOC_STREAMON,
    ERR_NO_MMAP_SUPPORT,
    ERR_VIDIO_REQBUFS,
    ERR_INSUFFICIENT_MEM,
    ERR_MMAP_INIT,
    ERR_VIDIO_QUERYBUF,
    ERR_VIDIOC_S_FMT,
    ERR_VIDIOC_G_FMT,
    ERR_READ_FRAME,
    ERR_SELECT_TIMEOUT,
    ERR_SELECT,
};

void print_err_code(error_code err) {
    switch (err) {
        case error_code::ERR_CANNOT_OPEN_DEVICE:
            std::cerr << "ERR_CANNOT_OPEN_DEVICE" << " " << static_cast<int>(err);
            break;
        default:
            std::cerr << "ERROR # " << static_cast<int>(err);
            break;
    }

}

std::ostream &operator<<(std::ostream &os, const error_code &obj) {
    os << static_cast<std::underlying_type<error_code>::type>(obj);
    return os;
}


enum class pixel_format {
    V4L2CXX_PIX_FMT_RGB332 = V4L2_PIX_FMT_RGB332,
    V4L2CXX_PIX_FMT_YUYV = V4L2_PIX_FMT_YUYV,
    V4L2CXX_PIX_FMT_YVYU = V4L2_PIX_FMT_YVYU,
    V4L2CXX_PIX_FMT_MJPEG = V4L2_PIX_FMT_MJPEG

};

class function;
namespace util_v4l2 {

    struct buffer {
        void *start;
        size_t length;
    };


    static int xioctl(int fd, int request, void *arg) {
        int r;
        do r = ioctl(fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    int open_device(std::string device_node, error_code *err) {

        int fd;
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        fd = open(device_node.c_str(), O_RDWR);
        if (fd == -1) {
            // couldn't find capture device
            perror("Opening Video device");
            SET_ERR_CODE(err, error_code::ERR_CANNOT_OPEN_DEVICE);
            return -1;
        }
        return fd;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    void set_format(int fd, uint32_t width, uint32_t height, pixel_format format, error_code *err) {
        struct v4l2_format fmt;
        unsigned int min;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);

        //std::cout << "set_format()\n";
        fmt.fmt.pix.width = width; //replace
        fmt.fmt.pix.height = height; //replace
        fmt.fmt.pix.pixelformat = static_cast<__u32>(format); //replace
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            std::cout << "ERROR set_format\n";
            SET_ERR_CODE(err, error_code::ERR_VIDIOC_S_FMT);
        }
        // Note VIDIOC_S_FMT may change width and height.

        // Buggy driver paranoia.
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min) {
            fmt.fmt.pix.bytesperline = min;
        }

        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

        if (fmt.fmt.pix.sizeimage < min) {
            fmt.fmt.pix.sizeimage = min;
        }

        // Test Resolution was set correctly
        if (width != fmt.fmt.pix.width || height != fmt.fmt.pix.height) {
            SET_ERR_CODE(err, error_code::ERR_CANNOT_SET_FORMAT);
        }

        // Test format was set correctly
        if (static_cast<__u32>(format) != fmt.fmt.pix.pixelformat) {
            SET_ERR_CODE(err, error_code::ERR_CANNOT_SET_FORMAT);
        }

    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void set_capture_steamon(int fd, error_code *err){
        SET_ERR_CODE(err,error_code::ERR_NO_ERROR);
        enum v4l2_buf_type type;
        type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_STREAMON, &type)) {
            std::cerr << "ERROR: queue_frames VIDIOC_STREAMON\n";
            SET_ERR_CODE(err,error_code::ERR_NO_ERROR);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void set_capture_steamoff(int fd, error_code *err){
        SET_ERR_CODE(err,error_code::ERR_NO_ERROR);
        enum v4l2_buf_type type;
        type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_STREAMON, &type)) {
            std::cerr << "ERROR: queue_frames VIDIOC_STREAMON\n";
            SET_ERR_CODE(err,error_code::ERR_NO_ERROR);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    v4l2_capability query_capabilites(int fd, error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        struct v4l2_capability caps;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            SET_ERR_CODE(err, error_code::ERR_QUERYING_CAP);
        }
        return caps;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    int read_one_frame(int fd, util_v4l2::buffer *pBuffer, std::function<void(uint8_t *p_data, size_t len)> callback, error_code *err){
        SET_ERR_CODE(err,error_code::ERR_NO_ERROR);
        struct v4l2_buffer buf;


        UTIL_CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf)) {
            printf("ERROR: queue_frames VIDIOC_QBUF");
            SET_ERR_CODE(err,error_code::ERR_READ_FRAME);
        }

        UTIL_CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_DQBUF, &buf)) {
            SET_ERR_CODE(err,error_code::ERR_READ_FRAME);
            switch (errno) {
                case EAGAIN:
                    // (try again)
                    return 0;

                case EIO:
                    // I/O error
                    /* fall through */

                default:
                    std::cerr << "ERROR: read_frame VIDIOC_DQBUF\n";
                    return -1;
                    break;

            }
        }

        assert(buf.index < 4);
        int numOfBytes = 0;
        numOfBytes = buf.bytesused;
        callback((uint8_t *) pBuffer[buf.index].start, buf.bytesused);

        return numOfBytes;
    }
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    void printv4l2_capabilites(v4l2_capability cap) {

        if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
            std::cout << "V4L2_CAP_VIDEO_CAPTURE" << std::endl;
        }
        if (cap.device_caps & V4L2_CAP_VIDEO_OUTPUT) {
            std::cout << "V4L2_CAP_VIDEO_OUTPUT" << std::endl;
        }
        if (cap.device_caps & V4L2_CAP_STREAMING) {
            std::cout << "V4L2_CAP_STREAMING" << std::endl;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    v4l2_format get_current_format(int fd, error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);

        v4l2_format format;
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_G_FMT, &format)) {

            SET_ERR_CODE(err, error_code::ERR_VIDIOC_G_FMT);

        }
        return format;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    std::vector<v4l2_fmtdesc> query_formats(int fd, error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);

        std::vector<v4l2_fmtdesc> formats;
        struct v4l2_fmtdesc fmt;
        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (-1 != util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
            formats.push_back(fmt);
            fmt.index++;
        }
        if (formats.size() == 0) {
            SET_ERR_CODE(err, error_code::ERR_QUERYING_FORMAT);
        }
        return formats;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void queue_frames(int fd, uint32_t numOfBuffers, error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        unsigned int i;
        enum v4l2_buf_type type;

        // Queue into all MMAP buffers from capture device.
        for (i = 0; i < numOfBuffers; ++i) {
            struct v4l2_buffer buf;

            UTIL_CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf)) {
                printf("ERROR: queue_frames VIDIOC_QBUF");
                SET_ERR_CODE(err, error_code::ERR_VIDIOC_QBUF);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void stop_capturing(int fd, uint32_t numOfBuffers, error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        unsigned int i;
        enum v4l2_buf_type type;

        for (i = 0; i < numOfBuffers; ++i) {
            struct v4l2_buffer buf;

            UTIL_CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

//            if (-1 == util_v4l2::xioctl(fd, VIDIOC_DQBUF, &buf)) {
//                printf("ERROR: queue_frames VIDIOC_DQBUF");
//                SET_ERR_CODE(err, error_code::ERR_VIDIOC_QBUF);
//            }

        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_STREAMOFF, &type)) {
            printf("ERROR: queue_frames VIDIOC_STREAMON");
            //SET_ERR_CODE(err, error_code::VIDIOC_STREAMOFF);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void init_mmap(int fd, buffer buffer[], error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        struct v4l2_requestbuffers req;

        UTIL_CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                printf("ERROR does not support memory mapping\n");
                SET_ERR_CODE(err, error_code::ERR_NO_MMAP_SUPPORT);
            } else {
                printf("ERROR: VIDIOC_REQBUFS");
                SET_ERR_CODE(err, error_code::ERR_VIDIO_REQBUFS);
            }
        }

        if (req.count < 2) {
            fprintf(stderr, "Insufficient buffer memory\n");
            SET_ERR_CODE(err, error_code::ERR_INSUFFICIENT_MEM);
        }

        //*ppBuffer = (buffer *) calloc(req.count, sizeof(buffer));


        if (!buffer) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        for (auto i = 0; i < req.count; ++i) {
            struct v4l2_buffer buf;

            UTIL_CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
                printf("ERROR: VIDIOC_QUERYBUF");
                SET_ERR_CODE(err, error_code::ERR_VIDIO_QUERYBUF);
            }


            (buffer[i]).length = buf.length;
            (buffer[i]).start =
                    mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         fd, buf.m.offset);

            if (MAP_FAILED == (buffer[i]).start) {
                printf("ERROR: mmap");
                SET_ERR_CODE(err, error_code::ERR_MMAP_INIT);
            }
        }
    }

    typedef struct {
        unsigned flag;
        const char *str;
    } flag_def;

    static const flag_def service_def[] = {
            {V4L2_SLICED_TELETEXT_B, "teletext"},
            {V4L2_SLICED_WSS_625,    "wss"},
            {0, NULL}
    };

    std::string fcc2s(unsigned int val) {
        std::string s;

        s += val & 0x7f;
        s += (val >> 8) & 0x7f;
        s += (val >> 16) & 0x7f;
        s += (val >> 24) & 0x7f;
        if (val & (1 << 31))
            s += "-BE";
        return s;
    }

    static std::string num2s(unsigned num) {
        char buf[10];

        sprintf(buf, "%08x", num);
        return buf;
    }

    std::string buftype2s(int type) {
        switch (type) {
            case 0:
                return "Invalid";
            case V4L2_BUF_TYPE_VIDEO_CAPTURE:
                return "Video Capture";
            case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
                return "Video Capture Multiplanar";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT:
                return "Video Output";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
                return "Video Output Multiplanar";
            case V4L2_BUF_TYPE_VIDEO_OVERLAY:
                return "Video Overlay";
            case V4L2_BUF_TYPE_VBI_CAPTURE:
                return "VBI Capture";
            case V4L2_BUF_TYPE_VBI_OUTPUT:
                return "VBI Output";
            case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
                return "Sliced VBI Capture";
            case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
                return "Sliced VBI Output";
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
                return "Video Output Overlay";
            case V4L2_BUF_TYPE_SDR_CAPTURE:
                return "SDR Capture";
            case V4L2_BUF_TYPE_SDR_OUTPUT:
                return "SDR Output";
            default:
                return "Unknown (" + num2s(type) + ")";
        }
    }

    std::string field2s(int val) {
        switch (val) {
            case V4L2_FIELD_ANY:
                return "Any";
            case V4L2_FIELD_NONE:
                return "None";
            case V4L2_FIELD_TOP:
                return "Top";
            case V4L2_FIELD_BOTTOM:
                return "Bottom";
            case V4L2_FIELD_INTERLACED:
                return "Interlaced";
            case V4L2_FIELD_SEQ_TB:
                return "Sequential Top-Bottom";
            case V4L2_FIELD_SEQ_BT:
                return "Sequential Bottom-Top";
            case V4L2_FIELD_ALTERNATE:
                return "Alternating";
            case V4L2_FIELD_INTERLACED_TB:
                return "Interlaced Top-Bottom";
            case V4L2_FIELD_INTERLACED_BT:
                return "Interlaced Bottom-Top";
            default:
                return "Unknown (" + num2s(val) + ")";
        }
    }

    std::string colorspace2s(int val) {
        switch (val) {
            case V4L2_COLORSPACE_DEFAULT:
                return "Default";
            case V4L2_COLORSPACE_SMPTE170M:
                return "SMPTE 170M";
            case V4L2_COLORSPACE_SMPTE240M:
                return "SMPTE 240M";
            case V4L2_COLORSPACE_REC709:
                return "Rec. 709";
            case V4L2_COLORSPACE_BT878:
                return "Broken Bt878";
            case V4L2_COLORSPACE_470_SYSTEM_M:
                return "470 System M";
            case V4L2_COLORSPACE_470_SYSTEM_BG:
                return "470 System BG";
            case V4L2_COLORSPACE_JPEG:
                return "JPEG";
            case V4L2_COLORSPACE_SRGB:
                return "sRGB";
            case V4L2_COLORSPACE_ADOBERGB:
                return "AdobeRGB";
            case V4L2_COLORSPACE_DCI_P3:
                return "DCI-P3";
            case V4L2_COLORSPACE_BT2020:
                return "BT.2020";
            case V4L2_COLORSPACE_RAW:
                return "Raw";
            default:
                return "Unknown (" + num2s(val) + ")";
        }
    }

    static std::string xfer_func2s(int val) {
        switch (val) {
            case V4L2_XFER_FUNC_DEFAULT:
                return "Default";
            case V4L2_XFER_FUNC_709:
                return "Rec. 709";
            case V4L2_XFER_FUNC_SRGB:
                return "sRGB";
            case V4L2_XFER_FUNC_ADOBERGB:
                return "AdobeRGB";
            case V4L2_XFER_FUNC_DCI_P3:
                return "DCI-P3";
            case V4L2_XFER_FUNC_SMPTE2084:
                return "SMPTE 2084";
            case V4L2_XFER_FUNC_SMPTE240M:
                return "SMPTE 240M";
            case V4L2_XFER_FUNC_NONE:
                return "None";
            default:
                return "Unknown (" + num2s(val) + ")";
        }
    }

    static std::string ycbcr_enc2s(int val) {
        switch (val) {
            case V4L2_YCBCR_ENC_DEFAULT:
                return "Default";
            case V4L2_YCBCR_ENC_601:
                return "ITU-R 601";
            case V4L2_YCBCR_ENC_709:
                return "Rec. 709";
            case V4L2_YCBCR_ENC_XV601:
                return "xvYCC 601";
            case V4L2_YCBCR_ENC_XV709:
                return "xvYCC 709";
            case V4L2_YCBCR_ENC_SYCC:
                return "sYCC";
            case V4L2_YCBCR_ENC_BT2020:
                return "BT.2020";
            case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
                return "BT.2020 Constant Luminance";
            case V4L2_YCBCR_ENC_SMPTE240M:
                return "SMPTE 240M";
            default:
                return "Unknown (" + num2s(val) + ")";
        }
    }

    static std::string quantization2s(int val) {
        switch (val) {
            case V4L2_QUANTIZATION_DEFAULT:
                return "Default";
            case V4L2_QUANTIZATION_FULL_RANGE:
                return "Full Range";
            case V4L2_QUANTIZATION_LIM_RANGE:
                return "Limited Range";
            default:
                return "Unknown (" + num2s(val) + ")";
        }
    }

    std::string flags2s(unsigned val, const flag_def *def) {
        std::string s;

        while (def->flag) {
            if (val & def->flag) {
                if (s.length()) s += ", ";
                s += def->str;
                val &= ~def->flag;
            }
            def++;
        }
        if (val) {
            if (s.length()) s += ", ";
            s += num2s(val);
        }
        return s;
    }

    static const flag_def pixflags_def[] = {
            {V4L2_PIX_FMT_FLAG_PREMUL_ALPHA, "premultiplied-alpha"},
            {0, NULL}
    };

    std::string pixflags2s(unsigned flags) {
        return flags2s(flags, pixflags_def);
    }

    std::string service2s(unsigned service) {
        return flags2s(service, service_def);
    }

    void printv4l2_fmt(const struct v4l2_format &vfmt) {
        const flag_def vbi_def[] = {
                {V4L2_VBI_UNSYNC,     "unsynchronized"},
                {V4L2_VBI_INTERLACED, "interlaced"},
                {0, NULL}
        };
        printf("Format %s:\n", buftype2s(vfmt.type).c_str());

        switch (vfmt.type) {
            case V4L2_BUF_TYPE_VIDEO_CAPTURE:
            case V4L2_BUF_TYPE_VIDEO_OUTPUT:
                printf("\tWidth/Height      : %u/%u\n", vfmt.fmt.pix.width, vfmt.fmt.pix.height);
                printf("\tPixel Format      : '%s'\n", fcc2s(vfmt.fmt.pix.pixelformat).c_str());
                printf("\tField             : %s\n", field2s(vfmt.fmt.pix.field).c_str());
                printf("\tBytes per Line    : %u\n", vfmt.fmt.pix.bytesperline);
                printf("\tSize Image        : %u\n", vfmt.fmt.pix.sizeimage);
                printf("\tColorspace        : %s\n", colorspace2s(vfmt.fmt.pix.colorspace).c_str());
                printf("\tTransfer Function : %s\n", xfer_func2s(vfmt.fmt.pix.xfer_func).c_str());
                printf("\tYCbCr Encoding    : %s\n", ycbcr_enc2s(vfmt.fmt.pix.ycbcr_enc).c_str());
                printf("\tQuantization      : %s\n", quantization2s(vfmt.fmt.pix.quantization).c_str());
                if (vfmt.fmt.pix.priv == V4L2_PIX_FMT_PRIV_MAGIC)
                    printf("\tFlags             : %s\n", pixflags2s(vfmt.fmt.pix.flags).c_str());
                break;
            case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
                printf("\tWidth/Height      : %u/%u\n", vfmt.fmt.pix_mp.width, vfmt.fmt.pix_mp.height);
                printf("\tPixel Format      : '%s'\n", fcc2s(vfmt.fmt.pix_mp.pixelformat).c_str());
                printf("\tField             : %s\n", field2s(vfmt.fmt.pix_mp.field).c_str());
                printf("\tNumber of planes  : %u\n", vfmt.fmt.pix_mp.num_planes);
                printf("\tFlags             : %s\n", pixflags2s(vfmt.fmt.pix_mp.flags).c_str());
                printf("\tColorspace        : %s\n", colorspace2s(vfmt.fmt.pix_mp.colorspace).c_str());
                printf("\tTransfer Function : %s\n", xfer_func2s(vfmt.fmt.pix_mp.xfer_func).c_str());
                printf("\tYCbCr Encoding    : %s\n", ycbcr_enc2s(vfmt.fmt.pix_mp.ycbcr_enc).c_str());
                printf("\tQuantization      : %s\n", quantization2s(vfmt.fmt.pix_mp.quantization).c_str());
                for (int i = 0; i < vfmt.fmt.pix_mp.num_planes && i < VIDEO_MAX_PLANES; i++) {
                    printf("\tPlane %d           :\n", i);
                    printf("\t   Bytes per Line : %u\n", vfmt.fmt.pix_mp.plane_fmt[i].bytesperline);
                    printf("\t   Size Image     : %u\n", vfmt.fmt.pix_mp.plane_fmt[i].sizeimage);
                }
                break;
            case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
            case V4L2_BUF_TYPE_VIDEO_OVERLAY:
                printf("\tLeft/Top    : %d/%d\n",
                       vfmt.fmt.win.w.left, vfmt.fmt.win.w.top);
                printf("\tWidth/Height: %d/%d\n",
                       vfmt.fmt.win.w.width, vfmt.fmt.win.w.height);
                printf("\tField       : %s\n", field2s(vfmt.fmt.win.field).c_str());
                printf("\tChroma Key  : 0x%08x\n", vfmt.fmt.win.chromakey);
                printf("\tGlobal Alpha: 0x%02x\n", vfmt.fmt.win.global_alpha);
                printf("\tClip Count  : %u\n", vfmt.fmt.win.clipcount);
                if (vfmt.fmt.win.clips)
                    for (unsigned i = 0; i < vfmt.fmt.win.clipcount; i++) {
                        struct v4l2_rect &r = vfmt.fmt.win.clips[i].c;

                        printf("\t\tClip %2d: %ux%u@%ux%u\n", i,
                               r.width, r.height, r.left, r.top);
                    }
                printf("\tClip Bitmap : %s", vfmt.fmt.win.bitmap ? "Yes, " : "No\n");
                if (vfmt.fmt.win.bitmap) {
                    unsigned char *bitmap = (unsigned char *) vfmt.fmt.win.bitmap;
                    unsigned stride = (vfmt.fmt.win.w.width + 7) / 8;
                    unsigned cnt = 0;

                    for (unsigned y = 0; y < vfmt.fmt.win.w.height; y++)
                        for (unsigned x = 0; x < vfmt.fmt.win.w.width; x++)
                            if (bitmap[y * stride + x / 8] & (1 << (x & 7)))
                                cnt++;
                    printf("%u bits of %u are set\n", cnt,
                           vfmt.fmt.win.w.width * vfmt.fmt.win.w.height);
                }
                break;
            case V4L2_BUF_TYPE_VBI_CAPTURE:
            case V4L2_BUF_TYPE_VBI_OUTPUT:
                printf("\tSampling Rate   : %u Hz\n", vfmt.fmt.vbi.sampling_rate);
                printf("\tOffset          : %u samples (%g secs after leading edge)\n",
                       vfmt.fmt.vbi.offset,
                       (double) vfmt.fmt.vbi.offset / (double) vfmt.fmt.vbi.sampling_rate);
                printf("\tSamples per Line: %u\n", vfmt.fmt.vbi.samples_per_line);
                printf("\tSample Format   : %s\n", fcc2s(vfmt.fmt.vbi.sample_format).c_str());
                printf("\tStart 1st Field : %u\n", vfmt.fmt.vbi.start[0]);
                printf("\tCount 1st Field : %u\n", vfmt.fmt.vbi.count[0]);
                printf("\tStart 2nd Field : %u\n", vfmt.fmt.vbi.start[1]);
                printf("\tCount 2nd Field : %u\n", vfmt.fmt.vbi.count[1]);
                if (vfmt.fmt.vbi.flags)
                    printf("\tFlags           : %s\n", flags2s(vfmt.fmt.vbi.flags, vbi_def).c_str());
                break;
            case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
            case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
                printf("\tService Set    : %s\n",
                       service2s(vfmt.fmt.sliced.service_set).c_str());
                for (int i = 0; i < 24; i++) {
                    printf("\tService Line %2d: %8s / %-8s\n", i,
                           service2s(vfmt.fmt.sliced.service_lines[0][i]).c_str(),
                           service2s(vfmt.fmt.sliced.service_lines[1][i]).c_str());
                }
                printf("\tI/O Size       : %u\n", vfmt.fmt.sliced.io_size);
                break;
            case V4L2_BUF_TYPE_SDR_CAPTURE:
            case V4L2_BUF_TYPE_SDR_OUTPUT:
                printf("\tSample Format   : %s\n", fcc2s(vfmt.fmt.sdr.pixelformat).c_str());
                printf("\tBuffer Size     : %u\n", vfmt.fmt.sdr.buffersize);
                break;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static std::string fract2sec(const struct v4l2_fract &f)
    {
        char buf[100];

        sprintf(buf, "%.3f", (1.0 * f.numerator) / f.denominator);
        return buf;
    }

    static std::string fract2fps(const struct v4l2_fract &f)
    {
        char buf[100];

        sprintf(buf, "%.3f", (1.0 * f.denominator) / f.numerator);
        return buf;
    }

    static std::string frmtype2s(unsigned type)
    {
        static const char *types[] = {
                "Unknown",
                "Discrete",
                "Continuous",
                "Stepwise"
        };

        if (type > 3)
            type = 0;
        return types[type];
    }

    static void print_frmival(const struct v4l2_frmivalenum &frmival, const char *prefix)
    {
        printf("%s\tInterval: %s ", prefix, frmtype2s(frmival.type).c_str());
        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            printf("%ss (%s fps)\n", fract2sec(frmival.discrete).c_str(),
                   fract2fps(frmival.discrete).c_str());
        } else if (frmival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            printf("%ss - %ss (%s-%s fps)\n",
                   fract2sec(frmival.stepwise.min).c_str(),
                   fract2sec(frmival.stepwise.max).c_str(),
                   fract2fps(frmival.stepwise.max).c_str(),
                   fract2fps(frmival.stepwise.min).c_str());
        } else if (frmival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
            printf("%ss - %ss with step %ss (%s-%s fps)\n",
                   fract2sec(frmival.stepwise.min).c_str(),
                   fract2sec(frmival.stepwise.max).c_str(),
                   fract2sec(frmival.stepwise.step).c_str(),
                   fract2fps(frmival.stepwise.max).c_str(),
                   fract2fps(frmival.stepwise.min).c_str());
        }
    }

    static void print_frmsize(const struct v4l2_frmsizeenum &frmsize, const char *prefix)
    {
        printf("%s\tSize: %s ", prefix, frmtype2s(frmsize.type).c_str());
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            printf("%dx%d", frmsize.discrete.width, frmsize.discrete.height);
        } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            printf("%dx%d - %dx%d with step %d/%d",
                   frmsize.stepwise.min_width,
                   frmsize.stepwise.min_height,
                   frmsize.stepwise.max_width,
                   frmsize.stepwise.max_height,
                   frmsize.stepwise.step_width,
                   frmsize.stepwise.step_height);
        }
        printf("\n");
    }

    static const flag_def fmtdesc_def[] = {
            { V4L2_FMT_FLAG_COMPRESSED, "compressed" },
            { V4L2_FMT_FLAG_EMULATED, "emulated" },
            { 0, NULL }
    };

    std::string fmtdesc2s(unsigned flags)
    {
        return flags2s(flags, fmtdesc_def);
    }

    static void print_video_formats_ext(int fd, __u32 type)
    {
        struct v4l2_fmtdesc fmt;
        struct v4l2_frmsizeenum frmsize;
        struct v4l2_frmivalenum frmival;

        fmt.index = 0;
        fmt.type = type;
        while (xioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
            printf("\tIndex       : %d\n", fmt.index);
            printf("\tType        : %s\n", buftype2s(type).c_str());
            printf("\tPixel Format: '%s'", fcc2s(fmt.pixelformat).c_str());
            if (fmt.flags)
                printf(" (%s)", fmtdesc2s(fmt.flags).c_str());
            printf("\n");
            printf("\tName        : %s\n", fmt.description);
            frmsize.pixel_format = fmt.pixelformat;
            frmsize.index = 0;
            while (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
                print_frmsize(frmsize, "\t");
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                    frmival.index = 0;
                    frmival.pixel_format = fmt.pixelformat;
                    frmival.width = frmsize.discrete.width;
                    frmival.height = frmsize.discrete.height;
                    while (xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0) {
                        print_frmival(frmival, "\t\t");
                        frmival.index++;
                    }
                }
                frmsize.index++;
            }
            printf("\n");
            fmt.index++;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    static void print_fmt_desc(v4l2_fmtdesc fmt){
        printf("\tIndex       : %d\n", fmt.index);
        printf("\tType        : %s\n", buftype2s(V4L2_BUF_TYPE_VIDEO_CAPTURE).c_str());
        printf("\tPixel Format: '%s'", fcc2s(fmt.pixelformat).c_str());
        if (fmt.flags)
            printf(" (%s)", fmtdesc2s(fmt.flags).c_str());
        printf("\n");
        printf("\tName        : %s\n", fmt.description);
    }








    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void
    raw_to_rgb(void *inBuff, int inBuffSize, void *outBuff, int outBuffSize, uint32_t numOfPixels, int bitPerPixel) {
        auto dst = static_cast<uint8_t *>(outBuff);

        auto shift = bitPerPixel - 8;  //i.e. 10bit - 8bit(1 byte) = 2, 12bit - 8bit = 4

        auto tmp = static_cast<uint16_t *>(inBuff);
        for (auto i = 0u; i < numOfPixels; i++) {
            uint16_t temp = (*tmp++) >> shift; //12 bit shift 4, 10bit shift 2
            *dst++ = static_cast<uint8_t>(temp);
            *dst++ = static_cast<uint8_t>(temp);
            *dst++ = static_cast<uint8_t>(temp);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void process_image(const void *p, int size) {
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
        if (enable_stdout) {
            //fwrite(outBuff, 921600, 1, fp);
            fwrite(outBuff, 921600, 1, stdout);
        }


//        fflush(fp);
//        fclose(fp);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static int
    read_frame(int fd, util_v4l2::buffer *pBuffer, std::function<void(uint8_t *p_data, size_t len)> callback,
               error_code *err) {
        SET_ERR_CODE(err, error_code::ERR_NO_ERROR);
        int numOfBytes = 0;

        struct v4l2_buffer buf;
        UTIL_CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_DQBUF, &buf)) {
            SET_ERR_CODE(err, error_code::ERR_READ_FRAME);
            switch (errno) {
                case EAGAIN:
                    // (try again)
                    return 0;

                case EIO:
                    // I/O error
                    /* fall through */

                default:
                    std::cerr << "ERROR: read_frame VIDIOC_DQBUF\n";
                    return -1;
                    break;

            }
        }

        assert(buf.index < 4);

        callback((uint8_t *) pBuffer[buf.index].start, buf.bytesused);

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf)) {
            std::cerr << "ERROR: read_frame VIDIOC_DQBUF\n";
            SET_ERR_CODE(err, error_code::ERR_READ_FRAME);
            return -1;
        }

        numOfBytes = buf.bytesused;
        return numOfBytes;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void
    mainloop(int fd, util_v4l2::buffer *pBuffer, std::function<void(uint8_t *p_data, size_t len)> callback,
             error_code *err) {

//        int num_of_frames = 200;

        for (;;) {
//            if(num_of_frames < 0)
//                return;

//            std::cerr << num_of_frames <<"  \n";
//            num_of_frames--;

            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 5;
            tv.tv_usec = 0;

//            On success, select() return the number of file
//            descriptors contained in the three returned descriptor sets (that is,
//                    the total number of bits that are set in readfds, writefds,
//            exceptfds) which may be zero if the timeout expires before anything
//            interesting happens.  On error, -1 is returned, and errno is set to
//            indicate the error; the file descriptor sets are unmodified, and
//            timeout becomes undefined.
            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno) {
                    continue;
                }

                SET_ERR_CODE(err, error_code::ERR_SELECT);
                return;
            }

            if (0 == r) {
                std::cerr << "ERROR: select timeout\n";
                SET_ERR_CODE(err, error_code::ERR_SELECT_TIMEOUT);
            }

            if (read_frame(fd, pBuffer, callback,err) == -1) {
                SET_ERR_CODE(err, error_code::ERR_READ_FRAME);
                // exit loop
                return;
            }

            /* EAGAIN (try again) - continue select loop. */
        }
    }

}// namespace util_v4l2
