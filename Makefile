kinect: kinect.c
	cc -I/usr/local/include/libfreenect -L/usr/local/lib -lpng -lfreenect -o kinect kinect.c
