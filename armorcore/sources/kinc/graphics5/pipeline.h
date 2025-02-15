#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics5/pipeline.h>
#include <kinc/graphics5/vertexstructure.h>
#include "graphics.h"

/*! \file pipeline.h
    \brief Provides functions for creating and using pipelines which configure the GPU for rendering.
*/

typedef struct kinc_g5_constant_location {
	ConstantLocation5Impl impl;
} kinc_g5_constant_location_t;

typedef enum kinc_g5_shader_type {
	KINC_G5_SHADER_TYPE_FRAGMENT,
	KINC_G5_SHADER_TYPE_VERTEX,
	KINC_G5_SHADER_TYPE_COMPUTE,
	KINC_G5_SHADER_TYPE_COUNT
} kinc_g5_shader_type_t;

typedef struct kinc_g5_shader {
	Shader5Impl impl;
} kinc_g5_shader_t;

typedef struct kinc_g5_texture_unit {
	int stages[KINC_G5_SHADER_TYPE_COUNT];
} kinc_g5_texture_unit_t;

struct kinc_g5_shader;

typedef enum {
	KINC_G5_BLEND_ONE,
	KINC_G5_BLEND_ZERO,
	KINC_G5_BLEND_SOURCE_ALPHA,
	KINC_G5_BLEND_DEST_ALPHA,
	KINC_G5_BLEND_INV_SOURCE_ALPHA,
	KINC_G5_BLEND_INV_DEST_ALPHA,
	KINC_G5_BLEND_SOURCE_COLOR,
	KINC_G5_BLEND_DEST_COLOR,
	KINC_G5_BLEND_INV_SOURCE_COLOR,
	KINC_G5_BLEND_INV_DEST_COLOR,
	KINC_G5_BLEND_CONSTANT,
	KINC_G5_BLEND_INV_CONSTANT
} kinc_g5_blending_factor_t;

typedef enum {
	KINC_G5_BLENDOP_ADD,
	KINC_G5_BLENDOP_SUBTRACT,
	KINC_G5_BLENDOP_REVERSE_SUBTRACT,
	KINC_G5_BLENDOP_MIN,
	KINC_G5_BLENDOP_MAX
} kinc_g5_blending_operation_t;

typedef enum kinc_g5_cull_mode {
	KINC_G5_CULL_MODE_CLOCKWISE,
	KINC_G5_CULL_MODE_COUNTERCLOCKWISE,
	KINC_G5_CULL_MODE_NEVER
} kinc_g5_cull_mode_t;

typedef enum kinc_g5_compare_mode {
	KINC_G5_COMPARE_MODE_ALWAYS,
	KINC_G5_COMPARE_MODE_NEVER,
	KINC_G5_COMPARE_MODE_EQUAL,
	KINC_G5_COMPARE_MODE_NOT_EQUAL,
	KINC_G5_COMPARE_MODE_LESS,
	KINC_G5_COMPARE_MODE_LESS_EQUAL,
	KINC_G5_COMPARE_MODE_GREATER,
	KINC_G5_COMPARE_MODE_GREATER_EQUAL
} kinc_g5_compare_mode_t;

typedef struct kinc_g5_pipeline {
	kinc_g5_vertex_structure_t *inputLayout;
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;

	kinc_g5_cull_mode_t cullMode;
	bool depthWrite;
	kinc_g5_compare_mode_t depthMode;

	// One, Zero deactivates blending
	kinc_g5_blending_factor_t blend_source;
	kinc_g5_blending_factor_t blend_destination;
	kinc_g5_blending_operation_t blend_operation;
	kinc_g5_blending_factor_t alpha_blend_source;
	kinc_g5_blending_factor_t alpha_blend_destination;
	kinc_g5_blending_operation_t alpha_blend_operation;

	bool colorWriteMaskRed[8]; // Per render target
	bool colorWriteMaskGreen[8];
	bool colorWriteMaskBlue[8];
	bool colorWriteMaskAlpha[8];

	int colorAttachmentCount;
	kinc_g5_render_target_format_t colorAttachment[8];
	int depthAttachmentBits;

	PipelineState5Impl impl;
} kinc_g5_pipeline_t;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline);
kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name);
kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name);

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type);
void kinc_g5_shader_destroy(kinc_g5_shader_t *shader);

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
