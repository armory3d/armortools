#include "iron_gpu.h"
#include <iron_system.h>

#define CONSTANT_BUFFER_SIZE 256
#define CONSTANT_BUFFER_MULTIPLE 2048

static gpu_buffer_t constant_buffer;
static bool gpu_thrown = false;

int constant_buffer_index = 0;
bool gpu_in_use = false;
gpu_texture_t *current_render_targets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int current_render_targets_count = 0;
gpu_texture_t *current_depth_buffer = NULL;
gpu_texture_t framebuffers[GPU_FRAMEBUFFER_COUNT];
gpu_texture_t framebuffer_depth;
int framebuffer_index = 0;

void gpu_init(int depth_buffer_bits, bool vsync) {
	gpu_init_internal(depth_buffer_bits, vsync);
	gpu_constant_buffer_init(&constant_buffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLE);
	gpu_constant_buffer_lock(&constant_buffer, 0, CONSTANT_BUFFER_SIZE);
}

void gpu_begin(gpu_texture_t **targets, int count, gpu_texture_t *depth_buffer, unsigned flags, unsigned color, float depth) {
	if (gpu_in_use && !gpu_thrown) {
		gpu_thrown = true;
		iron_log("End before you begin");
	}
	gpu_in_use = true;

	if (current_render_targets_count > 0 && current_render_targets[0] != &framebuffers[framebuffer_index]) {
		for (int i = 0; i < current_render_targets_count; ++i) {
			gpu_barrier(current_render_targets[i], GPU_TEXTURE_STATE_SHADER_RESOURCE);
		}
	}
	if (current_depth_buffer != NULL) {
		gpu_barrier(current_depth_buffer, GPU_TEXTURE_STATE_SHADER_RESOURCE);
	}

	if (targets == NULL) {
		current_render_targets[0] = &framebuffers[framebuffer_index];
		current_render_targets_count = 1;
	}
	else {
		for (int i = 0; i < count; ++i) {
			current_render_targets[i] = targets[i];
		}
		current_render_targets_count = count;
	}
	current_depth_buffer = depth_buffer;

	for (int i = 0; i < current_render_targets_count; ++i) {
		gpu_barrier(current_render_targets[i], GPU_TEXTURE_STATE_RENDER_TARGET);
	}
	if (current_depth_buffer != NULL) {
		gpu_barrier(current_depth_buffer, GPU_TEXTURE_STATE_RENDER_TARGET_DEPTH);
	}

	gpu_begin_internal(targets, count, depth_buffer, flags, color, depth);
}

void gpu_draw() {
	gpu_constant_buffer_unlock(&constant_buffer);
	gpu_set_constant_buffer(&constant_buffer, constant_buffer_index * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	gpu_draw_internal();
	++constant_buffer_index;
	if (constant_buffer_index >= CONSTANT_BUFFER_MULTIPLE) {
		constant_buffer_index = 0;
		// gpu_wait();
	}
	gpu_constant_buffer_lock(&constant_buffer, constant_buffer_index * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
}

void gpu_end() {
	if (!gpu_in_use && !gpu_thrown) {
		gpu_thrown = true;
		iron_log("Begin before you end");
	}
	gpu_in_use = false;

	gpu_end_internal();
}

void gpu_resize(int width, int height) {
	if (width == 0 || height == 0) {
		return;
	}
	if (width == framebuffers[0].width && height == framebuffers[0].height) {
		return;
	}
	gpu_resize_internal(width, height);
}

void gpu_set_int(int location, int value) {
	int *ints = (int *)(&constant_buffer.data[location]);
	ints[0] = value;
}

void gpu_set_int2(int location, int value1, int value2) {
	int *ints = (int *)(&constant_buffer.data[location]);
	ints[0] = value1;
	ints[1] = value2;
}

void gpu_set_int3(int location, int value1, int value2, int value3) {
	int *ints = (int *)(&constant_buffer.data[location]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

void gpu_set_int4(int location, int value1, int value2, int value3, int value4) {
	int *ints = (int *)(&constant_buffer.data[location]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

void gpu_set_ints(int location, int *values, int count) {
	int *ints = (int *)(&constant_buffer.data[location]);
	for (int i = 0; i < count; ++i) {
		ints[i] = values[i];
	}
}

void gpu_set_float(int location, float value) {
	float *floats = (float *)(&constant_buffer.data[location]);
	floats[0] = value;
}

void gpu_set_float2(int location, float value1, float value2) {
	float *floats = (float *)(&constant_buffer.data[location]);
	floats[0] = value1;
	floats[1] = value2;
}

void gpu_set_float3(int location, float value1, float value2, float value3) {
	float *floats = (float *)(&constant_buffer.data[location]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void gpu_set_float4(int location, float value1, float value2, float value3, float value4) {
	float *floats = (float *)(&constant_buffer.data[location]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

void gpu_set_floats(int location, f32_array_t *values) {
	float *floats = (float *)(&constant_buffer.data[location]);
	for (int i = 0; i < values->length; ++i) {
		floats[i] = values->buffer[i];
	}
}

void gpu_set_bool(int location, bool value) {
	int *ints = (int *)(&constant_buffer.data[location]);
	ints[0] = value ? 1 : 0;
}

static void gpu_internal_set_matrix3(int offset, iron_matrix3x3_t *value) {
	float *floats = (float *)(&constant_buffer.data[offset]);
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = iron_matrix3x3_get(value, x, y);
		}
	}
}

static void gpu_internal_set_matrix4(int offset, iron_matrix4x4_t *value) {
	float *floats = (float *)(&constant_buffer.data[offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = iron_matrix4x4_get(value, x, y);
		}
	}
}

void gpu_set_matrix3(int location, iron_matrix3x3_t value) {
	if (gpu_transpose_mat) {
		iron_matrix3x3_t m = value;
		iron_matrix3x3_transpose(&m);
		gpu_internal_set_matrix3(location, &m);
	}
	else {
		gpu_internal_set_matrix3(location, &value);
	}
}

void gpu_set_matrix4(int location, iron_matrix4x4_t value) {
	if (gpu_transpose_mat) {
		iron_matrix4x4_t m = value;
		iron_matrix4x4_transpose(&m);
		gpu_internal_set_matrix4(location, &m);
	}
	else {
		gpu_internal_set_matrix4(location, &value);
	}
}

void gpu_vertex_structure_add(gpu_vertex_structure_t *structure, const char *name, gpu_vertex_data_t data) {
	structure->elements[structure->size].name = name;
	structure->elements[structure->size].data = data;
	structure->size++;
}

void gpu_internal_pipeline_init(gpu_pipeline_t *pipe) {
	pipe->input_layout = NULL;
	pipe->vertex_shader = NULL;
	pipe->fragment_shader = NULL;
	pipe->cull_mode = GPU_CULL_MODE_NEVER;
	pipe->depth_write = false;
	pipe->depth_mode = GPU_COMPARE_MODE_ALWAYS;
	pipe->blend_source = GPU_BLEND_ONE;
	pipe->blend_destination = GPU_BLEND_ZERO;
	pipe->blend_operation = GPU_BLENDOP_ADD;
	pipe->alpha_blend_source = GPU_BLEND_ONE;
	pipe->alpha_blend_destination = GPU_BLEND_ZERO;
	pipe->alpha_blend_operation = GPU_BLENDOP_ADD;
	for (int i = 0; i < 8; ++i) {
		pipe->color_write_mask_red[i] = true;
		pipe->color_write_mask_green[i] = true;
		pipe->color_write_mask_blue[i] = true;
		pipe->color_write_mask_alpha[i] = true;
		pipe->color_attachment[i] = GPU_TEXTURE_FORMAT_RGBA32;
	}
	pipe->color_attachment_count = 1;
	pipe->depth_attachment_bits = 0;
}

void gpu_create_framebuffers(int depth_buffer_bits) {
	for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
		gpu_render_target_init2(&framebuffers[i], iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_RGBA32, i);
	}
	if (depth_buffer_bits > 0) {
		gpu_render_target_init(&framebuffer_depth, iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_D32);
	}
	else {
		framebuffer_depth.width = framebuffer_depth.height = 0;
	}
}
