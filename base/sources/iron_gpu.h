#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <iron_global.h>
#include <iron_math.h>
#include <iron_gpu.h>
#include <iron_array.h>
#include BACKEND_GPU_H

#define KINC_G5_CLEAR_COLOR 1
#define KINC_G5_CLEAR_DEPTH 2

enum kinc_internal_render_target_state {
	KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET,
	KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE
};

typedef enum kinc_image_compression {
	KINC_IMAGE_COMPRESSION_NONE,
	KINC_IMAGE_COMPRESSION_DXT5,
	KINC_IMAGE_COMPRESSION_ASTC
} kinc_image_compression_t;

typedef enum kinc_image_format {
	KINC_IMAGE_FORMAT_RGBA32,
	KINC_IMAGE_FORMAT_RGBA64,
	KINC_IMAGE_FORMAT_RGBA128,
	KINC_IMAGE_FORMAT_R8,
	KINC_IMAGE_FORMAT_R16,
	KINC_IMAGE_FORMAT_R32,
	KINC_IMAGE_FORMAT_BGRA32
} kinc_image_format_t;

typedef struct kinc_g5_texture {
	int width;
	int height;

	kinc_image_format_t format;
	kinc_image_compression_t compression;
	void *data;
	bool _uploaded;

	int framebuffer_index;
	bool isDepthAttachment;
	enum kinc_internal_render_target_state state;

	buffer_t *buffer;

	Texture5Impl impl;
} kinc_g5_texture_t;

int kinc_g5_max_bound_textures(void);
void kinc_g5_flush(void);
void kinc_g5_begin(kinc_g5_texture_t *renderTarget);
void kinc_g5_end(void);
bool kinc_g5_swap_buffers(void);

void kinc_g5_internal_init(void);
void kinc_g5_internal_init_window(int depth_buffer_bits, bool vsync);
void kinc_g5_internal_destroy_window(void);
void kinc_g5_internal_destroy(void);

struct kinc_g5_pipeline;
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
void kinc_g5_clear(unsigned color, float depth, unsigned flags);
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
void kinc_g4_set_render_targets(struct kinc_g5_texture **targets, int count);
void kinc_g4_set_texture(struct kinc_g5_texture_unit unit, struct kinc_g5_texture *texture);
void kinc_g4_compute(int x, int y, int z);
void kinc_g4_set_index_buffer(struct kinc_g5_index_buffer *buffer);

void kinc_g4_internal_init_window(int depth_buffer_bits, bool vsync);

void kinc_g4_render_target_use_depth_as_texture(struct kinc_g5_texture *renderTarget, struct kinc_g5_texture_unit unit);

void kinc_g4_index_buffer_unlock_all(struct kinc_g5_index_buffer *buffer);
void kinc_g4_index_buffer_unlock(struct kinc_g5_index_buffer *buffer, int count);

#define KINC_G5_MAX_VERTEX_ELEMENTS 16

typedef enum kinc_g5_vertex_data {
	KINC_G5_VERTEX_DATA_F32_1X = 0,
	KINC_G5_VERTEX_DATA_F32_2X = 1,
	KINC_G5_VERTEX_DATA_F32_3X = 2,
	KINC_G5_VERTEX_DATA_F32_4X = 3,
	KINC_G5_VERTEX_DATA_U8_4X_NORM = 4,
	KINC_G5_VERTEX_DATA_I16_2X_NORM = 5,
	KINC_G5_VERTEX_DATA_I16_4X_NORM = 6,
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
	case KINC_G5_VERTEX_DATA_F32_1X:
		return 1 * 4;
	case KINC_G5_VERTEX_DATA_F32_2X:
		return 2 * 4;
	case KINC_G5_VERTEX_DATA_F32_3X:
		return 3 * 4;
	case KINC_G5_VERTEX_DATA_F32_4X:
		return 4 * 4;
	case KINC_G5_VERTEX_DATA_U8_4X_NORM:
		return 4 * 1;
	case KINC_G5_VERTEX_DATA_I16_2X_NORM:
		return 2 * 2;
	case KINC_G5_VERTEX_DATA_I16_4X_NORM:
		return 4 * 2;
	}
}

static inline int kinc_g5_vertex_struct_size(kinc_g5_vertex_structure_t *s) {
	int size = 0;
	for (int i = 0; i < s->size; ++i) {
		size += kinc_g5_vertex_data_size(s->elements[i].data);
	}
	return size;
}

void kinc_g5_vertex_structure_init(kinc_g5_vertex_structure_t *structure);
void kinc_g5_vertex_structure_add(kinc_g5_vertex_structure_t *structure, const char *name, kinc_g5_vertex_data_t data);

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g5_texture_init_from_bytes(kinc_g5_texture_t *texture, void *data, int width, int height, kinc_image_format_t format);
void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture);
void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels);
void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level);
int kinc_g5_texture_stride(kinc_g5_texture_t *texture);

struct kinc_g5_texture_unit;

void kinc_g5_render_target_init(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits);
void kinc_g5_render_target_init_framebuffer(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits);
void kinc_g5_render_target_set_depth_from(kinc_g5_texture_t *target, kinc_g5_texture_t *source);

typedef struct kinc_g5_vertex_buffer {
	VertexBuffer5Impl impl;
} kinc_g5_vertex_buffer_t;

typedef struct {
	int myCount;
	kinc_g5_vertex_buffer_t _buffer[2];
	int _currentIndex;
	int _multiple;
} kinc_g4_vertex_buffer_impl_t;

typedef struct kinc_g4_vertex_buffer {
	kinc_g4_vertex_buffer_impl_t impl;
} kinc_g4_vertex_buffer_t;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpu_memory);
void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer);
float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer);
float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count);
void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer);
void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count);
int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer);
int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer);

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer);

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, kinc_g4_usage_t usage);
void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count);
void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count);
int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer);
int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer);

typedef struct kinc_g5_constant_buffer {
	uint8_t *data;
	ConstantBuffer5Impl impl;
} kinc_g5_constant_buffer_t;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size);
void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count);
void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer);
int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value);
void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value);
void kinc_g5_constant_buffer_set_int2(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2);
void kinc_g5_constant_buffer_set_int3(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3);
void kinc_g5_constant_buffer_set_int4(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4);
void kinc_g5_constant_buffer_set_ints(kinc_g5_constant_buffer_t *buffer, int offset, int *values, int count);
void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value);
void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2);
void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3);
void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4);
void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count);
void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value);
void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value);

extern bool kinc_g5_transpose_mat;

typedef struct kinc_g5_index_buffer {
	IndexBuffer5Impl impl;
} kinc_g5_index_buffer_t;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpu_memory);
void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer);
void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer);
void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count);
void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer);
void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count);
int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer);

typedef enum kinc_g5_shader_type {
	KINC_G5_SHADER_TYPE_FRAGMENT,
	KINC_G5_SHADER_TYPE_VERTEX,
	KINC_G5_SHADER_TYPE_COMPUTE,
	KINC_G5_SHADER_TYPE_COUNT
} kinc_g5_shader_type_t;

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

typedef struct kinc_g5_constant_location {
	ConstantLocation5Impl impl;
} kinc_g5_constant_location_t;

typedef struct kinc_g5_texture_unit {
	int stages[KINC_G5_SHADER_TYPE_COUNT];
} kinc_g5_texture_unit_t;

typedef struct kinc_g5_shader {
	Shader5Impl impl;
} kinc_g5_shader_t;

typedef struct kinc_g5_pipeline {
	kinc_g5_vertex_structure_t *input_layout;
	kinc_g5_shader_t *vertex_shader;
	kinc_g5_shader_t *fragment_shader;

	kinc_g5_cull_mode_t cull_mode;
	bool depth_write;
	kinc_g5_compare_mode_t depth_mode;

	// One, Zero deactivates blending
	kinc_g5_blending_factor_t blend_source;
	kinc_g5_blending_factor_t blend_destination;
	kinc_g5_blending_operation_t blend_operation;
	kinc_g5_blending_factor_t alpha_blend_source;
	kinc_g5_blending_factor_t alpha_blend_destination;
	kinc_g5_blending_operation_t alpha_blend_operation;

	bool color_write_mask_red[8]; // Per render target
	bool color_write_mask_green[8];
	bool color_write_mask_blue[8];
	bool color_write_mask_alpha[8];

	kinc_image_format_t color_attachment[8];
	int color_attachment_count;
	int depth_attachment_bits;

	PipelineState5Impl impl;
} kinc_g5_pipeline_t;

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

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline);
kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name);
kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name);

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type);
void kinc_g5_shader_destroy(kinc_g5_shader_t *shader);

void kinc_g5_sampler_options_set_defaults(kinc_g5_sampler_options_t *options);
void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options);
void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler);

void kinc_g5_internal_pipeline_set_defaults(kinc_g5_pipeline_t *pipeline);

struct kinc_g5_compute_shader;
struct kinc_g5_constant_buffer;
struct kinc_g5_index_buffer;
struct kinc_g5_vertex_buffer;
struct kinc_g5_pipeline;
struct kinc_g5_texture;
struct kinc_g5_texture;

typedef struct kinc_g5_command_list {
	CommandList5Impl impl;
} kinc_g5_command_list_t;

void kinc_g5_command_list_init(kinc_g5_command_list_t *list);
void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list);
void kinc_g5_command_list_begin(kinc_g5_command_list_t *list);
void kinc_g5_command_list_end(kinc_g5_command_list_t *list);
void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_texture *render_target, unsigned flags, unsigned color, float depth);
void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget);
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget);
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget);
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget);
void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list);
void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count);
void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height);
void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height);
void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list);
void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline);
void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, struct kinc_g5_compute_shader *shader);
void kinc_g5_command_list_set_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer);
void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);
void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_texture **targets, int count);
void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer);
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer);
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture);
void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);
void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);
void kinc_g5_command_list_set_compute_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size);
void kinc_g5_command_list_execute(kinc_g5_command_list_t *list);
void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list);
void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, struct kinc_g5_texture *render_target, uint8_t *data);
void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z);
void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture);
void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *target);
void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler);

void kinc_g5_render_target_get_pixels(kinc_g5_texture_t *render_target, uint8_t *data);

typedef struct kinc_g5_compute_shader {
	kinc_g5_compute_shader_impl impl;
} kinc_g5_compute_shader;

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *source, int length);
void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader);
kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name);
kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name);

struct kinc_g5_command_list;
struct kinc_g5_constant_buffer;
struct kinc_g5_index_buffer;
struct kinc_g5_texture;
struct kinc_g5_texture;
struct kinc_g5_vertex_buffer;

typedef struct kinc_g5_raytrace_pipeline {
	struct kinc_g5_constant_buffer *_constant_buffer;
	kinc_g5_raytrace_pipeline_impl_t impl;
} kinc_g5_raytrace_pipeline_t;

typedef struct kinc_g5_raytrace_acceleration_structure {
	kinc_g5_raytrace_acceleration_structure_impl_t impl;
} kinc_g5_raytrace_acceleration_structure_t;

bool kinc_g5_raytrace_supported(void);
void kinc_g5_raytrace_pipeline_init(kinc_g5_raytrace_pipeline_t *pipeline, struct kinc_g5_command_list *command_list,
    void *ray_shader, int ray_shader_size, struct kinc_g5_constant_buffer *constant_buffer);
void kinc_g5_raytrace_pipeline_destroy(kinc_g5_raytrace_pipeline_t *pipeline);

void kinc_g5_raytrace_acceleration_structure_init(kinc_g5_raytrace_acceleration_structure_t *accel);
void kinc_g5_raytrace_acceleration_structure_add(kinc_g5_raytrace_acceleration_structure_t *accel, struct kinc_g5_vertex_buffer *vb, struct kinc_g5_index_buffer *ib, kinc_matrix4x4_t transform);
void kinc_g5_raytrace_acceleration_structure_build(kinc_g5_raytrace_acceleration_structure_t *accel, struct kinc_g5_command_list *command_list,
    struct kinc_g5_vertex_buffer *_vb_full, struct kinc_g5_index_buffer *_ib_full);
void kinc_g5_raytrace_acceleration_structure_destroy(kinc_g5_raytrace_acceleration_structure_t *accel);

void kinc_g5_raytrace_set_textures(struct kinc_g5_texture *texpaint0, struct kinc_g5_texture *texpaint1, struct kinc_g5_texture *texpaint2,
    struct kinc_g5_texture *texenv, struct kinc_g5_texture *texsobol, struct kinc_g5_texture *texscramble, struct kinc_g5_texture *texrank);
void kinc_g5_raytrace_set_acceleration_structure(kinc_g5_raytrace_acceleration_structure_t *accel);
void kinc_g5_raytrace_set_pipeline(kinc_g5_raytrace_pipeline_t *pipeline);
void kinc_g5_raytrace_set_target(struct kinc_g5_texture *output);
void kinc_g5_raytrace_dispatch_rays(struct kinc_g5_command_list *command_list);
