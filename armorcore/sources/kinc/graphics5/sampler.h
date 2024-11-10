#pragma once

#include <kinc/backend/graphics5/sampler.h>
#include <kinc/graphics5/pipeline.h>

/*! \file sampler.h
    \brief Provides functions for sampler objects.
*/

#ifdef __cplusplus
extern "C" {
#endif

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
	kinc_g5_texture_addressing_t w_addressing;

	kinc_g5_texture_filter_t minification_filter;
	kinc_g5_texture_filter_t magnification_filter;
	kinc_g5_mipmap_filter_t mipmap_filter;

	float lod_min_clamp;
	float lod_max_clamp;

	uint16_t max_anisotropy;

	bool is_comparison;
	kinc_g5_compare_mode_t compare_mode;
} kinc_g5_sampler_options_t;

typedef struct kinc_g5_sampler {
	kinc_g5_sampler_impl_t impl;
} kinc_g5_sampler_t;

/// <summary>
/// Initializes the passed options-object with the default-options.
/// </summary>
/// <param name="options">The options-object for which the default-options will be set</param>
void kinc_g5_sampler_options_set_defaults(kinc_g5_sampler_options_t *options);

/// <summary>
/// Creates a sampler-object.
///
/// On platforms such as older OpenGL not all sampler attributes may be available.
/// </summary>
/// <param name="sampler">Pointer to the sampler object to initialize</param>
/// <param name="descriptor">Options for the sampler</param>
void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options);

/// <summary>
/// Destroys a sampler-object.
/// </summary>
/// <param name="sampler">The sampler-object to destroy</param>
void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler);

#ifdef __cplusplus
}
#endif
