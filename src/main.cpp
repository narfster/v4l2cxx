#include "v4l2cxx.h"

#include <chrono>
#include <thread>

void callback_stdout_pipe(uint8_t *p_data, size_t len) {

    uint8_t outBuff[921600];
    util_v4l2::raw_to_rgb(p_data, 0, outBuff, 921600, 640 * 480, 8);
    fwrite(outBuff, 921600, 1, stdout);
}

int main(){

    capture cap("/dev/video0", 640,480,pixel_format ::V4L2CXX_PIX_FMT_YUYV,callback_stdout_pipe);
//    capture cap("/dev/video0", callback_stdout_pipe);
//    cap.set_format(640,480,pixel_format ::V4L2CXX_PIX_FMT_YUYV);
    cap.run();

//
//    while(1){
//        cap.read();
//        std::this_thread::sleep_for(std::chrono::milliseconds(16));
//    }


}


#if 0
int main() {

    struct util_v4l2::buffer buffers[4];

    error_code err_ = error_code::ERR_NO_ERROR;

    int fd = util_v4l2::open_device("/dev/video0", &err_);
    ASSERT_ERR_CODE(err_);

    auto cap = util_v4l2::query_capabilites(fd, &err_);

    auto vec = util_v4l2::query_formats(fd, &err_);

    util_v4l2::set_format(fd, 640, 480, pixel_format::V4L2CXX_PIX_FMT_YVYU, &err_);

    auto vec_format = util_v4l2::get_current_format(fd, &err_);

    //util_v4l2::printv4l2_fmt(vec_format);

    util_v4l2::init_mmap(fd, buffers, &err_);

    util_v4l2::start_capturing(fd, 4, &err_);

    util_v4l2::mainloop(fd, buffers, callback_stdout_pipe);

    return 0;
}
#endif