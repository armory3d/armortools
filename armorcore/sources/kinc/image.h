#pragma once

#include <kinc/global.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*! \file image.h
    \brief Functionality for creating and loading images. Image loading supports PNG, JPEG and the custom K format. K files can contain image data that uses
   texture-compression (see kinc_image_compression).
*/

typedef enum kinc_image_compression {
	KINC_IMAGE_COMPRESSION_NONE,
	KINC_IMAGE_COMPRESSION_DXT5,
	KINC_IMAGE_COMPRESSION_ASTC,
	KINC_IMAGE_COMPRESSION_PVRTC
} kinc_image_compression_t;

typedef enum kinc_image_format {
	KINC_IMAGE_FORMAT_RGBA32,
	KINC_IMAGE_FORMAT_GREY8,
	KINC_IMAGE_FORMAT_RGB24,
	KINC_IMAGE_FORMAT_RGBA128,
	KINC_IMAGE_FORMAT_RGBA64,
	KINC_IMAGE_FORMAT_A32,
	KINC_IMAGE_FORMAT_BGRA32,
	KINC_IMAGE_FORMAT_A16
} kinc_image_format_t;

typedef struct kinc_image {
	int width, height;
	kinc_image_format_t format;
	unsigned internal_format;
	kinc_image_compression_t compression;
	void *data;
	size_t data_size;
} kinc_image_t;

size_t kinc_image_init(kinc_image_t *image, void *memory, int width, int height, kinc_image_format_t format);
void kinc_image_init_from_bytes(kinc_image_t *image, void *data, int width, int height, kinc_image_format_t format);
void kinc_image_destroy(kinc_image_t *image);
uint8_t *kinc_image_get_pixels(kinc_image_t *image);
int kinc_image_format_sizeof(kinc_image_format_t format);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include "image.h"
#include <lz4x.h>

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/log.h>
#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#undef KINC_IMPLEMENTATION
#include <kinc/graphics4/graphics.h>
#include <kinc/io/filereader.h>
#include <kinc/math/core.h>
#define KINC_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>

int kinc_image_format_sizeof(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
		return 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_RGB24:
		return 3;
	}
	return -1;
}

size_t kinc_image_init(kinc_image_t *image, void *memory, int width, int height, kinc_image_format_t format) {
	image->width = width;
	image->height = height;
	image->format = format;
	image->compression = KINC_IMAGE_COMPRESSION_NONE;
	image->data = memory;
	return width * height * kinc_image_format_sizeof(format);
}

void kinc_image_init_from_bytes(kinc_image_t *image, void *data, int width, int height, kinc_image_format_t format) {
	image->width = width;
	image->height = height;
	image->format = format;
	image->compression = KINC_IMAGE_COMPRESSION_NONE;
	image->data = data;
}

void kinc_image_destroy(kinc_image_t *image) {
	// user has to free the data
	image->data = NULL;
}

uint8_t *kinc_image_get_pixels(kinc_image_t *image) {
	return (uint8_t *)image->data;
}

#endif
