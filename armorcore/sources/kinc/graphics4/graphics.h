#pragma once

#include <kinc/global.h>

#include <kinc/math/matrix.h>

#include "constantlocation.h"
#include "pipeline.h"
#include "textureunit.h"

/*! \file graphics.h
    \brief Contains the base G4-functionality.
*/

struct kinc_g4_compute_shader;
struct kinc_g4_pipeline;
struct kinc_g4_render_target;
struct kinc_g4_texture;

typedef enum {
	KINC_G4_TEXTURE_ADDRESSING_REPEAT,
	KINC_G4_TEXTURE_ADDRESSING_MIRROR,
	KINC_G4_TEXTURE_ADDRESSING_CLAMP,
	KINC_G4_TEXTURE_ADDRESSING_BORDER
} kinc_g4_texture_addressing_t;

typedef enum {
	KINC_G4_TEXTURE_DIRECTION_U,
	KINC_G4_TEXTURE_DIRECTION_V
} kinc_g4_texture_direction_t;

typedef enum {
	KINC_G4_TEXTURE_FILTER_POINT,
	KINC_G4_TEXTURE_FILTER_LINEAR,
	KINC_G4_TEXTURE_FILTER_ANISOTROPIC
} kinc_g4_texture_filter_t;

typedef enum {
	KINC_G4_MIPMAP_FILTER_NONE,
	KINC_G4_MIPMAP_FILTER_POINT,
	KINC_G4_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} kinc_g4_mipmap_filter_t;

int kinc_g4_max_bound_textures(void);
void kinc_g4_flush(void);
void kinc_g4_begin(int window);
void kinc_g4_end(int window);
bool kinc_g4_swap_buffers(void);

#define KINC_G4_CLEAR_COLOR 1
#define KINC_G4_CLEAR_DEPTH 2

void kinc_g4_clear(unsigned flags, unsigned color, float depth);
void kinc_g4_viewport(int x, int y, int width, int height);
void kinc_g4_scissor(int x, int y, int width, int height);
void kinc_g4_disable_scissor(void);
void kinc_g4_draw_indexed_vertices(void);
void kinc_g4_draw_indexed_vertices_from_to(int start, int count);
void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);
void kinc_g4_set_pipeline(struct kinc_g4_pipeline *pipeline);
void kinc_g4_set_int(kinc_g4_constant_location_t location, int value);
void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2);
void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3);
void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4);
void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count);
void kinc_g4_set_float(kinc_g4_constant_location_t location, float value);
void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2);
void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3);
void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4);
void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count);
void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value);
void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value);
void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value);
void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);
void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);
void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, kinc_g4_mipmap_filter_t filter);
void kinc_g4_restore_render_target(void);
void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count);
void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);
void kinc_g4_set_compute_shader(struct kinc_g4_compute_shader *shader);
void kinc_g4_compute(int x, int y, int z);

void kinc_g4_internal_init(void);
void kinc_g4_internal_init_window(int window, int depth_buffer_bits, bool vsync);
void kinc_g4_internal_destroy_window(int window);
void kinc_g4_internal_destroy(void);
