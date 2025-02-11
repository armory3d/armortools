#pragma once

#include <kinc/global.h>
#include <kinc/graphics4/constantlocation.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/textureunit.h>
#include <kinc/backend/graphics4/pipeline.h>

/*! \file pipeline.h
    \brief Provides functions for creating and using pipelines which configure the GPU for rendering.
*/

struct kinc_g4_vertex_structure;
struct kinc_g4_shader;

typedef enum {
	KINC_G4_BLEND_ONE,
	KINC_G4_BLEND_ZERO,
	KINC_G4_BLEND_SOURCE_ALPHA,
	KINC_G4_BLEND_DEST_ALPHA,
	KINC_G4_BLEND_INV_SOURCE_ALPHA,
	KINC_G4_BLEND_INV_DEST_ALPHA,
	KINC_G4_BLEND_SOURCE_COLOR,
	KINC_G4_BLEND_DEST_COLOR,
	KINC_G4_BLEND_INV_SOURCE_COLOR,
	KINC_G4_BLEND_INV_DEST_COLOR,
	KINC_G4_BLEND_CONSTANT,
	KINC_G4_BLEND_INV_CONSTANT
} kinc_g4_blending_factor_t;

typedef enum {
	KINC_G4_BLENDOP_ADD,
	KINC_G4_BLENDOP_SUBTRACT,
	KINC_G4_BLENDOP_REVERSE_SUBTRACT,
	KINC_G4_BLENDOP_MIN,
	KINC_G4_BLENDOP_MAX
} kinc_g4_blending_operation_t;

typedef enum {
	KINC_G4_COMPARE_ALWAYS,
	KINC_G4_COMPARE_NEVER,
	KINC_G4_COMPARE_EQUAL,
	KINC_G4_COMPARE_NOT_EQUAL,
	KINC_G4_COMPARE_LESS,
	KINC_G4_COMPARE_LESS_EQUAL,
	KINC_G4_COMPARE_GREATER,
	KINC_G4_COMPARE_GREATER_EQUAL
} kinc_g4_compare_mode_t;

typedef enum {
	KINC_G4_CULL_CLOCKWISE,
	KINC_G4_CULL_COUNTER_CLOCKWISE,
	KINC_G4_CULL_NOTHING
} kinc_g4_cull_mode_t;

typedef struct kinc_g4_pipeline {
	struct kinc_g4_vertex_structure *input_layout;
	struct kinc_g4_shader *vertex_shader;
	struct kinc_g4_shader *fragment_shader;

	kinc_g4_cull_mode_t cull_mode;
	bool depth_write;
	kinc_g4_compare_mode_t depth_mode;

	// One, Zero deactivates blending
	kinc_g4_blending_factor_t blend_source;
	kinc_g4_blending_factor_t blend_destination;
	kinc_g4_blending_operation_t blend_operation;
	kinc_g4_blending_factor_t alpha_blend_source;
	kinc_g4_blending_factor_t alpha_blend_destination;
	kinc_g4_blending_operation_t alpha_blend_operation;

	bool color_write_mask_red[8]; // Per render target
	bool color_write_mask_green[8];
	bool color_write_mask_blue[8];
	bool color_write_mask_alpha[8];

	int color_attachment_count;
	kinc_g4_render_target_format_t color_attachment[8];
	int depth_attachment_bits;

	kinc_g4_pipeline_impl_t impl;
} kinc_g4_pipeline_t;

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *pipeline);
void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *pipeline);
void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *pipeline);
kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *pipeline, const char *name);
kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *pipeline, const char *name);

void kinc_g4_internal_set_pipeline(kinc_g4_pipeline_t *pipeline);
void kinc_g4_internal_pipeline_set_defaults(kinc_g4_pipeline_t *pipeline);
