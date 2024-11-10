#pragma once

#include <kinc/global.h>

#include <kinc/image.h>

#include "textureunit.h"

#include <kinc/backend/graphics5/texture.h>

/*! \file texture.h
    \brief Provides functions for handling textures.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_texture {
	int texWidth;
	int texHeight;
	kinc_image_format_t format;
	Texture5Impl impl;
} kinc_g5_texture_t;

/// <summary>
/// Allocates and initializes a texture without copying any data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="width">The width of the texture to create</param>
/// <param name="height">The height of the texture to create</param>
/// <param name="format">The format of the texture to create</param>
void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);

/// <summary>
/// Allocates and initializes a 3d-texture without copying any data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="width">The width of the texture to create</param>
/// <param name="height">The height of the texture to create</param>
/// <param name="depth">The depth of the texture to create</param>
/// <param name="format">The format of the texture to create</param>
void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format);

/// <summary>
/// Allocates and initializes a texture and copies image-data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="image">The image which's data is copied into the texture</param>
void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image);

// void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable);
// void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable);

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);

/// <summary>
/// Deallocates and destroys a texture.
/// </summary>
/// <param name="texture">The texture to destroy</param>
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture);

#ifdef KINC_ANDROID
void kinc_g5_texture_init_from_id(kinc_g5_texture_t *texture, unsigned texid);
#endif

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture);

void kinc_g5_texture_unlock(kinc_g5_texture_t *texture);

/// <summary>
/// Clears parts of a texture to a color.
/// </summary>
void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);

#if defined(KINC_IOS) || defined(KINC_MACOS)
void kinc_g5_texture_upload(kinc_g5_texture_t *texture, uint8_t *data);
#endif

/// <summary>
/// Generates the mipmap-chain for a texture.
/// </summary>
/// <param name="renderTarget">The render-target to create the mipmaps for</param>
/// <param name="levels">The number of mipmap-levels to generate</param>
void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels);

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level);

/// <summary>
/// Returns the stride of the first mipmap-layer of the texture in bytes.
/// </summary>
/// <param name="texture">The texture to figure out the stride for</param>
/// <returns>The stride of the first mipmap-layer in bytes</returns>
int kinc_g5_texture_stride(kinc_g5_texture_t *texture);

#ifdef __cplusplus
}
#endif
