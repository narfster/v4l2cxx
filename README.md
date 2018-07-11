# libv4l2cxx


- [Intro](#Intro)
- [Integration](#integration)
- [Examples](#examples)
  - [Pipe to ffplay](#pipe-to-ffplay)
- [ffmpeg commands](#ffmpeg-commands)


## Intro

Well, I am working on a project with lots of video processing elements. At one point in time i really just needed a callback fucntion to process frames arrving from a USB camera.  I found out that documnation / coding in c/c++ for newbies on how to accomplish this on linux was not a simple task.  my goal is to simplify.    

## Integration

- TODO write this section.


## Examples

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

