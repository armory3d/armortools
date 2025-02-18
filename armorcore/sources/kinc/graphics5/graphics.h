#pragma once

#include <kinc/global.h>
#include <kinc/graphics5/texture.h>
#include "rendertarget.h"
#include "vertexstructure.h"
#include <kinc/backend/graphics5/indexbuffer.h>
#include <kinc/backend/graphics5/vertexbuffer.h>
#include <kinc/backend/graphics5/rendertarget.h>
#include <kinc/backend/graphics5/texture.h>
#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>
#include "pipeline.h"

/*! \file graphics.h
    \brief Contains the base G5-functionality.
*/

#define KINC_G5_CLEAR_COLOR 1
#define KINC_G5_CLEAR_DEPTH 2

bool kinc_g5_supports_raytracing(void);
int kinc_g5_max_bound_textures(void);
void kinc_g5_flush(void);
void kinc_g5_begin(kinc_g5_render_target_t *renderTarget);
void kinc_g5_end(void);
bool kinc_g5_swap_buffers(void);

void kinc_g5_internal_init(void);
void kinc_g5_internal_init_window(int depth_buffer_bits, bool vsync);
void kinc_g5_internal_destroy_window(void);
void kinc_g5_internal_destroy(void);

struct kinc_g5_pipeline;
struct kinc_g5_render_target;
struct kinc_g5_texture;
struct kinc_g5_texture_unit;
struct kinc_g5_constant_location;
struct kinc_g5_index_buffer;

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

typedef enum kinc_g4_usage {
    KINC_G4_USAGE_STATIC,
    KINC_G4_USAGE_DYNAMIC,
    KINC_G4_USAGE_READABLE
} kinc_g4_usage_t;

void kinc_g4_begin();
void kinc_g4_end();
void kinc_g5_clear(unsigned flags, unsigned color, float depth);
void kinc_g4_viewport(int x, int y, int width, int height);
void kinc_g4_scissor(int x, int y, int width, int height);
void kinc_g4_disable_scissor(void);
void kinc_g5_draw_indexed_vertices(void);
void kinc_g5_draw_indexed_vertices_from_to(int start, int count);
void kinc_g4_set_texture_addressing(struct kinc_g5_texture_unit unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);
void kinc_g5_set_pipeline(struct kinc_g5_pipeline *pipeline);
void kinc_g4_set_int(struct kinc_g5_constant_location location, int value);
void kinc_g4_set_int2(struct kinc_g5_constant_location location, int value1, int value2);
void kinc_g4_set_int3(struct kinc_g5_constant_location location, int value1, int value2, int value3);
void kinc_g4_set_int4(struct kinc_g5_constant_location location, int value1, int value2, int value3, int value4);
void kinc_g4_set_ints(struct kinc_g5_constant_location location, int *values, int count);
void kinc_g4_set_float(struct kinc_g5_constant_location location, float value);
void kinc_g4_set_float2(struct kinc_g5_constant_location location, float value1, float value2);
void kinc_g4_set_float3(struct kinc_g5_constant_location location, float value1, float value2, float value3);
void kinc_g4_set_float4(struct kinc_g5_constant_location location, float value1, float value2, float value3, float value4);
void kinc_g4_set_floats(struct kinc_g5_constant_location location, float *values, int count);
void kinc_g4_set_bool(struct kinc_g5_constant_location location, bool value);
void kinc_g4_set_matrix3(struct kinc_g5_constant_location location, kinc_matrix3x3_t *value);
void kinc_g4_set_matrix4(struct kinc_g5_constant_location location, kinc_matrix4x4_t *value);
void kinc_g4_set_texture_magnification_filter(struct kinc_g5_texture_unit unit, kinc_g4_texture_filter_t filter);
void kinc_g4_set_texture_minification_filter(struct kinc_g5_texture_unit unit, kinc_g4_texture_filter_t filter);
void kinc_g4_set_texture_mipmap_filter(struct kinc_g5_texture_unit unit, kinc_g4_mipmap_filter_t filter);
void kinc_g4_restore_render_target(void);
void kinc_g4_set_render_targets(struct kinc_g5_render_target **targets, int count);
void kinc_g4_set_texture(struct kinc_g5_texture_unit unit, struct kinc_g5_texture *texture);
void kinc_g4_compute(int x, int y, int z);
void kinc_g4_set_index_buffer(struct kinc_g5_index_buffer *buffer);

void kinc_g4_internal_init_window(int depth_buffer_bits, bool vsync);

void kinc_g4_render_target_use_color_as_texture(kinc_g5_render_target_t *renderTarget, struct kinc_g5_texture_unit unit);
void kinc_g4_render_target_use_depth_as_texture(kinc_g5_render_target_t *renderTarget, struct kinc_g5_texture_unit unit);
void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data);
void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels);

void kinc_g4_index_buffer_unlock_all(struct kinc_g5_index_buffer *buffer);
void kinc_g4_index_buffer_unlock(struct kinc_g5_index_buffer *buffer, int count);
