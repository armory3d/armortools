#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/texture.h>
#include <kinc/image.h>

/*! \file texture.h
    \brief Provides functions for handling textures.
*/

typedef kinc_image_t kinc_g4_image_t;

typedef struct kinc_g4_texture {
	int tex_width;
	int tex_height;
	int tex_depth;
	kinc_image_format_t format;
	kinc_image_t *image;
	kinc_g4_texture_impl_t impl;
} kinc_g4_texture_t;

void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image);
void kinc_g4_texture_destroy(kinc_g4_texture_t *texture);
unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture);
void kinc_g4_texture_unlock(kinc_g4_texture_t *texture);
void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels);
void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level);
int kinc_g4_texture_stride(kinc_g4_texture_t *texture);
