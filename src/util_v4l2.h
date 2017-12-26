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
#include "v4l2cxx.h"

#define UTIL_CLEAR(x) memset(&(x), 0, sizeof(x))
#define SET_ERR_CODE(err,code) if(err !=nullptr) *err = code

struct man{
    man() {}

    virtual ~man() {

    }

    bool operator==(const man &rhs) const {
        return x == rhs.x;
    }



    int x;
};

enum class error_code{
    ERR_NO_ERROR = 0,
    ERR_CANNOT_OPEN_DEVICE = -1,
    ERR_CANNOT_SET_FORMAT = -2,
    ERR_QUERYING_CAP = -3,
    ERR_QUERYING_FORMAT = -4
};

std::ostream& operator << (std::ostream& os, const error_code& obj)
{
    os << static_cast<std::underlying_type<error_code>::type>(obj);
    return os;
}


enum class pixel_format{
    V4L2CXX_PIX_FMT_RGB332 = V4L2_PIX_FMT_RGB332,
    V4L2CXX_PIX_FMT_YVYU = V4L2_PIX_FMT_YVYU,
    V4L2CXX_PIX_FMT_MJPEG = V4L2_PIX_FMT_MJPEG

};
namespace util_v4l2{

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

        fd = open(device_node.c_str(), O_RDWR);
        if (fd == -1) {
            // couldn't find capture device
            perror("Opening Video device");
            SET_ERR_CODE(err,error_code ::ERR_CANNOT_OPEN_DEVICE);
            return -1;
        }
        return fd;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


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
        if (fmt.fmt.pix.bytesperline < min){
            fmt.fmt.pix.bytesperline = min;
        }

        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

        if (fmt.fmt.pix.sizeimage < min) {
            fmt.fmt.pix.sizeimage = min;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    v4l2_capability query_capabilites(int fd, error_code *err) {

        struct v4l2_capability caps;
        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            SET_ERR_CODE(err,error_code::ERR_QUERYING_CAP);
        }
        return caps;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    void printv4l2_capabilites(v4l2_capability cap){

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

    v4l2_format get_current_format(int fd) {

        v4l2_format format;
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 != util_v4l2::xioctl(fd, VIDIOC_G_FMT, &format)) {

            //struct v4l2_fmtdesc fmt2;
            //fmt2.index = 0;
            //fmt2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            //util_v4l2::xioctl(fd, VIDIOC_G_FMT, &fmt);

        }
        return format;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    std::vector<v4l2_fmtdesc> query_formats(int fd, error_code *err) {

        std::vector<v4l2_fmtdesc> formats;
        struct v4l2_fmtdesc fmt;
        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (-1 != util_v4l2::xioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
            formats.push_back(fmt);
            fmt.index++;
        }
        if(formats.size() == 0){
            SET_ERR_CODE(err,error_code ::ERR_QUERYING_FORMAT);
        }
        return formats;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void start_capturing(int fd, uint32_t numOfBuffers) {
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

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    static void init_mmap(int fd, buffer buffer[]) {
        struct v4l2_requestbuffers req;

        UTIL_CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                printf("ERROR does not support memory mapping\n");
                exit(EXIT_FAILURE);
            } else {
                printf("ERROR: VIDIOC_REQBUFS");
            }
        }

        if (req.count < 2) {
            fprintf(stderr, "Insufficient buffer memory\n");
            exit(EXIT_FAILURE);
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

            if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYBUF, &buf))
                printf("ERROR: VIDIOC_QUERYBUF");

            (buffer[i]).length = buf.length;
            (buffer[i]).start =
                    mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         fd, buf.m.offset);

            if (MAP_FAILED == (buffer[i]).start) {
                printf("ERROR: mmap");
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

}// namespace util_v4l2