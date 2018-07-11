# libv4l2cxx


- [Intro](#Intro)
- [Integration](#integration)
- [Examples](#examples)
  - [Pipe to ffplay](#pipe-to-ffplay)
- [ffmpeg commands](#ffmpeg-commands)


## Intro
A simple C++ header library to capture frames from a usb camera into a callback function for image processing.


## How to use

Run a simple hello camera to see how easy. no more v4l2 boilrplate code.
```cpp
// callback function to pipe out image to stdout 
// later pipe to ffplay to display image
void callback_stdout_pipe(uint8_t *p_data, size_t len) {
    uint8_t outBuff[921600];
    util_v4l2::raw_to_rgb(p_data, 0, outBuff, 921600, 640 * 480, 8);
    fwrite(outBuff, 640*480*3, 1, stdout);
}

int main() {
    // a capture instance 
    // device, resolution, pixel format , and callback function this should be known in advance you can use 
    // v4l2-ctl command to figure that out.
    capture cap("/dev/video0", 640,480,pixel_format ::V4L2CXX_PIX_FMT_YUYV,callback_stdout_pipe);
    // start streaming - a blocking function.
    cap.run();
}
```

## Examples
- See example folder
### pipe to ffplay
./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60


## ffmpeg commands

#### Play video device
> ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format yuyv422 -framerate 30

> ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format mjpeg

> ffplay -f v4l2 -i /dev/video0 -video_size 1280x720 -pixel_format mjpeg -framerate 30

#### Pipe libv4l2 to ffplay, tell ffplay to use input (-i pipe:0 = stdin standrd input) 
> ./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60

#### List formats for device
> v4l2-ctl --list-formats

#### List exdtended formats including resolution info for device
> v4l2-ctl --list-formats-ext

> v4l2-ctl -L

#### List controls for device
> v4l2-ctl --all 

#### Change control option to value
> v4l2-ctl -c \<option>=\<value>

> v4l2-ctl -d /dev/video1 -c exposure_absolute=100

