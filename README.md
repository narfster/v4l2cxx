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
```bash
ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format yuyv422 -framerate 30
ffplay -f v4l2 -i /dev/video0 -video_size 640x480  -pixel_format mjpeg
ffplay -f v4l2 -i /dev/video0 -video_size 1280x720 -pixel_format mjpeg -framerate 30

./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60

pipe:0 stdin

v4l2-ctl --list-formats

v4l2-ctl --list-formats-ext

v4l2-ctl -L

v4l2-ctl -c <option>=<value>
v4l2-ctl -d /dev/video1 -c exposure_absolute=100
'''
