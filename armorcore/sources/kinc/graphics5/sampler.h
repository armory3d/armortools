#pragma once

#include <kinc/backend/graphics5/sampler.h>
#include <kinc/graphics5/pipeline.h>

/*! \file sampler.h
    \brief Provides functions for sampler objects.
*/

typedef enum kinc_g5_texture_addressing {
	KINC_G5_TEXTURE_ADDRESSING_REPEAT,
	KINC_G5_TEXTURE_ADDRESSING_MIRROR,
	KINC_G5_TEXTURE_ADDRESSING_CLAMP,
	KINC_G5_TEXTURE_ADDRESSING_BORDER
} kinc_g5_texture_addressing_t;

typedef enum kinc_g5_texture_filter {
	KINC_G5_TEXTURE_FILTER_POINT,
	KINC_G5_TEXTURE_FILTER_LINEAR,
	KINC_G5_TEXTURE_FILTER_ANISOTROPIC
} kinc_g5_texture_filter_t;

typedef enum kinc_g5_mipmap_filter {
	KINC_G5_MIPMAP_FILTER_NONE,
	KINC_G5_MIPMAP_FILTER_POINT,
	KINC_G5_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} kinc_g5_mipmap_filter_t;

typedef struct kinc_g5_sampler_options {
	kinc_g5_texture_addressing_t u_addressing;
	kinc_g5_texture_addressing_t v_addressing;

	kinc_g5_texture_filter_t minification_filter;
	kinc_g5_texture_filter_t magnification_filter;
	kinc_g5_mipmap_filter_t mipmap_filter;

} kinc_g5_sampler_options_t;

typedef struct kinc_g5_sampler {
	kinc_g5_sampler_impl_t impl;
} kinc_g5_sampler_t;

void kinc_g5_sampler_options_set_defaults(kinc_g5_sampler_options_t *options);
void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options);
void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler);
