#include <png.h>
#include <stdio.h>
#include "image.h"

image *image_create(unsigned int width, unsigned int height) {
  image *img = (image *)malloc(sizeof(image));
  if (!img) return 0;

  img->data = calloc(width * height, 1);
  if (!img->data) {
    free(img);
    return 0;
  }

  img->width = width;
  img->height = height;

  return img;
}

void image_destroy(image *img) {
  if (img) {
    if (img->data) free(img->data);
    free(img);
  }
}

void image_set_pixel(image *img, unsigned int x, unsigned int y, unsigned char pixel) {
  if (x >= img->width) return;
  if (y >= img->height) return;
  img->data[y * img->width + x] = pixel;
}

unsigned char image_get_pixel(image *img, unsigned int x, unsigned int y) {
  if (x >= img->width) return 0;
  if (y >= img->height) return 0;
  return img->data[y * img->width + x];
}

char image_write_png(image *img, FILE *file) {
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) return 0;

  png_infop info = png_create_info_struct(png);
  if (!info) return 0;

  if (setjmp(png_jmpbuf(png))) return 0;
  png_init_io(png, file);

  if (setjmp(png_jmpbuf(png))) return 0;
  png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png, info);

  int x, y;
  unsigned char pixel;
  png_bytep row = (png_bytep)malloc(640 * 3 * sizeof(png_byte));

  for (y = 0; y < img->height; y++) {
    for (x = 0; x < img->width; x++) {
      pixel = image_get_pixel(img, x, y);
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
  return 1;
}

char image_downsample(image *src, image *dst) {
  return 1;
}
