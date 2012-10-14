#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libfreenect.h>
#include <png.h>

int running = 1;
unsigned char image[640 * 480];

void handle_interrupt(int signal) {
  running = 0;
}

void capture_depth_image(freenect_device *dev, void *v_depth, uint32_t timestamp) {
  int i;
  uint16_t *depth = (uint16_t*)v_depth;

  for (i = 0; i < 640 * 480; i++) {
    image[i] = depth[i];
  }
}

void write_depth_image(FILE *file) {
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) return;

  png_infop info = png_create_info_struct(png);
  if (!info) return;

  if (setjmp(png_jmpbuf(png))) return;
  png_init_io(png, file);

  if (setjmp(png_jmpbuf(png))) return;
  png_set_IHDR(png, info, 640, 480, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png, info);

  int x, y;
  unsigned char pixel;
  png_bytep row = (png_bytep)malloc(640 * 3 * sizeof(png_byte));

  for (y = 0; y < 480; y++) {
    for (x = 0; x < 640; x++) {
      pixel = image[(640 * y) + x];
      row[(x * 3) + 0] = pixel;
      row[(x * 3) + 1] = pixel;
      row[(x * 3) + 2] = pixel;
    }
    png_write_row(png, row);
  }

  free(row);
  png_write_end(png, NULL);
  png_free_data(png, info, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png, (png_infopp)NULL);
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

  while (running && freenect_process_events(f_ctx) >= 0);

  freenect_stop_depth(f_dev);
  freenect_set_led(f_dev, LED_OFF);
  freenect_close_device(f_dev);
  freenect_shutdown(f_ctx);

  write_depth_image(stdout);

  return 0;
}
