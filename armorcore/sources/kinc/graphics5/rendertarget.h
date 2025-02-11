#pragma once

#include <kinc/global.h>
#include "textureunit.h"
#include <kinc/backend/graphics5/rendertarget.h>

/*! \file rendertarget.h
    \brief Provides functions for handling render-targets.
*/

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
	bool isDepthAttachment;
	RenderTarget5Impl impl;
} kinc_g5_render_target_t;

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits);
void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits);
void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target);
void kinc_g5_render_target_set_depth_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source);
