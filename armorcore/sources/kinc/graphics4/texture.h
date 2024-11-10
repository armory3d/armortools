#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/texture.h>
#include <kinc/image.h>

/*! \file texture.h
    \brief Provides functions for handling textures.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef kinc_image_t kinc_g4_image_t;

typedef struct kinc_g4_texture {
	int tex_width;
	int tex_height;
	int tex_depth;
	kinc_image_format_t format;
	kinc_image_t *image;
	kinc_g4_texture_impl_t impl;
} kinc_g4_texture_t;

/// <summary>
/// Allocates and initializes a texture without copying any data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="width">The width of the texture to create</param>
/// <param name="height">The height of the texture to create</param>
/// <param name="format">The format of the texture to create</param>
void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format);

/// <summary>
/// Allocates and initializes a 3d-texture without copying any data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="width">The width of the texture to create</param>
/// <param name="height">The height of the texture to create</param>
/// <param name="depth">The depth of the texture to create</param>
/// <param name="format">The format of the texture to create</param>
void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format);

/// <summary>
/// Allocates and initializes a texture and copies image-data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="image">The image which's data is copied into the texture</param>
void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image);

/// <summary>
/// Allocates and initializes a texture and copies image-data into it.
/// </summary>
/// <param name="texture">The texture to initialize</param>
/// <param name="image">The image which's data is copied into the texture</param>
void kinc_g4_texture_init_from_image3d(kinc_g4_texture_t *texture, kinc_image_t *image);

/// <summary>
/// Deallocates and destroys a texture.
/// </summary>
/// <param name="texture">The texture to destroy</param>
void kinc_g4_texture_destroy(kinc_g4_texture_t *texture);

unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture);

void kinc_g4_texture_unlock(kinc_g4_texture_t *texture);

/// <summary>
/// Clears parts of a texture to a color.
/// </summary>
void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);

/// <summary>
/// Generates the mipmap-chain for a texture.
/// </summary>
/// <param name="renderTarget">The render-target to create the mipmaps for</param>
/// <param name="levels">The number of mipmap-levels to generate</param>
void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels);

/// <summary>
/// Sets the mipmap for one level of a texture.
/// </summary>
/// <param name="texture">The texture to set a mipmap-level for</param>
/// <param name="mipmap">The image-data for the mipmap-level to set</param>
/// <param name="level">The mipmap-level to set</param>
void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level);

/// <summary>
/// Returns the stride of the first mipmap-layer of the texture in bytes.
/// </summary>
/// <param name="texture">The texture to figure out the stride for</param>
/// <returns>The stride of the first mipmap-layer in bytes</returns>
int kinc_g4_texture_stride(kinc_g4_texture_t *texture);

#ifdef KINC_ANDROID
void kinc_g4_texture_init_from_id(kinc_g4_texture_t *texture, unsigned texid);
#endif

#if defined(KINC_IOS) || defined(KINC_MACOS)
void kinc_g4_texture_upload(kinc_g4_texture_t *texture, uint8_t *data, int stride);
#endif

#ifdef __cplusplus
}
#endif
