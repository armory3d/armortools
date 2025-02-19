#pragma once

#include <kinc/global.h>
#include <kinc/matrix.h>
#include <kinc/vector.h>
#include <kinc/g5.h>
#include <kinc/g5_texture.h>
#include <kinc/backend/graphics5/g5_texture.h>

#define KINC_G5_CLEAR_COLOR 1
#define KINC_G5_CLEAR_DEPTH 2

bool kinc_g5_supports_raytracing(void);
int kinc_g5_max_bound_textures(void);
void kinc_g5_flush(void);
void kinc_g5_begin(struct kinc_g5_render_target *renderTarget);
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

void kinc_g4_render_target_use_color_as_texture(struct kinc_g5_render_target *renderTarget, struct kinc_g5_texture_unit unit);
void kinc_g4_render_target_use_depth_as_texture(struct kinc_g5_render_target *renderTarget, struct kinc_g5_texture_unit unit);
void kinc_g5_render_target_get_pixels(struct kinc_g5_render_target *renderTarget, uint8_t *data);
void kinc_g5_render_target_generate_mipmaps(struct kinc_g5_render_target *renderTarget, int levels);

void kinc_g4_index_buffer_unlock_all(struct kinc_g5_index_buffer *buffer);
void kinc_g4_index_buffer_unlock(struct kinc_g5_index_buffer *buffer, int count);

#define KINC_G5_MAX_VERTEX_ELEMENTS 16

typedef enum kinc_g5_vertex_data {
	KINC_G5_VERTEX_DATA_NONE = 0,
	KINC_G5_VERTEX_DATA_F32_1X = 1,
	KINC_G5_VERTEX_DATA_F32_2X = 2,
	KINC_G5_VERTEX_DATA_F32_3X = 3,
	KINC_G5_VERTEX_DATA_F32_4X = 4,
	KINC_G5_VERTEX_DATA_F32_4X4 = 5,
	KINC_G5_VERTEX_DATA_I8_1X = 6,
	KINC_G5_VERTEX_DATA_U8_1X = 7,
	KINC_G5_VERTEX_DATA_I8_1X_NORMALIZED = 8,
	KINC_G5_VERTEX_DATA_U8_1X_NORMALIZED = 9,
	KINC_G5_VERTEX_DATA_I8_2X = 10,
	KINC_G5_VERTEX_DATA_U8_2X = 11,
	KINC_G5_VERTEX_DATA_I8_2X_NORMALIZED = 12,
	KINC_G5_VERTEX_DATA_U8_2X_NORMALIZED = 13,
	KINC_G5_VERTEX_DATA_I8_4X = 14,
	KINC_G5_VERTEX_DATA_U8_4X = 15,
	KINC_G5_VERTEX_DATA_I8_4X_NORMALIZED = 16,
	KINC_G5_VERTEX_DATA_U8_4X_NORMALIZED = 17,
	KINC_G5_VERTEX_DATA_I16_1X = 18,
	KINC_G5_VERTEX_DATA_U16_1X = 19,
	KINC_G5_VERTEX_DATA_I16_1X_NORMALIZED = 20,
	KINC_G5_VERTEX_DATA_U16_1X_NORMALIZED = 21,
	KINC_G5_VERTEX_DATA_I16_2X = 22,
	KINC_G5_VERTEX_DATA_U16_2X = 23,
	KINC_G5_VERTEX_DATA_I16_2X_NORMALIZED = 24,
	KINC_G5_VERTEX_DATA_U16_2X_NORMALIZED = 25,
	KINC_G5_VERTEX_DATA_I16_4X = 26,
	KINC_G5_VERTEX_DATA_U16_4X = 27,
	KINC_G5_VERTEX_DATA_I16_4X_NORMALIZED = 28,
	KINC_G5_VERTEX_DATA_U16_4X_NORMALIZED = 29,
	KINC_G5_VERTEX_DATA_I32_1X = 30,
	KINC_G5_VERTEX_DATA_U32_1X = 31,
	KINC_G5_VERTEX_DATA_I32_2X = 32,
	KINC_G5_VERTEX_DATA_U32_2X = 33,
	KINC_G5_VERTEX_DATA_I32_3X = 34,
	KINC_G5_VERTEX_DATA_U32_3X = 35,
	KINC_G5_VERTEX_DATA_I32_4X = 36,
	KINC_G5_VERTEX_DATA_U32_4X = 37,
} kinc_g5_vertex_data_t;

typedef struct kinc_g5_vertex_element {
	const char *name;
	kinc_g5_vertex_data_t data;
} kinc_g5_vertex_element_t;

typedef struct kinc_g5_vertex_structure {
	kinc_g5_vertex_element_t elements[KINC_G5_MAX_VERTEX_ELEMENTS];
	int size;
} kinc_g5_vertex_structure_t;

static inline int kinc_g5_vertex_data_size(kinc_g5_vertex_data_t data) {
	switch (data) {
	default:
	case KINC_G5_VERTEX_DATA_NONE:
		return 0;
	case KINC_G5_VERTEX_DATA_F32_1X:
		return 1 * 4;
	case KINC_G5_VERTEX_DATA_F32_2X:
		return 2 * 4;
	case KINC_G5_VERTEX_DATA_F32_3X:
		return 3 * 4;
	case KINC_G5_VERTEX_DATA_F32_4X:
		return 4 * 4;
	case KINC_G5_VERTEX_DATA_F32_4X4:
		return 4 * 4 * 4;
	case KINC_G5_VERTEX_DATA_I8_1X:
	case KINC_G5_VERTEX_DATA_U8_1X:
	case KINC_G5_VERTEX_DATA_I8_1X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U8_1X_NORMALIZED:
		return 1 * 1;
	case KINC_G5_VERTEX_DATA_I8_2X:
	case KINC_G5_VERTEX_DATA_U8_2X:
	case KINC_G5_VERTEX_DATA_I8_2X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U8_2X_NORMALIZED:
		return 2 * 1;
	case KINC_G5_VERTEX_DATA_I8_4X:
	case KINC_G5_VERTEX_DATA_U8_4X:
	case KINC_G5_VERTEX_DATA_I8_4X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U8_4X_NORMALIZED:
		return 4 * 1;
	case KINC_G5_VERTEX_DATA_I16_1X:
	case KINC_G5_VERTEX_DATA_U16_1X:
	case KINC_G5_VERTEX_DATA_I16_1X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U16_1X_NORMALIZED:
		return 1 * 2;
	case KINC_G5_VERTEX_DATA_I16_2X:
	case KINC_G5_VERTEX_DATA_U16_2X:
	case KINC_G5_VERTEX_DATA_I16_2X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U16_2X_NORMALIZED:
		return 2 * 2;
	case KINC_G5_VERTEX_DATA_I16_4X:
	case KINC_G5_VERTEX_DATA_U16_4X:
	case KINC_G5_VERTEX_DATA_I16_4X_NORMALIZED:
	case KINC_G5_VERTEX_DATA_U16_4X_NORMALIZED:
		return 4 * 2;
	case KINC_G5_VERTEX_DATA_I32_1X:
	case KINC_G5_VERTEX_DATA_U32_1X:
		return 1 * 4;
	case KINC_G5_VERTEX_DATA_I32_2X:
	case KINC_G5_VERTEX_DATA_U32_2X:
		return 2 * 4;
	case KINC_G5_VERTEX_DATA_I32_3X:
	case KINC_G5_VERTEX_DATA_U32_3X:
		return 3 * 4;
	case KINC_G5_VERTEX_DATA_I32_4X:
	case KINC_G5_VERTEX_DATA_U32_4X:
		return 4 * 4;
	}
}

void kinc_g5_vertex_structure_init(kinc_g5_vertex_structure_t *structure);
void kinc_g5_vertex_structure_add(kinc_g5_vertex_structure_t *structure, const char *name, kinc_g5_vertex_data_t data);
