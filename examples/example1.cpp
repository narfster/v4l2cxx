//
// Created by dan on 4/29/18.
//

#include "../src/v4l2cxx.h"



void callback_stdout_pipe(uint8_t *p_data, size_t len) {

    uint8_t outBuff[921600];
    util_v4l2::raw_to_rgb(p_data, 0, outBuff, 921600, 640 * 480, 8);
    fwrite(outBuff, 640*480*3, 1, stdout);
}


int main() {

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    capture cap("/dev/video0", 640,480,pixel_format ::V4L2CXX_PIX_FMT_YUYV,callback_stdout_pipe);
    cap.run();

    }