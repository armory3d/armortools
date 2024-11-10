#pragma once

#include <kinc/global.h>

#include <kinc/graphics4/constantlocation.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/textureunit.h>

#include <kinc/backend/graphics4/pipeline.h>

/*! \file pipeline.h
    \brief Provides functions for creating and using pipelines which configure the GPU for rendering.
*/

#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum { KINC_G4_CULL_CLOCKWISE, KINC_G4_CULL_COUNTER_CLOCKWISE, KINC_G4_CULL_NOTHING } kinc_g4_cull_mode_t;

typedef enum {
	KINC_G4_STENCIL_KEEP,
	KINC_G4_STENCIL_ZERO,
	KINC_G4_STENCIL_REPLACE,
	KINC_G4_STENCIL_INCREMENT,
	KINC_G4_STENCIL_INCREMENT_WRAP,
	KINC_G4_STENCIL_DECREMENT,
	KINC_G4_STENCIL_DECREMENT_WRAP,
	KINC_G4_STENCIL_INVERT
} kinc_g4_stencil_action_t;

typedef struct kinc_g4_pipeline {
	struct kinc_g4_vertex_structure *input_layout[16];
	struct kinc_g4_shader *vertex_shader;
	struct kinc_g4_shader *fragment_shader;
	struct kinc_g4_shader *geometry_shader;
	struct kinc_g4_shader *tessellation_control_shader;
	struct kinc_g4_shader *tessellation_evaluation_shader;

	kinc_g4_cull_mode_t cull_mode;

	bool depth_write;
	kinc_g4_compare_mode_t depth_mode;

	kinc_g4_compare_mode_t stencil_front_mode;
	kinc_g4_stencil_action_t stencil_front_both_pass;
	kinc_g4_stencil_action_t stencil_front_depth_fail;
	kinc_g4_stencil_action_t stencil_front_fail;

	kinc_g4_compare_mode_t stencil_back_mode;
	kinc_g4_stencil_action_t stencil_back_both_pass;
	kinc_g4_stencil_action_t stencil_back_depth_fail;
	kinc_g4_stencil_action_t stencil_back_fail;

	int stencil_reference_value;
	int stencil_read_mask;
	int stencil_write_mask;

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
	int stencil_attachment_bits;

	bool conservative_rasterization;

	kinc_g4_pipeline_impl_t impl;
} kinc_g4_pipeline_t;

/// <summary>
/// Initializes a pipeline.
/// </summary>
/// <param name="state">The pipeline to initialize</param>
void kinc_g4_pipeline_init(kinc_g4_pipeline_t *pipeline);

/// <summary>
/// Destroys a pipeline.
/// </summary>
/// <param name="pipeline">The pipeline to destroy</param>
void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *pipeline);

/// <summary>
/// Compiles a pipeline. After a pipeline has been compiled it is finalized. It cannot be compiled again and further changes to the pipeline are ignored.
/// </summary>
/// <param name="pipeline">The pipeline to compile</param>
void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *pipeline);

/// <summary>
/// Searches for a constant/uniform and returns a constant-location which can be used to change the constant/uniform.
/// </summary>
/// <param name="pipeline">The pipeline to search in</param>
/// <param name="name">The name of the constant/uniform to find</param>
/// <returns>The constant-location of the constant/uniform</returns>
kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *pipeline, const char *name);

/// <summary>
/// Searches for a texture-declaration and returns a texture-unit which can be used to assign a texture.
/// </summary>
/// <param name="pipeline">The pipeline to search in</param>
/// <param name="name">The name of the texture-declaration to search for</param>
/// <returns>The texture-unit of the texture-declaration</returns>
kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *pipeline, const char *name);

void kinc_g4_internal_set_pipeline(kinc_g4_pipeline_t *pipeline);
void kinc_g4_internal_pipeline_set_defaults(kinc_g4_pipeline_t *pipeline);

#ifdef __cplusplus
}
#endif
