#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <iron_global.h>
#include <iron_math.h>
#include <iron_gpu.h>
#include <iron_array.h>
#include BACKEND_GPU_H

#define IRON_GPU_CLEAR_NONE 0
#define IRON_GPU_CLEAR_COLOR 1
#define IRON_GPU_CLEAR_DEPTH 2
#define IRON_GPU_MAX_VERTEX_ELEMENTS 16

typedef enum iron_internal_render_target_state {
	IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET,
	IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE
} iron_internal_render_target_state_t;

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
	IRON_IMAGE_FORMAT_R32
} iron_image_format_t;

typedef enum {
    GPU_USAGE_STATIC,
    GPU_USAGE_DYNAMIC,
    GPU_USAGE_READABLE
} gpu_usage_t;

typedef enum iron_gpu_vertex_data {
	IRON_GPU_VERTEX_DATA_F32_1X,
	IRON_GPU_VERTEX_DATA_F32_2X,
	IRON_GPU_VERTEX_DATA_F32_3X,
	IRON_GPU_VERTEX_DATA_F32_4X,
	IRON_GPU_VERTEX_DATA_I16_2X_NORM,
	IRON_GPU_VERTEX_DATA_I16_4X_NORM,
} iron_gpu_vertex_data_t;

typedef enum iron_gpu_shader_type {
	IRON_GPU_SHADER_TYPE_VERTEX,
	IRON_GPU_SHADER_TYPE_FRAGMENT,
	IRON_GPU_SHADER_TYPE_COUNT
} iron_gpu_shader_type_t;

typedef enum {
	IRON_GPU_BLEND_ONE,
	IRON_GPU_BLEND_ZERO,
	IRON_GPU_BLEND_SOURCE_ALPHA,
	IRON_GPU_BLEND_DEST_ALPHA,
	IRON_GPU_BLEND_INV_SOURCE_ALPHA,
	IRON_GPU_BLEND_INV_DEST_ALPHA
} iron_gpu_blending_factor_t;

typedef enum {
	IRON_GPU_BLENDOP_ADD
} iron_gpu_blending_operation_t;

typedef enum iron_gpu_cull_mode {
	IRON_GPU_CULL_MODE_CLOCKWISE,
	IRON_GPU_CULL_MODE_COUNTERCLOCKWISE,
	IRON_GPU_CULL_MODE_NEVER
} iron_gpu_cull_mode_t;

typedef enum iron_gpu_compare_mode {
	IRON_GPU_COMPARE_MODE_ALWAYS,
	IRON_GPU_COMPARE_MODE_NEVER,
	IRON_GPU_COMPARE_MODE_LESS
} iron_gpu_compare_mode_t;

typedef struct iron_gpu_texture {
	int width;
	int height;
	iron_image_format_t format;
	iron_image_compression_t compression;
	void *data;
	bool _uploaded;
	int framebuffer_index;
	bool isDepthAttachment;
	iron_internal_render_target_state_t state;
	buffer_t *buffer;
	gpu_texture_impl_t impl;
} iron_gpu_texture_t;

typedef struct iron_gpu_buffer {
	int myCount;
	uint8_t *data;
	gpu_buffer_impl_t impl;
} iron_gpu_buffer_t;

typedef struct iron_gpu_vertex_element {
	const char *name;
	iron_gpu_vertex_data_t data;
} iron_gpu_vertex_element_t;

typedef struct iron_gpu_vertex_structure {
	iron_gpu_vertex_element_t elements[IRON_GPU_MAX_VERTEX_ELEMENTS];
	int size;
} iron_gpu_vertex_structure_t;

typedef struct iron_gpu_constant_location {
	gpu_constant_location_impl_t impl;
} iron_gpu_constant_location_t;

typedef struct iron_gpu_texture_unit {
	int offset;
} iron_gpu_texture_unit_t;

typedef struct iron_gpu_shader {
	gpu_shader_impl_t impl;
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

	gpu_pipeline_impl_t impl;
} iron_gpu_pipeline_t;

typedef struct iron_gpu_command_list {
	gpu_command_list_impl_t impl;
} iron_gpu_command_list_t;

typedef struct iron_gpu_raytrace_pipeline {
	iron_gpu_buffer_t *_constant_buffer;
	gpu_raytrace_pipeline_impl_t impl;
} iron_gpu_raytrace_pipeline_t;

typedef struct iron_gpu_raytrace_acceleration_structure {
	gpu_raytrace_acceleration_structure_impl_t impl;
} iron_gpu_raytrace_acceleration_structure_t;

struct iron_gpu_pipeline;
struct iron_gpu_texture;
struct iron_gpu_texture_unit;
struct iron_gpu_constant_location;
struct iron_gpu_buffer;

int iron_gpu_max_bound_textures(void);
void iron_gpu_begin(iron_gpu_texture_t *renderTarget);
void iron_gpu_end(void);
void iron_gpu_internal_init(void);
void iron_gpu_internal_init_window(int depth_buffer_bits, bool vsync);
void iron_gpu_internal_destroy(void);
void gpu_begin();
void gpu_end();
void gpu_viewport(int x, int y, int width, int height);
void gpu_scissor(int x, int y, int width, int height);
void gpu_disable_scissor(void);
void gpu_draw(void);
void gpu_set_pipeline(struct iron_gpu_pipeline *pipeline);
void gpu_set_int(struct iron_gpu_constant_location *location, int value);
void gpu_set_int2(struct iron_gpu_constant_location *location, int value1, int value2);
void gpu_set_int3(struct iron_gpu_constant_location *location, int value1, int value2, int value3);
void gpu_set_int4(struct iron_gpu_constant_location *location, int value1, int value2, int value3, int value4);
void gpu_set_ints(struct iron_gpu_constant_location *location, int *values, int count);
void gpu_set_float(struct iron_gpu_constant_location *location, float value);
void gpu_set_float2(struct iron_gpu_constant_location *location, float value1, float value2);
void gpu_set_float3(struct iron_gpu_constant_location *location, float value1, float value2, float value3);
void gpu_set_float4(struct iron_gpu_constant_location *location, float value1, float value2, float value3, float value4);
void gpu_set_floats(struct iron_gpu_constant_location *location, f32_array_t *values);
void gpu_set_bool(struct iron_gpu_constant_location *location, bool value);
void gpu_set_matrix3(struct iron_gpu_constant_location *location, iron_matrix3x3_t value);
void gpu_set_matrix4(struct iron_gpu_constant_location *location, iron_matrix4x4_t value);
void gpu_set_render_targets(struct iron_gpu_texture **targets, int count, unsigned flags, unsigned color, float depth);
void gpu_set_texture(struct iron_gpu_texture_unit *unit, struct iron_gpu_texture *texture);
void gpu_set_index_buffer(iron_gpu_buffer_t *buffer);
void gpu_internal_init_window(int depth_buffer_bits, bool vsync);
void gpu_set_texture_depth(struct iron_gpu_texture_unit *unit, struct iron_gpu_texture *renderTarget);
void gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer);

void gpu_vertex_structure_add(iron_gpu_vertex_structure_t *structure, const char *name, iron_gpu_vertex_data_t data);
void iron_gpu_texture_init(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format);
void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format);
void iron_gpu_texture_destroy(iron_gpu_texture_t *texture);
void iron_gpu_texture_generate_mipmaps(iron_gpu_texture_t *texture, int levels);
void iron_gpu_texture_set_mipmap(iron_gpu_texture_t *texture, iron_gpu_texture_t *mipmap, int level);
int iron_gpu_texture_stride(iron_gpu_texture_t *texture);
void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits);
void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits);
void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *target, iron_gpu_texture_t *source);
void iron_gpu_vertex_buffer_init(iron_gpu_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, bool gpu_memory);
void iron_gpu_vertex_buffer_destroy(iron_gpu_buffer_t *buffer);
float *iron_gpu_vertex_buffer_lock(iron_gpu_buffer_t *buffer);
void iron_gpu_vertex_buffer_unlock(iron_gpu_buffer_t *buffer);
int iron_gpu_vertex_buffer_count(iron_gpu_buffer_t *buffer);
int iron_gpu_vertex_buffer_stride(iron_gpu_buffer_t *buffer);
void gpu_set_vertex_buffer(iron_gpu_buffer_t *buffer);
void iron_gpu_constant_buffer_init(iron_gpu_buffer_t *buffer, int size);
void iron_gpu_constant_buffer_destroy(iron_gpu_buffer_t *buffer);
void iron_gpu_constant_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count);
void iron_gpu_constant_buffer_unlock(iron_gpu_buffer_t *buffer);
int iron_gpu_constant_buffer_size(iron_gpu_buffer_t *buffer);
void iron_gpu_index_buffer_init(iron_gpu_buffer_t *buffer, int count, bool gpu_memory);
void iron_gpu_index_buffer_destroy(iron_gpu_buffer_t *buffer);
void *iron_gpu_index_buffer_lock(iron_gpu_buffer_t *buffer);
void iron_gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer);
int iron_gpu_index_buffer_count(iron_gpu_buffer_t *buffer);

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipeline);
void gpu_internal_pipeline_init(iron_gpu_pipeline_t *pipeline);
void iron_gpu_pipeline_destroy(iron_gpu_pipeline_t *pipeline);
void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipeline);
iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(iron_gpu_pipeline_t *pipeline, const char *name);
iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipeline, const char *name);
void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *source, size_t length, iron_gpu_shader_type_t type);
void iron_gpu_shader_destroy(iron_gpu_shader_t *shader);
void iron_gpu_command_list_init(iron_gpu_command_list_t *list);
void iron_gpu_command_list_destroy(iron_gpu_command_list_t *list);
void iron_gpu_command_list_begin(iron_gpu_command_list_t *list);
void iron_gpu_command_list_end(iron_gpu_command_list_t *list);
void iron_gpu_command_list_render_target_to_framebuffer_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_framebuffer_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_texture_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_render_target_to_texture_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget);
void iron_gpu_command_list_draw(iron_gpu_command_list_t *list);
void iron_gpu_command_list_viewport(iron_gpu_command_list_t *list, int x, int y, int width, int height);
void iron_gpu_command_list_scissor(iron_gpu_command_list_t *list, int x, int y, int width, int height);
void iron_gpu_command_list_disable_scissor(iron_gpu_command_list_t *list);
void iron_gpu_command_list_set_pipeline(iron_gpu_command_list_t *list, struct iron_gpu_pipeline *pipeline);
void iron_gpu_command_list_set_vertex_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer);
void iron_gpu_command_list_set_index_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer);
void iron_gpu_command_list_set_render_targets(iron_gpu_command_list_t *list, struct iron_gpu_texture **targets, int count, unsigned flags, unsigned color, float depth);
void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer);
void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer);
void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, struct iron_gpu_texture *texture);
void iron_gpu_command_list_set_constant_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer, int offset, size_t size);
void iron_gpu_command_list_wait(iron_gpu_command_list_t *list);
void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, struct iron_gpu_texture *render_target, uint8_t *data);
void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture);
void iron_gpu_command_list_set_texture_from_render_target_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *target);
void gpu_render_target_get_pixels(iron_gpu_texture_t *render_target, uint8_t *data);

bool iron_gpu_raytrace_supported(void);
void iron_gpu_raytrace_pipeline_init(iron_gpu_raytrace_pipeline_t *pipeline, struct iron_gpu_command_list *command_list, void *ray_shader, int ray_shader_size, struct iron_gpu_buffer *constant_buffer);
void iron_gpu_raytrace_pipeline_destroy(iron_gpu_raytrace_pipeline_t *pipeline);
void iron_gpu_raytrace_acceleration_structure_init(iron_gpu_raytrace_acceleration_structure_t *accel);
void iron_gpu_raytrace_acceleration_structure_add(iron_gpu_raytrace_acceleration_structure_t *accel, struct iron_gpu_buffer *vb, struct iron_gpu_buffer *ib, iron_matrix4x4_t transform);
void iron_gpu_raytrace_acceleration_structure_build(iron_gpu_raytrace_acceleration_structure_t *accel, struct iron_gpu_command_list *command_list, struct iron_gpu_buffer *_vb_full, struct iron_gpu_buffer *_ib_full);
void iron_gpu_raytrace_acceleration_structure_destroy(iron_gpu_raytrace_acceleration_structure_t *accel);
void iron_gpu_raytrace_set_textures(struct iron_gpu_texture *texpaint0, struct iron_gpu_texture *texpaint1, struct iron_gpu_texture *texpaint2, struct iron_gpu_texture *texenv, struct iron_gpu_texture *texsobol, struct iron_gpu_texture *texscramble, struct iron_gpu_texture *texrank);
void iron_gpu_raytrace_set_acceleration_structure(iron_gpu_raytrace_acceleration_structure_t *accel);
void iron_gpu_raytrace_set_pipeline(iron_gpu_raytrace_pipeline_t *pipeline);
void iron_gpu_raytrace_set_target(struct iron_gpu_texture *output);
void iron_gpu_raytrace_dispatch_rays(struct iron_gpu_command_list *command_list);

extern bool iron_gpu_transpose_mat;

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
