all: kinect

kinect: image.o kinect.c
	cc -I/usr/local/include/libfreenect -L/usr/local/lib -lpng -lfreenect -o kinect image.o kinect.c

image.o: image.h image.c
	cc -c image.c

clean:
	rm -f image.o kinect
