#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics4/rendertarget.h>
#include "textureunit.h"
#include <stdint.h>

/*! \file rendertarget.h
    \brief Provides functions for handling render-targets.
*/

typedef enum kinc_g4_render_target_format {
	KINC_G4_RENDER_TARGET_FORMAT_32BIT,
	KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH,
	KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT
} kinc_g4_render_target_format_t;

typedef struct kinc_g4_render_target {
	int width;
	int height;
	int texWidth;
	int texHeight;
	bool isDepthAttachment;
	kinc_g4_render_target_impl_t impl;
} kinc_g4_render_target_t;

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, kinc_g4_render_target_format_t format, int depthBufferBits);
void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget);
void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit);
void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit);
void kinc_g4_render_target_set_depth_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source);
void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data);
void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels);
