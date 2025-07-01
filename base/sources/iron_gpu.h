#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <iron_global.h>
#include <iron_math.h>
#include <iron_gpu.h>
#include <iron_array.h>
#include BACKEND_GPU_H

#define GPU_CLEAR_NONE 0
#define GPU_CLEAR_COLOR 1
#define GPU_CLEAR_DEPTH 2
#define GPU_MAX_VERTEX_ELEMENTS 16
#define GPU_FRAMEBUFFER_COUNT 2

typedef enum gpu_texture_state {
	GPU_TEXTURE_STATE_SHADER_RESOURCE,
	GPU_TEXTURE_STATE_RENDER_TARGET,
	GPU_TEXTURE_STATE_PRESENT
} gpu_texture_state_t;

typedef enum gpu_texture_compression {
	GPU_TEXTURE_COMPRESSION_NONE,
	GPU_TEXTURE_COMPRESSION_DXT5,
	GPU_TEXTURE_COMPRESSION_ASTC
} gpu_texture_compression_t;

typedef enum gpu_texture_format {
	GPU_TEXTURE_FORMAT_RGBA32,
	GPU_TEXTURE_FORMAT_RGBA64,
	GPU_TEXTURE_FORMAT_RGBA128,
	GPU_TEXTURE_FORMAT_R8,
	GPU_TEXTURE_FORMAT_R16,
	GPU_TEXTURE_FORMAT_R32,
	GPU_TEXTURE_FORMAT_D32
} gpu_texture_format_t;

typedef enum gpu_vertex_data {
	GPU_VERTEX_DATA_F32_1X,
	GPU_VERTEX_DATA_F32_2X,
	GPU_VERTEX_DATA_F32_3X,
	GPU_VERTEX_DATA_F32_4X,
	GPU_VERTEX_DATA_I16_2X_NORM,
	GPU_VERTEX_DATA_I16_4X_NORM
} gpu_vertex_data_t;

typedef enum gpu_shader_type {
	GPU_SHADER_TYPE_VERTEX,
	GPU_SHADER_TYPE_FRAGMENT,
	GPU_SHADER_TYPE_COUNT
} gpu_shader_type_t;

typedef enum {
	GPU_BLEND_ONE,
	GPU_BLEND_ZERO,
	GPU_BLEND_SOURCE_ALPHA,
	GPU_BLEND_DEST_ALPHA,
	GPU_BLEND_INV_SOURCE_ALPHA,
	GPU_BLEND_INV_DEST_ALPHA
} gpu_blending_factor_t;

typedef enum {
	GPU_BLENDOP_ADD
} gpu_blending_operation_t;

typedef enum gpu_cull_mode {
	GPU_CULL_MODE_CLOCKWISE,
	GPU_CULL_MODE_COUNTERCLOCKWISE,
	GPU_CULL_MODE_NEVER
} gpu_cull_mode_t;

typedef enum gpu_compare_mode {
	GPU_COMPARE_MODE_ALWAYS,
	GPU_COMPARE_MODE_NEVER,
	GPU_COMPARE_MODE_LESS
} gpu_compare_mode_t;

typedef struct gpu_texture {
	int width;
	int height;
	gpu_texture_format_t format;
	gpu_texture_compression_t compression;
	void *data;
	bool uploaded;
	gpu_texture_state_t state;
	buffer_t *buffer;
	gpu_texture_impl_t impl;
} gpu_texture_t;

typedef struct gpu_buffer {
	int count;
	uint8_t *data;
	gpu_buffer_impl_t impl;
} gpu_buffer_t;

typedef struct gpu_vertex_element {
	const char *name;
	gpu_vertex_data_t data;
} gpu_vertex_element_t;

typedef struct gpu_vertex_structure {
	gpu_vertex_element_t elements[GPU_MAX_VERTEX_ELEMENTS];
	int size;
} gpu_vertex_structure_t;

typedef struct gpu_shader {
	gpu_shader_impl_t impl;
} gpu_shader_t;

typedef struct gpu_pipeline {
	gpu_vertex_structure_t *input_layout;
	gpu_shader_t *vertex_shader;
	gpu_shader_t *fragment_shader;
	gpu_cull_mode_t cull_mode;
	bool depth_write;
	gpu_compare_mode_t depth_mode;
	gpu_blending_factor_t blend_source;
	gpu_blending_factor_t blend_destination;
	gpu_blending_operation_t blend_operation;
	gpu_blending_factor_t alpha_blend_source;
	gpu_blending_factor_t alpha_blend_destination;
	gpu_blending_operation_t alpha_blend_operation;
	bool color_write_mask_red[8];
	bool color_write_mask_green[8];
	bool color_write_mask_blue[8];
	bool color_write_mask_alpha[8];
	gpu_texture_format_t color_attachment[8];
	int color_attachment_count;
	int depth_attachment_bits;
	gpu_pipeline_impl_t impl;
} gpu_pipeline_t;

typedef struct gpu_raytrace_pipeline {
	gpu_buffer_t *_constant_buffer;
	gpu_raytrace_pipeline_impl_t impl;
} gpu_raytrace_pipeline_t;

typedef struct gpu_raytrace_acceleration_structure {
	gpu_raytrace_acceleration_structure_impl_t impl;
} gpu_raytrace_acceleration_structure_t;

int gpu_max_bound_textures(void);
void gpu_begin(gpu_texture_t **targets, int count, gpu_texture_t *depth_buffer, unsigned flags, unsigned color, float depth);
void gpu_begin_internal(gpu_texture_t **targets, int count, gpu_texture_t *depth_buffer, unsigned flags, unsigned color, float depth);
void gpu_end(void);
void gpu_end_internal(void);
void gpu_wait(void);
void gpu_present(void);
void gpu_barrier(gpu_texture_t *render_target, gpu_texture_state_t state_after);
void gpu_create_framebuffers(int depth_buffer_bits);
void gpu_init(int depth_buffer_bits, bool vsync);
void gpu_init_internal(int depth_buffer_bits, bool vsync);
void gpu_destroy(void);
void gpu_draw(void);
void gpu_set_int(int location, int value);
void gpu_set_int2(int location, int value1, int value2);
void gpu_set_int3(int location, int value1, int value2, int value3);
void gpu_set_int4(int location, int value1, int value2, int value3, int value4);
void gpu_set_ints(int location, int *values, int count);
void gpu_set_float(int location, float value);
void gpu_set_float2(int location, float value1, float value2);
void gpu_set_float3(int location, float value1, float value2, float value3);
void gpu_set_float4(int location, float value1, float value2, float value3, float value4);
void gpu_set_floats(int location, f32_array_t *values);
void gpu_set_bool(int location, bool value);
void gpu_set_matrix3(int location, iron_matrix3x3_t value);
void gpu_set_matrix4(int location, iron_matrix4x4_t value);

void gpu_vertex_structure_add(gpu_vertex_structure_t *structure, const char *name, gpu_vertex_data_t data);
void gpu_texture_init(gpu_texture_t *texture, int width, int height, gpu_texture_format_t format);
void gpu_texture_init_from_bytes(gpu_texture_t *texture, void *data, int width, int height, gpu_texture_format_t format);
void gpu_texture_destroy(gpu_texture_t *texture);
void gpu_texture_generate_mipmaps(gpu_texture_t *texture, int levels);
void gpu_texture_set_mipmap(gpu_texture_t *texture, gpu_texture_t *mipmap, int level);
int gpu_texture_stride(gpu_texture_t *texture);
void gpu_render_target_init(gpu_texture_t *target, int width, int height, gpu_texture_format_t format);
void gpu_render_target_init2(gpu_texture_t *render_target, int width, int height, gpu_texture_format_t format, int framebuffer_index);
void gpu_vertex_buffer_init(gpu_buffer_t *buffer, int count, gpu_vertex_structure_t *structure);
float *gpu_vertex_buffer_lock(gpu_buffer_t *buffer);
void gpu_vertex_buffer_unlock(gpu_buffer_t *buffer);
int gpu_vertex_buffer_count(gpu_buffer_t *buffer);
int gpu_vertex_buffer_stride(gpu_buffer_t *buffer);
void gpu_constant_buffer_init(gpu_buffer_t *buffer, int size);
void gpu_constant_buffer_destroy(gpu_buffer_t *buffer);
void gpu_constant_buffer_lock(gpu_buffer_t *buffer, int start, int count);
void gpu_constant_buffer_unlock(gpu_buffer_t *buffer);
int gpu_constant_buffer_size(gpu_buffer_t *buffer);
void gpu_index_buffer_init(gpu_buffer_t *buffer, int count);
void gpu_buffer_destroy(gpu_buffer_t *buffer);
void *gpu_index_buffer_lock(gpu_buffer_t *buffer);
void gpu_index_buffer_unlock(gpu_buffer_t *buffer);
int gpu_index_buffer_count(gpu_buffer_t *buffer);

void gpu_pipeline_init(gpu_pipeline_t *pipeline);
void gpu_internal_pipeline_init(gpu_pipeline_t *pipeline);
void gpu_pipeline_destroy(gpu_pipeline_t *pipeline);
void gpu_pipeline_compile(gpu_pipeline_t *pipeline);
void gpu_shader_init(gpu_shader_t *shader, const void *source, size_t length, gpu_shader_type_t type);
void gpu_shader_destroy(gpu_shader_t *shader);

void gpu_draw_internal();
void gpu_viewport(int x, int y, int width, int height);
void gpu_scissor(int x, int y, int width, int height);
void gpu_disable_scissor();
void gpu_set_pipeline(gpu_pipeline_t *pipeline);
void gpu_set_vertex_buffer(gpu_buffer_t *buffer);
void gpu_set_index_buffer(gpu_buffer_t *buffer);
void gpu_upload_texture(gpu_texture_t *texture);
void gpu_set_constant_buffer(gpu_buffer_t *buffer, int offset, size_t size);
void gpu_get_render_target_pixels(gpu_texture_t *render_target, uint8_t *data);
void gpu_set_texture(int unit, gpu_texture_t *texture);

bool gpu_raytrace_supported(void);
void gpu_raytrace_pipeline_init(gpu_raytrace_pipeline_t *pipeline, void *ray_shader, int ray_shader_size, gpu_buffer_t *constant_buffer);
void gpu_raytrace_pipeline_destroy(gpu_raytrace_pipeline_t *pipeline);
void gpu_raytrace_acceleration_structure_init(gpu_raytrace_acceleration_structure_t *accel);
void gpu_raytrace_acceleration_structure_add(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *vb, gpu_buffer_t *ib, iron_matrix4x4_t transform);
void gpu_raytrace_acceleration_structure_build(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *_vb_full, gpu_buffer_t *_ib_full);
void gpu_raytrace_acceleration_structure_destroy(gpu_raytrace_acceleration_structure_t *accel);
void gpu_raytrace_set_textures(gpu_texture_t *texpaint0, gpu_texture_t *texpaint1, gpu_texture_t *texpaint2, gpu_texture_t *texenv, gpu_texture_t *texsobol, gpu_texture_t *texscramble, gpu_texture_t *texrank);
void gpu_raytrace_set_acceleration_structure(gpu_raytrace_acceleration_structure_t *accel);
void gpu_raytrace_set_pipeline(gpu_raytrace_pipeline_t *pipeline);
void gpu_raytrace_set_target(gpu_texture_t *output);
void gpu_raytrace_dispatch_rays();

static inline int gpu_vertex_data_size(gpu_vertex_data_t data) {
	switch (data) {
	case GPU_VERTEX_DATA_F32_1X:
		return 1 * 4;
	case GPU_VERTEX_DATA_F32_2X:
		return 2 * 4;
	case GPU_VERTEX_DATA_F32_3X:
		return 3 * 4;
	case GPU_VERTEX_DATA_F32_4X:
		return 4 * 4;
	case GPU_VERTEX_DATA_I16_2X_NORM:
		return 2 * 2;
	case GPU_VERTEX_DATA_I16_4X_NORM:
		return 4 * 2;
	}
}

static inline int gpu_vertex_struct_size(gpu_vertex_structure_t *s) {
	int size = 0;
	for (int i = 0; i < s->size; ++i) {
		size += gpu_vertex_data_size(s->elements[i].data);
	}
	return size;
}

extern bool gpu_transpose_mat;
extern bool gpu_in_use;
extern gpu_texture_t *current_render_targets[8];
extern int current_render_targets_count;
extern int constant_buffer_index;
extern gpu_texture_t framebuffers[GPU_FRAMEBUFFER_COUNT];
extern gpu_texture_t framebuffer_depth;
extern int framebuffer_index;
