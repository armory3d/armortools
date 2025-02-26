#include "iron_gpu.h"
#include <assert.h>
#include <string.h>
#include <iron_system.h>
#include <iron_math.h>
#include <iron_file.h>
#include <stddef.h>

#define CONSTANT_BUFFER_SIZE 4096
#define CONSTANT_BUFFER_MULTIPLY 100
#define FRAMEBUFFER_COUNT 2
#define MAX_TEXTURES 16

#define MAX_SAMPLERS_PER_STAGE 16

static kinc_g5_sampler_options_t sampler_options[KINC_G5_SHADER_TYPE_COUNT][MAX_SAMPLERS_PER_STAGE];

struct sampler_cache_entry {
	kinc_g5_sampler_options_t options;
	kinc_g5_sampler_t sampler;
};

#define MAX_SAMPLER_CACHE_SIZE 256
static struct sampler_cache_entry sampler_cache[MAX_SAMPLER_CACHE_SIZE];
static int sampler_cache_size = 0;

void kinc_internal_samplers_reset(void);

kinc_g5_command_list_t commandList;
bool waitAfterNextDraw = false;

static kinc_g5_constant_buffer_t vertexConstantBuffer;
static kinc_g5_constant_buffer_t fragmentConstantBuffer;
static kinc_g5_constant_buffer_t computeConstantBuffer;
static int constantBufferIndex = 0;

static struct {
	int currentBuffer;
	kinc_g5_render_target_t framebuffers[FRAMEBUFFER_COUNT];
	kinc_g5_render_target_t *current_render_targets[8];
	int current_render_target_count;
	bool resized;
} windows[1] = {0};

typedef struct render_state {
	kinc_g5_pipeline_t *pipeline;
	kinc_g5_compute_shader *compute_shader;

	kinc_g5_index_buffer_t *index_buffer;
	kinc_g5_vertex_buffer_t *vertex_buffer;

	bool viewport_set;
	int viewport_x;
	int viewport_y;
	int viewport_width;
	int viewport_height;

	bool scissor_set;
	int scissor_x;
	int scissor_y;
	int scissor_width;
	int scissor_height;

	kinc_g5_texture_t *textures[MAX_TEXTURES];
	kinc_g5_texture_unit_t texture_units[MAX_TEXTURES];
	int texture_count;

	kinc_g5_render_target_t *render_targets[MAX_TEXTURES];
	kinc_g5_texture_unit_t render_target_units[MAX_TEXTURES];
	int render_target_count;

	kinc_g5_render_target_t *depth_render_targets[MAX_TEXTURES];
	kinc_g5_texture_unit_t depth_render_target_units[MAX_TEXTURES];
	int depth_render_target_count;

	uint8_t vertex_constant_data[CONSTANT_BUFFER_SIZE];
	uint8_t fragment_constant_data[CONSTANT_BUFFER_SIZE];
	uint8_t compute_constant_data[CONSTANT_BUFFER_SIZE];
} render_state;

static render_state current_state;

bool kinc_g5_texture_unit_equals(kinc_g5_texture_unit_t *unit1, kinc_g5_texture_unit_t *unit2) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (unit1->stages[i] != unit2->stages[i]) {
			return false;
		}
	}
	return true;
}

void kinc_g5_internal_resize(int width, int height) {
	windows[0].resized = true;
}

void kinc_g5_internal_restore_render_target(void) {
	windows[0].current_render_targets[0] = NULL;
	kinc_g5_render_target_t *render_target = &windows[0].framebuffers[windows[0].currentBuffer];
	kinc_g5_command_list_set_render_targets(&commandList, &render_target, 1);
	windows[0].current_render_target_count = 1;
}

void kinc_g4_internal_init_window(int depthBufferBits, bool vsync) {
	kinc_g5_internal_init_window(depthBufferBits, vsync);

	kinc_g5_command_list_init(&commandList);
	windows[0].currentBuffer = -1;
	for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
		kinc_g5_render_target_init_framebuffer(&windows[0].framebuffers[i], kinc_window_width(), kinc_window_height(),
		                                       KINC_IMAGE_FORMAT_RGBA32, depthBufferBits);
	}
	kinc_g5_constant_buffer_init(&vertexConstantBuffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLY);
	kinc_g5_constant_buffer_init(&fragmentConstantBuffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLY);
	kinc_g5_constant_buffer_init(&computeConstantBuffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLY);

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

kinc_g5_sampler_t *kinc_internal_get_current_sampler(int stage, int unit);

void kinc_g5_internal_set_samplers(int count, kinc_g5_texture_unit_t *texture_units) {
	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < KINC_G5_SHADER_TYPE_COUNT; ++j) {
			if (texture_units[i].stages[j] >= 0) {
				kinc_g5_sampler_t *sampler = kinc_internal_get_current_sampler(j, texture_units[i].stages[j]);
				kinc_g5_texture_unit_t unit;
				for (int k = 0; k < KINC_G5_SHADER_TYPE_COUNT; ++k) {
					unit.stages[k] = -1;
				}
				unit.stages[j] = texture_units[i].stages[j];
				kinc_g5_command_list_set_sampler(&commandList, unit, sampler);
			}
		}
	}
}

static void kinc_internal_start_draw(bool compute) {
	if ((constantBufferIndex + 1) >= CONSTANT_BUFFER_MULTIPLY || waitAfterNextDraw) {
		memcpy(current_state.vertex_constant_data, vertexConstantBuffer.data, CONSTANT_BUFFER_SIZE);
		memcpy(current_state.fragment_constant_data, fragmentConstantBuffer.data, CONSTANT_BUFFER_SIZE);
		memcpy(current_state.compute_constant_data, computeConstantBuffer.data, CONSTANT_BUFFER_SIZE);
	}
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);
	kinc_g5_constant_buffer_unlock(&computeConstantBuffer);

	kinc_g5_internal_set_samplers(current_state.texture_count, current_state.texture_units);
	kinc_g5_internal_set_samplers(current_state.render_target_count, current_state.render_target_units);
	kinc_g5_internal_set_samplers(current_state.depth_render_target_count, current_state.depth_render_target_units);

	if (compute) {
		kinc_g5_command_list_set_compute_constant_buffer(&commandList, &computeConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	}
	else {
		kinc_g5_command_list_set_vertex_constant_buffer(&commandList, &vertexConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
		kinc_g5_command_list_set_fragment_constant_buffer(&commandList, &fragmentConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	}
}

static void kinc_internal_end_draw(bool compute) {
	++constantBufferIndex;
	if (constantBufferIndex >= CONSTANT_BUFFER_MULTIPLY || waitAfterNextDraw) {
		kinc_g5_command_list_end(&commandList);
		kinc_g5_command_list_execute(&commandList);
		kinc_g5_command_list_wait_for_execution_to_finish(&commandList);
		kinc_g5_command_list_begin(&commandList);
		if (windows[0].current_render_targets[0] == NULL) {
			kinc_g5_internal_restore_render_target();
		}
		else {
			const int count = windows[0].current_render_target_count;
			kinc_g5_render_target_t *render_targets[16];
			for (int i = 0; i < count; ++i) {
				render_targets[i] = windows[0].current_render_targets[i];
			}
			kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
		}

		if (current_state.pipeline != NULL) {
			kinc_g5_command_list_set_pipeline(&commandList, current_state.pipeline);
		}
		if (current_state.compute_shader != NULL) {
			kinc_g5_command_list_set_compute_shader(&commandList, current_state.compute_shader);
		}
		if (current_state.index_buffer != NULL) {
			kinc_g5_command_list_set_index_buffer(&commandList, current_state.index_buffer);
		}
		if (current_state.vertex_buffer != NULL) {
			kinc_g5_command_list_set_vertex_buffer(&commandList, current_state.vertex_buffer);
		}
		if (current_state.viewport_set) {
			kinc_g5_command_list_viewport(&commandList, current_state.viewport_x, current_state.viewport_y, current_state.viewport_width,
			                              current_state.viewport_height);
		}
		if (current_state.scissor_set) {
			kinc_g5_command_list_scissor(&commandList, current_state.scissor_x, current_state.scissor_y, current_state.scissor_width,
			                             current_state.scissor_height);
		}
		for (int i = 0; i < current_state.texture_count; ++i) {
			kinc_g5_command_list_set_texture(&commandList, current_state.texture_units[i], current_state.textures[i]);
		}
		for (int i = 0; i < current_state.render_target_count; ++i) {
			kinc_g5_command_list_set_texture_from_render_target(&commandList, current_state.render_target_units[i], current_state.render_targets[i]);
		}
		for (int i = 0; i < current_state.depth_render_target_count; ++i) {
			kinc_g5_command_list_set_texture_from_render_target_depth(&commandList, current_state.depth_render_target_units[i],
			                                                          current_state.depth_render_targets[i]);
		}
		constantBufferIndex = 0;
		waitAfterNextDraw = false;

		kinc_g5_constant_buffer_lock(&vertexConstantBuffer, 0, CONSTANT_BUFFER_SIZE);
		kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, 0, CONSTANT_BUFFER_SIZE);
		kinc_g5_constant_buffer_lock(&computeConstantBuffer, 0, CONSTANT_BUFFER_SIZE);

		memcpy(vertexConstantBuffer.data, current_state.vertex_constant_data, CONSTANT_BUFFER_SIZE);
		memcpy(fragmentConstantBuffer.data, current_state.fragment_constant_data, CONSTANT_BUFFER_SIZE);
		memcpy(computeConstantBuffer.data, current_state.compute_constant_data, CONSTANT_BUFFER_SIZE);
	}
	else {
		kinc_g5_constant_buffer_lock(&vertexConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
		kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
		kinc_g5_constant_buffer_lock(&computeConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	}
}

void kinc_g5_draw_indexed_vertices(void) {
	kinc_internal_start_draw(false);
	kinc_g5_command_list_draw_indexed_vertices(&commandList);
	kinc_internal_end_draw(false);
}

void kinc_g5_draw_indexed_vertices_from_to(int start, int count) {
	kinc_internal_start_draw(false);
	kinc_g5_command_list_draw_indexed_vertices_from_to(&commandList, start, count);
	kinc_internal_end_draw(false);
}

void kinc_g5_clear(unsigned flags, unsigned color, float depth) {
	if (windows[0].current_render_target_count > 0) {
		if (windows[0].current_render_targets[0] == NULL) {
			kinc_g5_command_list_clear(&commandList, &windows[0].framebuffers[windows[0].currentBuffer], flags, color, depth);
		}
		else {
			if (windows[0].current_render_targets[0]->state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
				kinc_g5_command_list_texture_to_render_target_barrier(&commandList, windows[0].current_render_targets[0]);
				windows[0].current_render_targets[0]->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
			}
			kinc_g5_command_list_clear(&commandList, windows[0].current_render_targets[0], flags, color, depth);
		}
	}
}

void kinc_g4_begin() {
	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	windows[0].currentBuffer = (windows[0].currentBuffer + 1) % FRAMEBUFFER_COUNT;

	bool resized = windows[0].resized;
	if (resized) {
		for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
			kinc_g5_render_target_destroy(&windows[0].framebuffers[i]);
		}
		windows[0].currentBuffer = 0;
	}

	kinc_g5_begin(&windows[0].framebuffers[windows[0].currentBuffer]);

	if (resized) {
		for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
			kinc_g5_render_target_init_framebuffer(&windows[0].framebuffers[i], kinc_window_width(), kinc_window_height(),
			                                       KINC_IMAGE_FORMAT_RGBA32, 16);
		}
		windows[0].resized = false;
	}

	windows[0].current_render_targets[0] = NULL;
	windows[0].current_render_target_count = 1;

	current_state.pipeline = NULL;
	current_state.compute_shader = NULL;
	current_state.index_buffer = NULL;
	current_state.vertex_buffer = NULL;
	current_state.viewport_set = false;
	current_state.scissor_set = false;
	current_state.texture_count = 0;
	current_state.render_target_count = 0;
	current_state.depth_render_target_count = 0;

	kinc_internal_samplers_reset();

	kinc_g5_command_list_begin(&commandList);

	// Currently we do not necessarily wait at the end of a frame so for now it's kinc_internal_end_draw
	kinc_internal_end_draw(false);

	kinc_g5_command_list_framebuffer_to_render_target_barrier(&commandList, &windows[0].framebuffers[windows[0].currentBuffer]);
	kinc_g4_restore_render_target();
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	current_state.viewport_x = x;
	current_state.viewport_y = y;
	current_state.viewport_width = width;
	current_state.viewport_height = height;
	current_state.viewport_set = true;
	kinc_g5_command_list_viewport(&commandList, x, y, width, height);
}

void kinc_g4_scissor(int x, int y, int width, int height) {
	current_state.scissor_x = x;
	current_state.scissor_y = y;
	current_state.scissor_width = width;
	current_state.scissor_height = height;
	current_state.scissor_set = true;
	kinc_g5_command_list_scissor(&commandList, x, y, width, height);
}

void kinc_g4_disable_scissor(void) {
	current_state.scissor_set = false;
	kinc_g5_command_list_disable_scissor(&commandList);
}

void kinc_g4_end() {
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);
	kinc_g5_constant_buffer_unlock(&computeConstantBuffer);

	kinc_g5_command_list_render_target_to_framebuffer_barrier(&commandList, &windows[0].framebuffers[windows[0].currentBuffer]);
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	kinc_g5_end();

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

void kinc_g4_set_int(kinc_g5_constant_location_t location, int value) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int(&vertexConstantBuffer, location.impl.vertexOffset, value);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int(&fragmentConstantBuffer, location.impl.fragmentOffset, value);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int(&computeConstantBuffer, location.impl.computeOffset, value);
}

void kinc_g4_set_int2(kinc_g5_constant_location_t location, int value1, int value2) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&computeConstantBuffer, location.impl.computeOffset, value1, value2);
}

void kinc_g4_set_int3(kinc_g5_constant_location_t location, int value1, int value2, int value3) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2, value3);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2, value3);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&computeConstantBuffer, location.impl.computeOffset, value1, value2, value3);
}

void kinc_g4_set_int4(kinc_g5_constant_location_t location, int value1, int value2, int value3, int value4) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2, value3, value4);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2, value3, value4);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&computeConstantBuffer, location.impl.computeOffset, value1, value2, value3, value4);
}

void kinc_g4_set_ints(kinc_g5_constant_location_t location, int *values, int count) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&vertexConstantBuffer, location.impl.vertexOffset, values, count);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&fragmentConstantBuffer, location.impl.fragmentOffset, values, count);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&computeConstantBuffer, location.impl.computeOffset, values, count);
}

void kinc_g4_set_float(kinc_g5_constant_location_t location, float value) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float(&vertexConstantBuffer, location.impl.vertexOffset, value);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float(&fragmentConstantBuffer, location.impl.fragmentOffset, value);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float(&computeConstantBuffer, location.impl.computeOffset, value);
}

void kinc_g4_set_float2(kinc_g5_constant_location_t location, float value1, float value2) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&computeConstantBuffer, location.impl.computeOffset, value1, value2);
}

void kinc_g4_set_float3(kinc_g5_constant_location_t location, float value1, float value2, float value3) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2, value3);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2, value3);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&computeConstantBuffer, location.impl.computeOffset, value1, value2, value3);
}

void kinc_g4_set_float4(kinc_g5_constant_location_t location, float value1, float value2, float value3, float value4) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&vertexConstantBuffer, location.impl.vertexOffset, value1, value2, value3, value4);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&fragmentConstantBuffer, location.impl.fragmentOffset, value1, value2, value3, value4);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&computeConstantBuffer, location.impl.computeOffset, value1, value2, value3, value4);
}

void kinc_g4_set_floats(kinc_g5_constant_location_t location, float *values, int count) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&vertexConstantBuffer, location.impl.vertexOffset, values, count);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&fragmentConstantBuffer, location.impl.fragmentOffset, values, count);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&computeConstantBuffer, location.impl.computeOffset, values, count);
}

void kinc_g4_set_bool(kinc_g5_constant_location_t location, bool value) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&vertexConstantBuffer, location.impl.vertexOffset, value);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&fragmentConstantBuffer, location.impl.fragmentOffset, value);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&computeConstantBuffer, location.impl.computeOffset, value);
}

void kinc_g4_set_matrix4(kinc_g5_constant_location_t location, kinc_matrix4x4_t *value) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&vertexConstantBuffer, location.impl.vertexOffset, value);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&fragmentConstantBuffer, location.impl.fragmentOffset, value);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&computeConstantBuffer, location.impl.computeOffset, value);
}

void kinc_g4_set_matrix3(kinc_g5_constant_location_t location, kinc_matrix3x3_t *value) {
	if (location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&vertexConstantBuffer, location.impl.vertexOffset, value);
	if (location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&fragmentConstantBuffer, location.impl.fragmentOffset, value);
	if (location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&computeConstantBuffer, location.impl.computeOffset, value);
}

void kinc_g4_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (unit.stages[i] >= 0) {
			if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
				sampler_options[i][unit.stages[i]].u_addressing = (kinc_g5_texture_addressing_t)addressing;
			}
			if (dir == KINC_G4_TEXTURE_DIRECTION_V) {
				sampler_options[i][unit.stages[i]].v_addressing = (kinc_g5_texture_addressing_t)addressing;
			}
		}
	}
}

void kinc_g4_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].magnification_filter = (kinc_g5_texture_filter_t)filter;
		}
	}
}

void kinc_g4_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].minification_filter = (kinc_g5_texture_filter_t)filter;
		}
	}
}

void kinc_g4_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].mipmap_filter = (kinc_g5_mipmap_filter_t)filter;
		}
	}
}

void kinc_g4_restore_render_target(void) {
	kinc_g5_internal_restore_render_target();
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void kinc_g4_set_render_targets(kinc_g5_render_target_t **targets, int count) {
	for (int i = 0; i < count; ++i) {
		windows[0].current_render_targets[i] = targets[i];
		if (windows[0].current_render_targets[i]->state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
			kinc_g5_command_list_texture_to_render_target_barrier(&commandList, windows[0].current_render_targets[i]);
			windows[0].current_render_targets[i]->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
		}
	}
	windows[0].current_render_target_count = count;
	kinc_g5_render_target_t *render_targets[16];
	assert(count <= 16);
	for (int i = 0; i < count; ++i) {
		render_targets[i] = targets[i];
	}
	kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_t *g5_vertex_buffer = &buffer->impl._buffer[buffer->impl._currentIndex];
	current_state.vertex_buffer = g5_vertex_buffer;
	kinc_g5_command_list_set_vertex_buffer(&commandList, g5_vertex_buffer);
}

void kinc_g4_set_index_buffer(kinc_g5_index_buffer_t *buffer) {
	current_state.index_buffer = buffer;
	kinc_g5_command_list_set_index_buffer(&commandList, buffer);
}

void kinc_g4_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	if (!texture->_uploaded) {
		kinc_g5_command_list_upload_texture(&commandList, texture);
		texture->_uploaded = true;
	}

	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.texture_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.texture_units[i], &g5_unit)) {
			current_state.textures[i] = texture;
			current_state.texture_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.texture_count < MAX_TEXTURES);
		current_state.textures[current_state.texture_count] = texture;
		current_state.texture_units[current_state.texture_count] = g5_unit;
		current_state.texture_count += 1;
	}

	kinc_g5_command_list_set_texture(&commandList, g5_unit, texture);
}

void kinc_g5_set_pipeline(kinc_g5_pipeline_t *pipeline) {
	current_state.pipeline = pipeline;
	kinc_g5_command_list_set_pipeline(&commandList, pipeline);
}

void kinc_g4_render_target_use_color_as_texture(kinc_g5_render_target_t *render_target, kinc_g5_texture_unit_t unit) {
	if (render_target->state != KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		kinc_g5_command_list_render_target_to_texture_barrier(&commandList, render_target);
		render_target->state = KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.render_target_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.render_target_units[i], &g5_unit)) {
			current_state.render_targets[i] = render_target;
			current_state.render_target_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.render_target_count < MAX_TEXTURES);
		current_state.render_targets[current_state.render_target_count] = render_target;
		current_state.render_target_units[current_state.render_target_count] = g5_unit;
		current_state.render_target_count += 1;
	}

	kinc_g5_command_list_set_texture_from_render_target(&commandList, g5_unit, render_target);
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g5_render_target_t *render_target, kinc_g5_texture_unit_t unit) {
	if (render_target->state != KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		kinc_g5_command_list_render_target_to_texture_barrier(&commandList, render_target);
		render_target->state = KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.depth_render_target_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.depth_render_target_units[i], &g5_unit)) {
			current_state.depth_render_targets[i] = render_target;
			current_state.depth_render_target_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.depth_render_target_count < MAX_TEXTURES);
		current_state.depth_render_targets[current_state.depth_render_target_count] = render_target;
		current_state.depth_render_target_units[current_state.depth_render_target_count] = g5_unit;
		current_state.depth_render_target_count += 1;
	}

	kinc_g5_command_list_set_texture_from_render_target_depth(&commandList, g5_unit, render_target);
}

void kinc_g5_set_compute_shader(kinc_g5_compute_shader *shader) {
	current_state.compute_shader = shader;
	kinc_g5_command_list_set_compute_shader(&commandList, shader);
}

void kinc_g4_compute(int x, int y, int z) {
	kinc_internal_start_draw(true);
	kinc_g5_command_list_compute(&commandList, x, y, z);
	kinc_internal_end_draw(true);
}

void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *render_target, uint8_t *data) {
	kinc_g5_command_list_get_render_target_pixels(&commandList, render_target, data);
}

void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *render_target, int levels) {

}

void kinc_g4_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_unlock_all(buffer);
	kinc_g5_command_list_upload_index_buffer(&commandList, buffer);
}

void kinc_g4_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	kinc_g5_index_buffer_unlock(buffer, count);
	kinc_g5_command_list_upload_index_buffer(&commandList, buffer);
}

static void kinc_internal_init_vertex_element(kinc_g5_vertex_element_t *element, const char *name, kinc_g5_vertex_data_t data) {
	element->name = name;
	element->data = data;
}

void kinc_g5_vertex_structure_init(kinc_g5_vertex_structure_t *structure) {
	structure->size = 0;
}

void kinc_g5_vertex_structure_add(kinc_g5_vertex_structure_t *structure, const char *name, kinc_g5_vertex_data_t data) {
	kinc_internal_init_vertex_element(&structure->elements[structure->size++], name, data);
}

extern bool waitAfterNextDraw;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, kinc_g4_usage_t usage) {
	buffer->impl._multiple = usage == KINC_G4_USAGE_STATIC ? 1 : 2;
	buffer->impl._currentIndex = 0;
	buffer->impl.myCount = count;
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_init(&buffer->impl._buffer[i], count, structure, usage == KINC_G4_USAGE_STATIC);
	}
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_destroy(&buffer->impl._buffer[i]);
	}
}

static void kinc_internal_prepare_lock(kinc_g4_vertex_buffer_t *buffer) {
	++buffer->impl._currentIndex;
	if (buffer->impl._currentIndex >= buffer->impl._multiple - 1) {
		waitAfterNextDraw = true;
	}
	if (buffer->impl._currentIndex >= buffer->impl._multiple) {
		buffer->impl._currentIndex = 0;
	}
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_internal_prepare_lock(buffer);
	return kinc_g5_vertex_buffer_lock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	kinc_internal_prepare_lock(buffer);
	return kinc_g5_vertex_buffer_lock(&buffer->impl._buffer[buffer->impl._currentIndex], start, count);
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_unlock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g5_vertex_buffer_unlock(&buffer->impl._buffer[buffer->impl._currentIndex], count);
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_stride(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value;
}

void kinc_g5_constant_buffer_set_int2(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
}

void kinc_g5_constant_buffer_set_int3(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

void kinc_g5_constant_buffer_set_int4(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

void kinc_g5_constant_buffer_set_ints(kinc_g5_constant_buffer_t *buffer, int offset, int *values, int count) {
	int *ints = (int *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		ints[i] = values[i];
	}
}

void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value;
}

void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
}

void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count) {
	float *floats = (float *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		floats[i] = values[i];
	}
}

void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value ? 1 : 0;
}

static void kinc_internal_set_matrix3(uint8_t *constants, int offset, kinc_matrix3x3_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = kinc_matrix3x3_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value) {
	if (kinc_g5_transpose_mat) {
		kinc_matrix3x3_t m = *value;
		kinc_matrix3x3_transpose(&m);
		kinc_internal_set_matrix3(buffer->data, offset, &m);
	}
	else {
		kinc_internal_set_matrix3(buffer->data, offset, value);
	}
}

static void kinc_internal_set_matrix4(uint8_t *constants, int offset, kinc_matrix4x4_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = kinc_matrix4x4_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value) {
	if (kinc_g5_transpose_mat) {
		kinc_matrix4x4_t m = *value;
		kinc_matrix4x4_transpose(&m);
		kinc_internal_set_matrix4(buffer->data, offset, &m);
	}
	else {
		kinc_internal_set_matrix4(buffer->data, offset, value);
	}
}

void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipe) {
	pipe->input_layout = NULL;
	pipe->vertex_shader = NULL;
	pipe->fragment_shader = NULL;

	pipe->cull_mode = KINC_G5_CULL_MODE_NEVER;
	pipe->depth_write = false;
	pipe->depth_mode = KINC_G5_COMPARE_MODE_ALWAYS;

	pipe->blend_source = KINC_G5_BLEND_ONE;
	pipe->blend_destination = KINC_G5_BLEND_ZERO;
	pipe->blend_operation = KINC_G5_BLENDOP_ADD;
	pipe->alpha_blend_source = KINC_G5_BLEND_ONE;
	pipe->alpha_blend_destination = KINC_G5_BLEND_ZERO;
	pipe->alpha_blend_operation = KINC_G5_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		pipe->color_write_mask_red[i] = true;
		pipe->color_write_mask_green[i] = true;
		pipe->color_write_mask_blue[i] = true;
		pipe->color_write_mask_alpha[i] = true;
		pipe->color_attachment[i] = KINC_IMAGE_FORMAT_RGBA32;
	}

	pipe->color_attachment_count = 1;
	pipe->depth_attachment_bits = 0;
}

void kinc_g5_internal_pipeline_set_defaults(kinc_g5_pipeline_t *state) {
	state->input_layout = NULL;
	state->vertex_shader = NULL;
	state->fragment_shader = NULL;

	state->cull_mode = KINC_G5_CULL_MODE_NEVER;
	state->depth_write = false;
	state->depth_mode = KINC_G5_COMPARE_MODE_ALWAYS;

	state->blend_source = KINC_G5_BLEND_ONE;
	state->blend_destination = KINC_G5_BLEND_ZERO;
	state->blend_operation = KINC_G5_BLENDOP_ADD;
	state->alpha_blend_source = KINC_G5_BLEND_ONE;
	state->alpha_blend_destination = KINC_G5_BLEND_ZERO;
	state->alpha_blend_operation = KINC_G5_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		state->color_write_mask_red[i] = true;
		state->color_write_mask_green[i] = true;
		state->color_write_mask_blue[i] = true;
		state->color_write_mask_alpha[i] = true;
		state->color_attachment[i] = KINC_IMAGE_FORMAT_RGBA32;
	}

	state->color_attachment_count = 1;
	state->depth_attachment_bits = 0;
}

void kinc_g5_sampler_options_set_defaults(kinc_g5_sampler_options_t *options) {
	options->u_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;
	options->v_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;

	options->magnification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	options->minification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	options->mipmap_filter = KINC_G5_MIPMAP_FILTER_POINT;
}

void kinc_internal_samplers_reset(void) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		for (int j = 0; j < MAX_SAMPLERS_PER_STAGE; ++j) {
			kinc_g5_sampler_options_set_defaults(&sampler_options[i][j]);
		}
	}
}

bool sampler_options_equals(kinc_g5_sampler_options_t *options1, kinc_g5_sampler_options_t *options2) {
	return options1->u_addressing == options2->u_addressing &&
		   options1->v_addressing == options2->v_addressing &&
	       options1->minification_filter == options2->minification_filter &&
	       options1->magnification_filter == options2->magnification_filter &&
		   options1->mipmap_filter == options2->mipmap_filter;
}

kinc_g5_sampler_t *kinc_internal_get_current_sampler(int stage, int unit) {
	// TODO: Please make this much faster
	for (int i = 0; i < sampler_cache_size; ++i) {
		if (sampler_options_equals(&sampler_cache[i].options, &sampler_options[stage][unit])) {
			return &sampler_cache[i].sampler;
		}
	}

	assert(sampler_cache_size < MAX_SAMPLER_CACHE_SIZE);
	kinc_g5_sampler_t *sampler = &sampler_cache[sampler_cache_size].sampler;
	kinc_g5_sampler_init(sampler, &sampler_options[stage][unit]);
	sampler_cache[sampler_cache_size].options = sampler_options[stage][unit];
	sampler_cache_size += 1;

	return sampler;
}
