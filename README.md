# libv4l2cxx


- [Design goals](#design-goals)
- [Integration](#integration)
- [Examples](#examples)
  - [JSON as first-class data type](#pipe-to-ffplay)


## Design goals

- TODO write this section.


## Integration

- TODO write this section.


## Examples

### pipe to ffplay
./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60
