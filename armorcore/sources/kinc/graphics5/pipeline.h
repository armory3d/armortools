#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/pipeline.h>

#include <kinc/graphics5/vertexstructure.h>

#include "constantlocation.h"
#include "graphics.h"

/*! \file pipeline.h
    \brief Provides functions for creating and using pipelines which configure the GPU for rendering.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_shader;

// identical to kinc_g4_blending_factor_t
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

// identical to kinc_g4_blending_operation_t
typedef enum {
	KINC_G5_BLENDOP_ADD,
	KINC_G5_BLENDOP_SUBTRACT,
	KINC_G5_BLENDOP_REVERSE_SUBTRACT,
	KINC_G5_BLENDOP_MIN,
	KINC_G5_BLENDOP_MAX
} kinc_g5_blending_operation_t;

typedef enum kinc_g5_cull_mode { KINC_G5_CULL_MODE_CLOCKWISE, KINC_G5_CULL_MODE_COUNTERCLOCKWISE, KINC_G5_CULL_MODE_NEVER } kinc_g5_cull_mode_t;

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

typedef enum kinc_g5_stencil_action {
	KINC_G5_STENCIL_ACTION_KEEP,
	KINC_G5_STENCIL_ACTION_ZERO,
	KINC_G5_STENCIL_ACTION_REPLACE,
	KINC_G5_STENCIL_ACTION_INCREMENT,
	KINC_G5_STENCIL_ACTION_INCREMENT_WRAP,
	KINC_G5_STENCIL_ACTION_DECREMENT,
	KINC_G5_STENCIL_ACTION_DECREMENT_WRAP,
	KINC_G5_STENCIL_ACTION_INVERT
} kinc_g5_stencil_action_t;

typedef struct kinc_g5_pipeline {
	kinc_g5_vertex_structure_t *inputLayout[16];
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;
	struct kinc_g5_shader *geometryShader;
	struct kinc_g5_shader *tessellationControlShader;
	struct kinc_g5_shader *tessellationEvaluationShader;

	kinc_g5_cull_mode_t cullMode;

	bool depthWrite;
	kinc_g5_compare_mode_t depthMode;

	kinc_g5_compare_mode_t stencilMode;
	kinc_g5_stencil_action_t stencilBothPass;
	kinc_g5_stencil_action_t stencilDepthFail;
	kinc_g5_stencil_action_t stencilFail;
	int stencilReferenceValue;
	int stencilReadMask;
	int stencilWriteMask;

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
	int stencilAttachmentBits;

	bool conservativeRasterization;

	PipelineState5Impl impl;
} kinc_g5_pipeline_t;

/// <summary>
/// Initializes a pipeline.
/// </summary>
/// <param name="state">The pipeline to initialize</param>
void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline);

void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipeline);

/// <summary>
/// Destroys a pipeline.
/// </summary>
/// <param name="pipeline">The pipeline to destroy</param>
void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline);

/// <summary>
/// Compiles a pipeline. After a pipeline has been compiled it is finalized. It cannot be compiled again and further changes to the pipeline are ignored.
/// </summary>
/// <param name="pipeline">The pipeline to compile</param>
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline);

/// <summary>
/// Searches for a constant/uniform and returns a constant-location which can be used to change the constant/uniform.
/// </summary>
/// <param name="pipeline">The pipeline to search in</param>
/// <param name="name">The name of the constant/uniform to find</param>
/// <returns>The constant-location of the constant/uniform</returns>
kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name);

/// <summary>
/// Searches for a texture-declaration and returns a texture-unit which can be used to assign a texture.
/// </summary>
/// <param name="pipeline">The pipeline to search in</param>
/// <param name="name">The name of the texture-declaration to search for</param>
/// <returns>The texture-unit of the texture-declaration</returns>
kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name);

#ifdef __cplusplus
}
#endif
