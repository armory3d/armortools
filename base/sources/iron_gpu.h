#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <iron_global.h>
#include <iron_math.h>
#include <iron_gpu.h>
#include <iron_array.h>
#include BACKEND_GPU_H

#define IRON_GPU_CLEAR_COLOR 1
#define IRON_GPU_CLEAR_DEPTH 2

enum iron_internal_render_target_state {
	IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET,
	IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE
};

typedef enum iron_image_compression {
	IRON_IMAGE_COMPRESSION_NONE,
	IRON_IMAGE_COMPRESSION_DXT5,
	IRON_IMAGE_COMPRESSION_ASTC
} iron_image_compression_t;

typedef enum iron_image_format {
	IRON_IMAGE_FORMAT_RGBA32,
	IRON_IMAGE_FORMAT_RGBA64,
	IRON_IMAGE_FORMAT_RGBA128,
	IRON_IMAGE_FORMAT_R8,
	IRON_IMAGE_FORMAT_R16,
	IRON_IMAGE_FORMAT_R32,
	IRON_IMAGE_FORMAT_BGRA32
} iron_image_format_t;

typedef struct iron_gpu_texture {
	int width;
	int height;

	iron_image_format_t format;
	iron_image_compression_t compression;
	void *data;
	bool _uploaded;

	int framebuffer_index;
	bool isDepthAttachment;
	enum iron_internal_render_target_state state;

	buffer_t *buffer;

	Texture5Impl impl;
} iron_gpu_texture_t;

int iron_gpu_max_bound_textures(void);
void iron_gpu_flush(void);
void iron_gpu_begin(iron_gpu_texture_t *renderTarget);
void iron_gpu_end(void);
bool iron_gpu_swap_buffers(void);

void iron_gpu_internal_init(void);
void iron_gpu_internal_init_window(int depth_buffer_bits, bool vsync);
void iron_gpu_internal_destroy_window(void);
void iron_gpu_internal_destroy(void);

struct iron_gpu_pipeline;
struct iron_gpu_texture;
struct iron_gpu_texture_unit;
struct iron_gpu_constant_location;
struct iron_gpu_index_buffer;

typedef enum {
	GPU_TEXTURE_ADDRESSING_REPEAT,
	GPU_TEXTURE_ADDRESSING_MIRROR,
	GPU_TEXTURE_ADDRESSING_CLAMP,
	GPU_TEXTURE_ADDRESSING_BORDER
} gpu_texture_addressing_t;

typedef enum {
	GPU_TEXTURE_DIRECTION_U,
	GPU_TEXTURE_DIRECTION_V
} gpu_texture_direction_t;

typedef enum {
	GPU_TEXTURE_FILTER_POINT,
	GPU_TEXTURE_FILTER_LINEAR,
	GPU_TEXTURE_FILTER_ANISOTROPIC
} gpu_texture_filter_t;

typedef enum {
	GPU_MIPMAP_FILTER_NONE,
	GPU_MIPMAP_FILTER_POINT,
	GPU_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} gpu_mipmap_filter_t;

typedef enum {
    GPU_USAGE_STATIC,
    GPU_USAGE_DYNAMIC,
    GPU_USAGE_READABLE
} gpu_usage_t;

void gpu_begin();
void gpu_end();
void iron_gpu_clear(unsigned color, float depth, unsigned flags);
void gpu_viewport(int x, int y, int width, int height);
void gpu_scissor(int x, int y, int width, int height);
void gpu_disable_scissor(void);
void iron_gpu_draw_indexed_vertices(void);
void iron_gpu_draw_indexed_vertices_from_to(int start, int count);
void gpu_set_texture_addressing(struct iron_gpu_texture_unit unit, gpu_texture_direction_t dir, gpu_texture_addressing_t addressing);
void iron_gpu_set_pipeline(struct iron_gpu_pipeline *pipeline);
void gpu_set_int(struct iron_gpu_constant_location location, int value);
void gpu_set_int2(struct iron_gpu_constant_location location, int value1, int value2);
void gpu_set_int3(struct iron_gpu_constant_location location, int value1, int value2, int value3);
void gpu_set_int4(struct iron_gpu_constant_location location, int value1, int value2, int value3, int value4);
void gpu_set_ints(struct iron_gpu_constant_location location, int *values, int count);
void gpu_set_float(struct iron_gpu_constant_location location, float value);
void gpu_set_float2(struct iron_gpu_constant_location location, float value1, float value2);
void gpu_set_float3(struct iron_gpu_constant_location location, float value1, float value2, float value3);
void gpu_set_float4(struct iron_gpu_constant_location location, float value1, float value2, float value3, float value4);
void gpu_set_floats(struct iron_gpu_constant_location location, float *values, int count);
void gpu_set_bool(struct iron_gpu_constant_location location, bool value);
void gpu_set_matrix3(struct iron_gpu_constant_location location, iron_matrix3x3_t *value);
void gpu_set_matrix4(struct iron_gpu_constant_location location, iron_matrix4x4_t *value);
void gpu_set_texture_magnification_filter(struct iron_gpu_texture_unit unit, gpu_texture_filter_t filter);
void gpu_set_texture_minification_filter(struct iron_gpu_texture_unit unit, gpu_texture_filter_t filter);
void gpu_set_texture_mipmap_filter(struct iron_gpu_texture_unit unit, gpu_mipmap_filter_t filter);
void gpu_restore_render_target(void);
void gpu_set_render_targets(struct iron_gpu_texture **targets, int count);
void gpu_set_texture(struct iron_gpu_texture_unit unit, struct iron_gpu_texture *texture);
void gpu_compute(int x, int y, int z);
void gpu_set_index_buffer(struct iron_gpu_index_buffer *buffer);

void gpu_internal_init_window(int depth_buffer_bits, bool vsync);

void gpu_render_target_use_depth_as_texture(struct iron_gpu_texture *renderTarget, struct iron_gpu_texture_unit unit);

void gpu_index_buffer_unlock_all(struct iron_gpu_index_buffer *buffer);
void gpu_index_buffer_unlock(struct iron_gpu_index_buffer *buffer, int count);

#define IRON_GPU_MAX_VERTEX_ELEMENTS 16

typedef enum iron_gpu_vertex_data {
	IRON_GPU_VERTEX_DATA_F32_1X = 0,
	IRON_GPU_VERTEX_DATA_F32_2X = 1,
	IRON_GPU_VERTEX_DATA_F32_3X = 2,
	IRON_GPU_VERTEX_DATA_F32_4X = 3,
	IRON_GPU_VERTEX_DATA_U8_4X_NORM = 4,
	IRON_GPU_VERTEX_DATA_I16_2X_NORM = 5,
	IRON_GPU_VERTEX_DATA_I16_4X_NORM = 6,
} iron_gpu_vertex_data_t;

typedef struct iron_gpu_vertex_element {
	const char *name;
	iron_gpu_vertex_data_t data;
} iron_gpu_vertex_element_t;

typedef struct iron_gpu_vertex_structure {
	iron_gpu_vertex_element_t elements[IRON_GPU_MAX_VERTEX_ELEMENTS];
	int size;
} iron_gpu_vertex_structure_t;

static inline int iron_gpu_vertex_data_size(iron_gpu_vertex_data_t data) {
	switch (data) {
	case IRON_GPU_VERTEX_DATA_F32_1X:
		return 1 * 4;
	case IRON_GPU_VERTEX_DATA_F32_2X:
		return 2 * 4;
	case IRON_GPU_VERTEX_DATA_F32_3X:
		return 3 * 4;
	case IRON_GPU_VERTEX_DATA_F32_4X:
		return 4 * 4;
	case IRON_GPU_VERTEX_DATA_U8_4X_NORM:
		return 4 * 1;
	case IRON_GPU_VERTEX_DATA_I16_2X_NORM:
		return 2 * 2;
	case IRON_GPU_VERTEX_DATA_I16_4X_NORM:
		return 4 * 2;
	}
}

static inline int iron_gpu_vertex_struct_size(iron_gpu_vertex_structure_t *s) {
	int size = 0;
	for (int i = 0; i < s->size; ++i) {
		size += iron_gpu_vertex_data_size(s->elements[i].data);
	}
	return size;
}

void iron_gpu_vertex_structure_init(iron_gpu_vertex_structure_t *structure);
void iron_gpu_vertex_structure_add(iron_gpu_vertex_structure_t *structure, const char *name, iron_gpu_vertex_data_t data);

void iron_gpu_texture_init(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format);
void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format);
void iron_gpu_texture_init_non_sampled_access(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format);
void iron_gpu_texture_destroy(iron_gpu_texture_t *texture);
void iron_gpu_texture_generate_mipmaps(iron_gpu_texture_t *texture, int levels);
void iron_gpu_texture_set_mipmap(iron_gpu_texture_t *texture, iron_gpu_texture_t *mipmap, int level);
int iron_gpu_texture_stride(iron_gpu_texture_t *texture);

struct iron_gpu_texture_unit;

void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits);
void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits);
void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *target, iron_gpu_texture_t *source);

typedef struct iron_gpu_vertex_buffer {
	VertexBuffer5Impl impl;
} iron_gpu_vertex_buffer_t;

typedef struct {
	int myCount;
	iron_gpu_vertex_buffer_t _buffer[2];
	int _currentIndex;
	int _multiple;
} gpu_vertex_buffer_impl_t;

typedef struct gpu_vertex_buffer {
	gpu_vertex_buffer_impl_t impl;
} gpu_vertex_buffer_t;

void iron_gpu_vertex_buffer_init(iron_gpu_vertex_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, bool gpu_memory);
void iron_gpu_vertex_buffer_destroy(iron_gpu_vertex_buffer_t *buffer);
float *iron_gpu_vertex_buffer_lock_all(iron_gpu_vertex_buffer_t *buffer);
float *iron_gpu_vertex_buffer_lock(iron_gpu_vertex_buffer_t *buffer, int start, int count);
void iron_gpu_vertex_buffer_unlock_all(iron_gpu_vertex_buffer_t *buffer);
void iron_gpu_vertex_buffer_unlock(iron_gpu_vertex_buffer_t *buffer, int count);
int iron_gpu_vertex_buffer_count(iron_gpu_vertex_buffer_t *buffer);
int iron_gpu_vertex_buffer_stride(iron_gpu_vertex_buffer_t *buffer);

int iron_gpu_internal_vertex_buffer_set(iron_gpu_vertex_buffer_t *buffer);

void gpu_vertex_buffer_init(gpu_vertex_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, gpu_usage_t usage);
void gpu_vertex_buffer_destroy(gpu_vertex_buffer_t *buffer);
float *gpu_vertex_buffer_lock_all(gpu_vertex_buffer_t *buffer);
float *gpu_vertex_buffer_lock(gpu_vertex_buffer_t *buffer, int start, int count);
void gpu_vertex_buffer_unlock_all(gpu_vertex_buffer_t *buffer);
void gpu_vertex_buffer_unlock(gpu_vertex_buffer_t *buffer, int count);
int gpu_vertex_buffer_count(gpu_vertex_buffer_t *buffer);
int gpu_vertex_buffer_stride(gpu_vertex_buffer_t *buffer);
void gpu_set_vertex_buffer(gpu_vertex_buffer_t *buffer);

typedef struct iron_gpu_constant_buffer {
	uint8_t *data;
	ConstantBuffer5Impl impl;
} iron_gpu_constant_buffer_t;

void iron_gpu_constant_buffer_init(iron_gpu_constant_buffer_t *buffer, int size);
void iron_gpu_constant_buffer_destroy(iron_gpu_constant_buffer_t *buffer);
void iron_gpu_constant_buffer_lock_all(iron_gpu_constant_buffer_t *buffer);
void iron_gpu_constant_buffer_lock(iron_gpu_constant_buffer_t *buffer, int start, int count);
void iron_gpu_constant_buffer_unlock(iron_gpu_constant_buffer_t *buffer);
int iron_gpu_constant_buffer_size(iron_gpu_constant_buffer_t *buffer);
void iron_gpu_constant_buffer_set_bool(iron_gpu_constant_buffer_t *buffer, int offset, bool value);
void iron_gpu_constant_buffer_set_int(iron_gpu_constant_buffer_t *buffer, int offset, int value);
void iron_gpu_constant_buffer_set_int2(iron_gpu_constant_buffer_t *buffer, int offset, int value1, int value2);
void iron_gpu_constant_buffer_set_int3(iron_gpu_constant_buffer_t *buffer, int offset, int value1, int value2, int value3);
void iron_gpu_constant_buffer_set_int4(iron_gpu_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4);
void iron_gpu_constant_buffer_set_ints(iron_gpu_constant_buffer_t *buffer, int offset, int *values, int count);
void iron_gpu_constant_buffer_set_float(iron_gpu_constant_buffer_t *buffer, int offset, float value);
void iron_gpu_constant_buffer_set_float2(iron_gpu_constant_buffer_t *buffer, int offset, float value1, float value2);
void iron_gpu_constant_buffer_set_float3(iron_gpu_constant_buffer_t *buffer, int offset, float value1, float value2, float value3);
void iron_gpu_constant_buffer_set_float4(iron_gpu_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4);
void iron_gpu_constant_buffer_set_floats(iron_gpu_constant_buffer_t *buffer, int offset, float *values, int count);
void iron_gpu_constant_buffer_set_matrix3(iron_gpu_constant_buffer_t *buffer, int offset, iron_matrix3x3_t *value);
void iron_gpu_constant_buffer_set_matrix4(iron_gpu_constant_buffer_t *buffer, int offset, iron_matrix4x4_t *value);

extern bool iron_gpu_transpose_mat;

typedef struct iron_gpu_index_buffer {
	IndexBuffer5Impl impl;
} iron_gpu_index_buffer_t;

void iron_gpu_index_buffer_init(iron_gpu_index_buffer_t *buffer, int count, bool gpu_memory);
void iron_gpu_index_buffer_destroy(iron_gpu_index_buffer_t *buffer);
void *iron_gpu_index_buffer_lock_all(iron_gpu_index_buffer_t *buffer);
void *iron_gpu_index_buffer_lock(iron_gpu_index_buffer_t *buffer, int start, int count);
void iron_gpu_index_buffer_unlock_all(iron_gpu_index_buffer_t *buffer);
void iron_gpu_index_buffer_unlock(iron_gpu_index_buffer_t *buffer, int count);
int iron_gpu_index_buffer_count(iron_gpu_index_buffer_t *buffer);

typedef enum iron_gpu_shader_type {
	IRON_GPU_SHADER_TYPE_FRAGMENT,
	IRON_GPU_SHADER_TYPE_VERTEX,
	IRON_GPU_SHADER_TYPE_COMPUTE,
	IRON_GPU_SHADER_TYPE_COUNT
} iron_gpu_shader_type_t;

typedef enum {
	IRON_GPU_BLEND_ONE,
	IRON_GPU_BLEND_ZERO,
	IRON_GPU_BLEND_SOURCE_ALPHA,
	IRON_GPU_BLEND_DEST_ALPHA,
	IRON_GPU_BLEND_INV_SOURCE_ALPHA,
	IRON_GPU_BLEND_INV_DEST_ALPHA,
	IRON_GPU_BLEND_SOURCE_COLOR,
	IRON_GPU_BLEND_DEST_COLOR,
	IRON_GPU_BLEND_INV_SOURCE_COLOR,
	IRON_GPU_BLEND_INV_DEST_COLOR,
	IRON_GPU_BLEND_CONSTANT,
	IRON_GPU_BLEND_INV_CONSTANT
} iron_gpu_blending_factor_t;

typedef enum {
	IRON_GPU_BLENDOP_ADD,
	IRON_GPU_BLENDOP_SUBTRACT,
	IRON_GPU_BLENDOP_REVERSE_SUBTRACT,
	IRON_GPU_BLENDOP_MIN,
	IRON_GPU_BLENDOP_MAX
} iron_gpu_blending_operation_t;

typedef enum iron_gpu_cull_mode {
	IRON_GPU_CULL_MODE_CLOCKWISE,
	IRON_GPU_CULL_MODE_COUNTERCLOCKWISE,
	IRON_GPU_CULL_MODE_NEVER
} iron_gpu_cull_mode_t;

typedef enum iron_gpu_compare_mode {
	IRON_GPU_COMPARE_MODE_ALWAYS,
	IRON_GPU_COMPARE_MODE_NEVER,
	IRON_GPU_COMPARE_MODE_EQUAL,
	IRON_GPU_COMPARE_MODE_NOT_EQUAL,
	IRON_GPU_COMPARE_MODE_LESS,
	IRON_GPU_COMPARE_MODE_LESS_EQUAL,
	IRON_GPU_COMPARE_MODE_GREATER,
	IRON_GPU_COMPARE_MODE_GREATER_EQUAL
} iron_gpu_compare_mode_t;

typedef enum iron_gpu_texture_addressing {
	IRON_GPU_TEXTURE_ADDRESSING_REPEAT,
	IRON_GPU_TEXTURE_ADDRESSING_MIRROR,
	IRON_GPU_TEXTURE_ADDRESSING_CLAMP,
	IRON_GPU_TEXTURE_ADDRESSING_BORDER
} iron_gpu_texture_addressing_t;

typedef enum iron_gpu_texture_filter {
	IRON_GPU_TEXTURE_FILTER_POINT,
	IRON_GPU_TEXTURE_FILTER_LINEAR,
	IRON_GPU_TEXTURE_FILTER_ANISOTROPIC
} iron_gpu_texture_filter_t;

typedef enum iron_gpu_mipmap_filter {
	IRON_GPU_MIPMAP_FILTER_NONE,
	IRON_GPU_MIPMAP_FILTER_POINT,
	IRON_GPU_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} iron_gpu_mipmap_filter_t;

typedef struct iron_gpu_constant_location {
	ConstantLocation5Impl impl;
} iron_gpu_constant_location_t;

typedef struct iron_gpu_texture_unit {
	int stages[IRON_GPU_SHADER_TYPE_COUNT];
} iron_gpu_texture_unit_t;

typedef struct iron_gpu_shader {
	Shader5Impl impl;
} iron_gpu_shader_t;

typedef struct iron_gpu_pipeline {
	iron_gpu_vertex_structure_t *input_layout;
	iron_gpu_shader_t *vertex_shader;
	iron_gpu_shader_t *fragment_shader;

	iron_gpu_cull_mode_t cull_mode;
	bool depth_write;
	iron_gpu_compare_mode_t depth_mode;

	// One, Zero deactivates blending
	iron_gpu_blending_factor_t blend_source;
	iron_gpu_blending_factor_t blend_destination;
	iron_gpu_blending_operation_t blend_operation;
	iron_gpu_blending_factor_t alpha_blend_source;
	iron_gpu_blending_factor_t alpha_blend_destination;
	iron_gpu_blending_operation_t alpha_blend_operation;

	bool color_write_mask_red[8]; // Per render target
	bool color_write_mask_green[8];
	bool color_write_mask_blue[8];
	bool color_write_mask_alpha[8];

	iron_image_format_t color_attachment[8];
	int color_attachment_count;
	int depth_attachment_bits;

	PipelineState5Impl impl;
} iron_gpu_pipeline_t;

typedef struct iron_gpu_sampler_options {
	iron_gpu_texture_addressing_t u_addressing;
	iron_gpu_texture_addressing_t v_addressing;

	iron_gpu_texture_filter_t minification_filter;
	iron_gpu_texture_filter_t magnification_filter;
	iron_gpu_mipmap_filter_t mipmap_filter;

} iron_gpu_sampler_options_t;

typedef struct iron_gpu_sampler {
	iron_gpu_sampler_impl_t impl;
} iron_gpu_sampler_t;

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipeline);
void iron_gpu_internal_pipeline_init(iron_gpu_pipeline_t *pipeline);
void iron_gpu_pipeline_destroy(iron_gpu_pipeline_t *pipeline);
void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipeline);
iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(iron_gpu_pipeline_t *pipeline, const char *name);
iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipeline, const char *name);

void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *source, size_t length, iron_gpu_shader_type_t type);
void iron_gpu_shader_destroy(iron_gpu_shader_t *shader);

void iron_gpu_sampler_options_set_defaults(iron_gpu_sampler_options_t *options);
void iron_gpu_sampler_init(iron_gpu_sampler_t *sampler, const iron_gpu_sampler_options_t *options);
void iron_gpu_sampler_destroy(iron_gpu_sampler_t *sampler);

void iron_gpu_internal_pipeline_set_defaults(iron_gpu_pipeline_t *pipeline);

struct iron_gpu_compute_shader;
struct iron_gpu_constant_buffer;
struct iron_gpu_index_buffer;
struct iron_gpu_vertex_buffer;
struct iron_gpu_pipeline;
struct iron_gpu_texture;
struct iron_gpu_texture;

typedef struct iron_gpu_command_list {
	CommandList5Impl impl;
} iron_gpu_command_list_t;

void iron_gpu_command_list_init(iron_gpu_command_list_t *list);
void iron_gpu_command_list_destroy(iron_gpu_command_list_t *list);
void iron_gpu_command_list_begin(iron_gpu_command_list_t *list);
void iron_gpu_command_list_end(iron_gpu_command_list_t *list);
void iron_gpu_command_list_clear(iron_gpu_command_list_t *list, struct iron_gpu_texture *render_target, unsigned flags, unsigned color, float depth);
void iron_gpu_command_list_render_target_to_framebuffer_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_framebuffer_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_texture_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_render_target_to_texture_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_draw_indexed_vertices(iron_gpu_command_list_t *list);
void iron_gpu_command_list_draw_indexed_vertices_from_to(iron_gpu_command_list_t *list, int start, int count);
void iron_gpu_command_list_viewport(iron_gpu_command_list_t *list, int x, int y, int width, int height);
void iron_gpu_command_list_scissor(iron_gpu_command_list_t *list, int x, int y, int width, int height);
void iron_gpu_command_list_disable_scissor(iron_gpu_command_list_t *list);
void iron_gpu_command_list_set_pipeline(iron_gpu_command_list_t *list, struct iron_gpu_pipeline *pipeline);
void iron_gpu_command_list_set_compute_shader(iron_gpu_command_list_t *list, struct iron_gpu_compute_shader *shader);
void iron_gpu_command_list_set_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_vertex_buffer *buffer);
void iron_gpu_command_list_set_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_index_buffer *buffer);
void iron_gpu_command_list_set_render_targets(iron_gpu_command_list_t *list, struct iron_gpu_texture **targets, int count);
void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_index_buffer *buffer);
void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_vertex_buffer *buffer);
void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, struct iron_gpu_texture *texture);
void iron_gpu_command_list_set_vertex_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_constant_buffer *buffer, int offset, size_t size);
void iron_gpu_command_list_set_fragment_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_constant_buffer *buffer, int offset, size_t size);
void iron_gpu_command_list_set_compute_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_constant_buffer *buffer, int offset, size_t size);
void iron_gpu_command_list_execute(iron_gpu_command_list_t *list);
void iron_gpu_command_list_wait_for_execution_to_finish(iron_gpu_command_list_t *list);
void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, struct iron_gpu_texture *render_target, uint8_t *data);
void iron_gpu_command_list_compute(iron_gpu_command_list_t *list, int x, int y, int z);
void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture);
void iron_gpu_command_list_set_texture_from_render_target_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *target);
void iron_gpu_command_list_set_sampler(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_sampler_t *sampler);

void iron_gpu_render_target_get_pixels(iron_gpu_texture_t *render_target, uint8_t *data);

typedef struct iron_gpu_compute_shader {
	iron_gpu_compute_shader_impl impl;
} iron_gpu_compute_shader;

void iron_gpu_compute_shader_init(iron_gpu_compute_shader *shader, void *source, int length);
void iron_gpu_compute_shader_destroy(iron_gpu_compute_shader *shader);
iron_gpu_constant_location_t iron_gpu_compute_shader_get_constant_location(iron_gpu_compute_shader *shader, const char *name);
iron_gpu_texture_unit_t iron_gpu_compute_shader_get_texture_unit(iron_gpu_compute_shader *shader, const char *name);

struct iron_gpu_command_list;
struct iron_gpu_constant_buffer;
struct iron_gpu_index_buffer;
struct iron_gpu_texture;
struct iron_gpu_texture;
struct iron_gpu_vertex_buffer;

typedef struct iron_gpu_raytrace_pipeline {
	struct iron_gpu_constant_buffer *_constant_buffer;
	iron_gpu_raytrace_pipeline_impl_t impl;
} iron_gpu_raytrace_pipeline_t;

typedef struct iron_gpu_raytrace_acceleration_structure {
	iron_gpu_raytrace_acceleration_structure_impl_t impl;
} iron_gpu_raytrace_acceleration_structure_t;

bool iron_gpu_raytrace_supported(void);
void iron_gpu_raytrace_pipeline_init(iron_gpu_raytrace_pipeline_t *pipeline, struct iron_gpu_command_list *command_list,
    void *ray_shader, int ray_shader_size, struct iron_gpu_constant_buffer *constant_buffer);
void iron_gpu_raytrace_pipeline_destroy(iron_gpu_raytrace_pipeline_t *pipeline);

void iron_gpu_raytrace_acceleration_structure_init(iron_gpu_raytrace_acceleration_structure_t *accel);
void iron_gpu_raytrace_acceleration_structure_add(iron_gpu_raytrace_acceleration_structure_t *accel, struct iron_gpu_vertex_buffer *vb, struct iron_gpu_index_buffer *ib, iron_matrix4x4_t transform);
void iron_gpu_raytrace_acceleration_structure_build(iron_gpu_raytrace_acceleration_structure_t *accel, struct iron_gpu_command_list *command_list,
    struct iron_gpu_vertex_buffer *_vb_full, struct iron_gpu_index_buffer *_ib_full);
void iron_gpu_raytrace_acceleration_structure_destroy(iron_gpu_raytrace_acceleration_structure_t *accel);

void iron_gpu_raytrace_set_textures(struct iron_gpu_texture *texpaint0, struct iron_gpu_texture *texpaint1, struct iron_gpu_texture *texpaint2,
    struct iron_gpu_texture *texenv, struct iron_gpu_texture *texsobol, struct iron_gpu_texture *texscramble, struct iron_gpu_texture *texrank);
void iron_gpu_raytrace_set_acceleration_structure(iron_gpu_raytrace_acceleration_structure_t *accel);
void iron_gpu_raytrace_set_pipeline(iron_gpu_raytrace_pipeline_t *pipeline);
void iron_gpu_raytrace_set_target(struct iron_gpu_texture *output);
void iron_gpu_raytrace_dispatch_rays(struct iron_gpu_command_list *command_list);
