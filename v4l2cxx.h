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
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include "util_v4l2.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

void raw_to_rgb(void* inBuff, int inBuffSize, void* outBuff, int outBuffSize, uint32_t numOfPixels, int bitPerPixel)
{
    auto dst = static_cast<uint8_t *>(outBuff);

    auto shift = bitPerPixel - 8;  //i.e. 10bit - 8bit(1 byte) = 2, 12bit - 8bit = 4

    auto tmp = static_cast<uint16_t *>(inBuff);
    uint8_t c = 0;
    for (auto i = 0u; i < numOfPixels; i++)
    {
        uint16_t temp = (*tmp++) >> shift; //12 bit shift 4, 10bit shift 2
        *dst++ = static_cast<uint8_t>(temp);
        *dst++ = static_cast<uint8_t>(temp);
        *dst++ = static_cast<uint8_t>(temp);
    }
}

struct buffer {
    void *start;
    size_t length;
};

struct buffer *buffers;
static unsigned int n_buffers;


namespace util_v4l2_b {



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

    void printfmt(const struct v4l2_format &vfmt) {
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











    static void init_mmap(int fd) {
        struct v4l2_requestbuffers req;

        CLEAR(req);

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

        buffers = (buffer *) calloc(req.count, sizeof(*buffers));

        if (!buffers) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = n_buffers;

            if (-1 == util_v4l2::xioctl(fd, VIDIOC_QUERYBUF, &buf))
                printf("ERROR: VIDIOC_QUERYBUF");

            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start =
                    mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].start) {
                printf("ERROR: mmap");
            }
        }
    }



    static void process_image(const void *p, int size)
    {
        static uint32_t frame_number = 0;
        frame_number++;

        char filename[15];

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

    static int read_frame(int fd) {
        struct v4l2_buffer buf;
        unsigned int i;


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

        assert(buf.index < n_buffers);

        process_image(buffers[buf.index].start, buf.bytesused);

        if (-1 == util_v4l2::xioctl(fd, VIDIOC_QBUF, &buf))
        {
            printf("ERROR: read_frame VIDIOC_DQBUF\n");
            return -1;
        }

        return 1;
    }

    static void mainloop(int fd) {
        unsigned int count;

        count = 200;

        //while (count-- > 0) {
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

                if (read_frame(fd))
                    break;

                /* EAGAIN - continue select loop. */
            }
        }
    }




}

