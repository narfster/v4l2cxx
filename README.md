# libv4l2cxx


pipe from stdout to ffplay
./libv4l2cxx | ffplay -f rawvideo -i pipe:0 -video_size 640x480 -pixel_format rgb24 -framerate 60
