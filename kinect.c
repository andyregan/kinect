#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libfreenect.h>
#include "image.h"

int running = 1;
image *depth_image;

void handle_interrupt(int signal) {
  running = 0;
}

void capture_depth_image(freenect_device *dev, void *v_depth, uint32_t timestamp) {
  int x, y;
  uint16_t *depth = (uint16_t*)v_depth;

  for (y = 0; y < 480; y++) {
    for (x = 0; x < 640; x++) {
      image_set_pixel(depth_image, x, y, depth[y * 640 + x]);
    }
  }
}

void draw_depth_image(FILE *file, int width, int height) {
  image *img = image_create(width, height);
  image_downsample(depth_image, img);

  int x, y;
  unsigned char pixel, c;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      pixel = image_get_pixel(img, x, y);

      if (pixel < 0x40) {
        c = '%';
      } else if (pixel < 0x80) {
        c = '+';
      } else if (pixel < 0xC0) {
        c = '.';
      } else {
        c = ' ';
      }

      fprintf(file, "%c", c);
    }
    fprintf(file, "\n");
  }

  image_destroy(img);
}

int main() {
  freenect_context *f_ctx;
  freenect_device *f_dev;

  if (freenect_init(&f_ctx, NULL) < 0) {
    fprintf(stderr, "freenect_init() failed\n");
    return 1;
  }

  freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

  if (freenect_num_devices(f_ctx) < 1) {
    fprintf(stderr, "no devices found\n");
    freenect_shutdown(f_ctx);
    return 1;
  }

  if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
    fprintf(stderr, "can't open device\n");
    freenect_shutdown(f_ctx);
    return 1;
  }

  depth_image = image_create(640, 480);

  freenect_set_led(f_dev, LED_GREEN);
  freenect_set_depth_callback(f_dev, capture_depth_image);
  freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
  freenect_start_depth(f_dev);

  if (signal(SIGINT, handle_interrupt) == SIG_IGN) {
    signal(SIGINT, SIG_IGN);
  }

  if (signal(SIGTERM, handle_interrupt) == SIG_IGN) {
    signal(SIGTERM, SIG_IGN);
  }

  fprintf(stdout, "\x1B[2J");

  while (running && freenect_process_events(f_ctx) >= 0) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    fprintf(stdout, "\x1B[1;1H");
    draw_depth_image(stdout, w.ws_col, w.ws_row - 1);
  }

  freenect_stop_depth(f_dev);
  freenect_set_led(f_dev, LED_OFF);
  freenect_close_device(f_dev);
  freenect_shutdown(f_ctx);

  image_destroy(depth_image);

  return 0;
}
