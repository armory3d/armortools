#pragma once

#include <kinc/global.h>

#include "textureunit.h"

#include <kinc/backend/graphics5/rendertarget.h>

/*! \file rendertarget.h
    \brief Provides functions for handling render-targets.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g5_render_target_format {
	KINC_G5_RENDER_TARGET_FORMAT_32BIT,
	KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT,
	KINC_G5_RENDER_TARGET_FORMAT_16BIT_DEPTH,
	KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED,
	KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT
} kinc_g5_render_target_format_t;

typedef struct kinc_g5_render_target {
	int width;
	int height;
	int texWidth;
	int texHeight;
	int framebuffer_index;
	bool isCubeMap;
	bool isDepthAttachment;
	RenderTarget5Impl impl;
} kinc_g5_render_target_t;

/// <summary>
/// Allocates and initializes a regular render-target. The contents of the render-target are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <returns></returns>
void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits,
                                          int stencilBufferBits);

/// <summary>
/// Allocates and initializes a regular render-target. Can fall back to a regular render-target. The contents of the render-target are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <param name="samples_per_pixel"></param>
/// <returns></returns>
void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                             int depthBufferBits, int stencilBufferBits, int samples_per_pixel);

/// <summary>
/// Allocates and initializes a framebuffer. The contents of the framebuffer are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <returns></returns>
void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                      int depthBufferBits, int stencilBufferBits);

/// <summary>
/// Allocates and initializes a multisampled framebuffer. Can fall back to a regular framebuffer. The contents of the framebuffer are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <param name="samples_per_pixel"></param>
/// <returns></returns>
void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height,
                                                                         kinc_g5_render_target_format_t format, int depthBufferBits, int stencilBufferBits,
                                                                         int samples_per_pixel);

/// <summary>
/// Allocates and initializes a render-target-cube-map. The contents of the render-target are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="cubeMapSize"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <returns></returns>
void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *target, int cubeMapSize, kinc_g5_render_target_format_t format, int depthBufferBits,
                                               int stencilBufferBits);

/// <summary>
/// Allocates and initializes a multisampled render-target-cube-map. Can fall back to a regular cube-map. The contents of the render-target are undefined.
/// </summary>
/// <param name="target"></param>
/// <param name="cubeMapSize"></param>
/// <param name="format"></param>
/// <param name="depthBufferBits"></param>
/// <param name="stencilBufferBits"></param>
/// <param name="samples_per_pixel"></param>
/// <returns></returns>
void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                                  int depthBufferBits, int stencilBufferBits, int samples_per_pixel);

/// <summary>
/// Deallocates and destroys a render-target.
/// </summary>
/// <param name="renderTarget">The render-target to destroy</param>
void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target);

/// <summary>
/// Copies the depth and stencil-components of one render-target into another one.
/// </summary>
/// <param name="renderTarget">The render-target to copy the data into</param>
/// <param name="source">The render-target from which to copy the data</param>
/// <returns></returns>
void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source);

#ifdef __cplusplus
}
#endif
