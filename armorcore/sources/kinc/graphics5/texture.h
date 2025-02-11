#pragma once

#include <kinc/global.h>
#include <kinc/image.h>
#include "textureunit.h"
#include <kinc/backend/graphics5/texture.h>

/*! \file texture.h
    \brief Provides functions for handling textures.
*/

typedef struct kinc_g5_texture {
	int texWidth;
	int texHeight;
	kinc_image_format_t format;
	Texture5Impl impl;
} kinc_g5_texture_t;

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image);
void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture);
uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture);
void kinc_g5_texture_unlock(kinc_g5_texture_t *texture);
void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels);
void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level);
int kinc_g5_texture_stride(kinc_g5_texture_t *texture);
