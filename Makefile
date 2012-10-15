all: kinect

CFLAGS=-O3

kinect: image.o kinect.c
	cc $(CFLAGS) -I/usr/local/include/libfreenect -L/usr/local/lib -lpng -lfreenect -o kinect image.o kinect.c

image.o: image.h image.c
	cc $(CFLAGS) -c image.c

clean:
	rm -f image.o kinect
