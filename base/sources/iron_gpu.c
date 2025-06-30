#include "iron_gpu.h"

#define CONSTANT_BUFFER_SIZE 256
#define CONSTANT_BUFFER_MULTIPLE 2048
int constant_buffer_index = 0;
static gpu_buffer_t constant_buffer;

void gpu_init(int depth_buffer_bits, bool vsync) {
	gpu_init_internal(depth_buffer_bits, vsync);
	gpu_constant_buffer_init(&constant_buffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLE);
	gpu_constant_buffer_lock(&constant_buffer, 0, CONSTANT_BUFFER_SIZE);
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
		pipe->color_attachment[i] = IRON_IMAGE_FORMAT_RGBA32;
	}
	pipe->color_attachment_count = 1;
	pipe->depth_attachment_bits = 0;
}
