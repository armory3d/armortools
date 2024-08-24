
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include "iron_array.h"
void *image_from_bytes(void *buffer, int width, int height, int format);

void *io_svg_parse(char *buf) {
	NSVGimage *image = nsvgParse(buf, "px", 96.0f);
	int w = (int)image->width;
	int h = (int)image->height;
	unsigned char *pixels = malloc(w * h * 4);
	struct NSVGrasterizer *rast = nsvgCreateRasterizer();
	nsvgRasterize(rast, image, 0, 0, 1, pixels, w, h, w * 4);
	nsvgDelete(image);
	nsvgDeleteRasterizer(rast);

	buffer_t *b = malloc(sizeof(buffer_t));
	b->buffer = pixels;
	b->length = b->capacity = w * h * 4;
	return image_from_bytes(b, w, h, 0);
}
