#include "iron_gpu.h"
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include "iron_system.h"
#include "iron_math.h"
#include "iron_file.h"

#define CONSTANT_BUFFER_SIZE 256
#define CONSTANT_BUFFER_MULTIPLE 2048
#define FRAMEBUFFER_COUNT 2

iron_gpu_command_list_t gpu_command_list;
static iron_gpu_buffer_t constant_buffer;
int constant_buffer_index = 0;
static int framebuffer_index;
static iron_gpu_texture_t framebuffers[FRAMEBUFFER_COUNT];
static iron_gpu_texture_t *render_targets[8];
static bool window_resized = false;

void gpu_internal_resize(int width, int height) {
	window_resized = true;
}

void gpu_internal_init_window(int depth_buffer_bits, bool vsync) {
	iron_gpu_internal_init_window(depth_buffer_bits, vsync);
	iron_gpu_command_list_init(&gpu_command_list);
	framebuffer_index = -1;
	for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
		iron_gpu_render_target_init_framebuffer(&framebuffers[i], iron_window_width(), iron_window_height(), IRON_IMAGE_FORMAT_RGBA32, depth_buffer_bits);
	}
	iron_gpu_constant_buffer_init(&constant_buffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLE);
}

void gpu_draw(void) {
	iron_gpu_constant_buffer_unlock(&constant_buffer);
	iron_gpu_command_list_set_constant_buffer(&gpu_command_list, &constant_buffer, constant_buffer_index * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	iron_gpu_command_list_draw(&gpu_command_list);
	++constant_buffer_index;

	// if (constant_buffer_index == CONSTANT_BUFFER_MULTIPLE) {
	// 	iron_gpu_command_list_end(&gpu_command_list);
	// 	iron_gpu_command_list_wait(&gpu_command_list);
	// 	iron_gpu_command_list_begin(&gpu_command_list);
	// 	constant_buffer_index = 0;
	// }

	iron_gpu_constant_buffer_lock(&constant_buffer, constant_buffer_index * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
}

void gpu_begin() {
	constant_buffer_index = 0;
	iron_gpu_constant_buffer_lock(&constant_buffer, constant_buffer_index * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	framebuffer_index = (framebuffer_index + 1) % FRAMEBUFFER_COUNT;

	// if (window_resized) {
	// 	for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
	// 		iron_gpu_texture_destroy(&framebuffers[i]);
	// 	}
	// 	framebuffer_index = 0;
	// }

	iron_gpu_begin(&framebuffers[framebuffer_index]);

	// if (window_resized) {
	// 	for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
	// 		iron_gpu_render_target_init_framebuffer(&framebuffers[i], iron_window_width(), iron_window_height(), IRON_IMAGE_FORMAT_RGBA32, 0);
	// 	}
	// 	window_resized = false;
	// }

	render_targets[0] = NULL;
	iron_gpu_command_list_begin(&gpu_command_list);
	iron_gpu_command_list_framebuffer_to_render_target_barrier(&gpu_command_list, &framebuffers[framebuffer_index]);
	gpu_set_render_targets(NULL, 0, IRON_GPU_CLEAR_NONE, 0, 0);
}

void gpu_end() {
	iron_gpu_constant_buffer_unlock(&constant_buffer);
	iron_gpu_command_list_render_target_to_framebuffer_barrier(&gpu_command_list, &framebuffers[framebuffer_index]);
	iron_gpu_command_list_end(&gpu_command_list);
	iron_gpu_command_list_wait(&gpu_command_list);
	iron_gpu_end();
}

void gpu_set_render_targets(iron_gpu_texture_t **targets, int count, unsigned flags, unsigned color, float depth) {
	iron_gpu_texture_t *_targets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	if (targets == NULL) {
		_targets[0] = &framebuffers[framebuffer_index];
		targets = _targets;
		count = 1;
	}
	for (int i = 0; i < count; ++i) {
		render_targets[i] = targets[i];
		if (render_targets[i]->state != IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
			iron_gpu_command_list_texture_to_render_target_barrier(&gpu_command_list, render_targets[i]);
			render_targets[i]->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
		}
	}
	iron_gpu_command_list_set_render_targets(&gpu_command_list, render_targets, count, flags, color, depth);
}

void gpu_viewport(int x, int y, int width, int height) {
	iron_gpu_command_list_viewport(&gpu_command_list, x, y, width, height);
}

void gpu_scissor(int x, int y, int width, int height) {
	iron_gpu_command_list_scissor(&gpu_command_list, x, y, width, height);
}

void gpu_disable_scissor(void) {
	iron_gpu_command_list_disable_scissor(&gpu_command_list);
}

void gpu_set_int(iron_gpu_constant_location_t *location, int value) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	ints[0] = value;
}

void gpu_set_int2(iron_gpu_constant_location_t *location, int value1, int value2) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	ints[0] = value1;
	ints[1] = value2;
}

void gpu_set_int3(iron_gpu_constant_location_t *location, int value1, int value2, int value3) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

void gpu_set_int4(iron_gpu_constant_location_t *location, int value1, int value2, int value3, int value4) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

void gpu_set_ints(iron_gpu_constant_location_t *location, int *values, int count) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	for (int i = 0; i < count; ++i) {
		ints[i] = values[i];
	}
}

void gpu_set_float(iron_gpu_constant_location_t *location, float value) {
	float *floats = (float *)(&constant_buffer.data[location->impl.vertexOffset]);
	floats[0] = value;
}

void gpu_set_float2(iron_gpu_constant_location_t *location, float value1, float value2) {
	float *floats = (float *)(&constant_buffer.data[location->impl.vertexOffset]);
	floats[0] = value1;
	floats[1] = value2;
}

void gpu_set_float3(iron_gpu_constant_location_t *location, float value1, float value2, float value3) {
	float *floats = (float *)(&constant_buffer.data[location->impl.vertexOffset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void gpu_set_float4(iron_gpu_constant_location_t *location, float value1, float value2, float value3, float value4) {
	float *floats = (float *)(&constant_buffer.data[location->impl.vertexOffset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

void gpu_set_floats(iron_gpu_constant_location_t *location, f32_array_t *values) {
	float *floats = (float *)(&constant_buffer.data[location->impl.vertexOffset]);
	for (int i = 0; i < values->length; ++i) {
		floats[i] = values->buffer[i];
	}
}

void gpu_set_bool(iron_gpu_constant_location_t *location, bool value) {
	int *ints = (int *)(&constant_buffer.data[location->impl.vertexOffset]);
	ints[0] = value ? 1 : 0;
}

static void iron_internal_set_matrix3(uint8_t *constants, int offset, iron_matrix3x3_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = iron_matrix3x3_get(value, x, y);
		}
	}
}

static void iron_internal_set_matrix4(uint8_t *constants, int offset, iron_matrix4x4_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = iron_matrix4x4_get(value, x, y);
		}
	}
}

void gpu_set_matrix3(iron_gpu_constant_location_t *location, iron_matrix3x3_t value) {
	if (iron_gpu_transpose_mat) {
		iron_matrix3x3_t m = value;
		iron_matrix3x3_transpose(&m);
		iron_internal_set_matrix3(constant_buffer.data, location->impl.vertexOffset, &m);
	}
	else {
		iron_internal_set_matrix3(constant_buffer.data, location->impl.vertexOffset, &value);
	}
}

void gpu_set_matrix4(iron_gpu_constant_location_t *location, iron_matrix4x4_t value) {
	if (iron_gpu_transpose_mat) {
		iron_matrix4x4_t m = value;
		iron_matrix4x4_transpose(&m);
		iron_internal_set_matrix4(constant_buffer.data, location->impl.vertexOffset, &m);
	}
	else {
		iron_internal_set_matrix4(constant_buffer.data, location->impl.vertexOffset, &value);
	}
}

void gpu_set_vertex_buffer(iron_gpu_buffer_t *buffer) {
	iron_gpu_command_list_set_vertex_buffer(&gpu_command_list, buffer);
}

void gpu_set_index_buffer(iron_gpu_buffer_t *buffer) {
	iron_gpu_command_list_set_index_buffer(&gpu_command_list, buffer);
}

void gpu_set_pipeline(iron_gpu_pipeline_t *pipeline) {
	iron_gpu_command_list_set_pipeline(&gpu_command_list, pipeline);
}

void gpu_set_texture(iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *render_target) {
	if (!render_target->_uploaded) {
		iron_gpu_command_list_upload_texture(&gpu_command_list, render_target);
		render_target->_uploaded = true;
	}
	if (render_target->state != IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		iron_gpu_command_list_render_target_to_texture_barrier(&gpu_command_list, render_target);
		render_target->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}
	iron_gpu_command_list_set_texture(&gpu_command_list, *unit, render_target);
}

void gpu_set_texture_depth(iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *render_target) {
	if (render_target->state != IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		iron_gpu_command_list_render_target_to_texture_barrier(&gpu_command_list, render_target);
		render_target->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}
	iron_gpu_command_list_set_texture_from_render_target_depth(&gpu_command_list, *unit, render_target);
}

void gpu_render_target_get_pixels(iron_gpu_texture_t *render_target, uint8_t *data) {
	iron_gpu_command_list_get_render_target_pixels(&gpu_command_list, render_target, data);
}

void gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer) {
	iron_gpu_index_buffer_unlock(buffer);
	iron_gpu_command_list_upload_index_buffer(&gpu_command_list, buffer);
}

void gpu_vertex_structure_add(iron_gpu_vertex_structure_t *structure, const char *name, iron_gpu_vertex_data_t data) {
	structure->elements[structure->size].name = name;
	structure->elements[structure->size].data = data;
	structure->size++;
}

void gpu_internal_pipeline_init(iron_gpu_pipeline_t *pipe) {
	pipe->input_layout = NULL;
	pipe->vertex_shader = NULL;
	pipe->fragment_shader = NULL;
	pipe->cull_mode = IRON_GPU_CULL_MODE_NEVER;
	pipe->depth_write = false;
	pipe->depth_mode = IRON_GPU_COMPARE_MODE_ALWAYS;
	pipe->blend_source = IRON_GPU_BLEND_ONE;
	pipe->blend_destination = IRON_GPU_BLEND_ZERO;
	pipe->blend_operation = IRON_GPU_BLENDOP_ADD;
	pipe->alpha_blend_source = IRON_GPU_BLEND_ONE;
	pipe->alpha_blend_destination = IRON_GPU_BLEND_ZERO;
	pipe->alpha_blend_operation = IRON_GPU_BLENDOP_ADD;
	for (int i = 0; i < 8; ++i) {
		pipe->color_write_mask_red[i] = true;
		pipe->color_write_mask_green[i] = true;
		pipe->color_write_mask_blue[i] = true;
		pipe->color_write_mask_alpha[i] = true;
		pipe->color_attachment[i] = IRON_IMAGE_FORMAT_RGBA32;
	}
	pipe->color_attachment_count = 1;
	pipe->depth_attachment_bits = 0;
}
