# libv4l2cxx


- [Design goals](#design-goals)
- [Integration](#integration)
- [Examples](#examples)
  - [Pipe to ffplay](#pipe-to-ffplay)


## Design goals

- TODO write this section.


## Integration

- TODO write this section.


## Examples

### pipe to ffplay
./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60


## ffmpeg commands

Play video device
'''
$ ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format yuyv422 -framerate 30
'''

> $ ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format mjpeg

> $ ffplay -f v4l2 -i /dev/video0 -video_size 1280x720 -pixel_format mjpeg -framerate 30

Pipe libv4l2 to ffplay, tell ffplay to use input (-i pipe:0 = stdin standrd input) 
> $ ./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60

List formats for device
> $ v4l2-ctl --list-formats

List exdtended formats including resolution info for device
> $ v4l2-ctl --list-formats-ext

> $ v4l2-ctl -L

List controls for device
> v4l2-ctl --all 

Change control with to value
> $ v4l2-ctl -c <option>=<value>
$ v4l2-ctl -d /dev/video1 -c exposure_absolute=100

