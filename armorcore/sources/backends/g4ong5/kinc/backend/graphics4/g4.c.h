#include "g4.h"
#include "kinc/graphics4/graphics.h"
#include "kinc/window.h"
#include <kinc/backend/graphics4/indexbuffer.h>
#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/backend/graphics4/vertexbuffer.h>
#include <kinc/color.h>
#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/compute.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/sampler.h>
#include <kinc/io/filereader.h>
#include <kinc/math/core.h>
#include <kinc/math/matrix.h>
#include <kinc/system.h>
#include <assert.h>
#include <string.h>

kinc_g5_command_list_t commandList;

bool waitAfterNextDraw = false;

static kinc_g5_constant_buffer_t vertexConstantBuffer;
static kinc_g5_constant_buffer_t fragmentConstantBuffer;
static kinc_g5_constant_buffer_t computeConstantBuffer;
#define constantBufferSize 4096
#define constantBufferMultiply 100
static int constantBufferIndex = 0;

void kinc_g4_internal_init(void) {
	kinc_g5_internal_init();
}

void kinc_g4_internal_destroy(void) {
	kinc_g5_internal_destroy();
}

void kinc_g4_internal_destroy_window(int window) {
	kinc_g5_internal_destroy_window(window);
}

#define bufferCount 2
#define renderTargetCount 8

static struct {
	int currentBuffer;
	kinc_g5_render_target_t framebuffers[bufferCount];

	kinc_g4_render_target_t *current_render_targets[renderTargetCount];
	int current_render_target_count;

	bool resized;
} windows[16] = {0};

static int current_window;

#define MAX_TEXTURES 16

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

	uint8_t vertex_constant_data[constantBufferSize];
	uint8_t fragment_constant_data[constantBufferSize];
	uint8_t compute_constant_data[constantBufferSize];
} render_state;

static render_state current_state;

void kinc_g4_on_g5_internal_resize(int window, int width, int height) {
	windows[window].resized = true;
}

void kinc_g4_on_g5_internal_restore_render_target(void) {
	windows[current_window].current_render_targets[0] = NULL;
	kinc_g5_render_target_t *render_target = &windows[current_window].framebuffers[windows[current_window].currentBuffer];
	kinc_g5_command_list_set_render_targets(&commandList, &render_target, 1);
	windows[current_window].current_render_target_count = 1;
}

void kinc_g4_internal_init_window(int window, int depthBufferBits, bool vsync) {
	kinc_g5_internal_init_window(window, depthBufferBits, vsync);

	kinc_g5_command_list_init(&commandList);
	windows[window].currentBuffer = -1;
	for (int i = 0; i < bufferCount; ++i) {
		kinc_g5_render_target_init_framebuffer(&windows[window].framebuffers[i], kinc_window_width(window), kinc_window_height(window),
		                                       KINC_G5_RENDER_TARGET_FORMAT_32BIT, depthBufferBits);
	}
	kinc_g5_constant_buffer_init(&vertexConstantBuffer, constantBufferSize * constantBufferMultiply);
	kinc_g5_constant_buffer_init(&fragmentConstantBuffer, constantBufferSize * constantBufferMultiply);
	kinc_g5_constant_buffer_init(&computeConstantBuffer, constantBufferSize * constantBufferMultiply);

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

void kinc_g4_on_g5_internal_set_samplers(int count, kinc_g5_texture_unit_t *texture_units) {
	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < KINC_G5_SHADER_TYPE_COUNT; ++j) {
			if (texture_units[i].stages[j] >= 0) {
				kinc_g5_sampler_t *sampler = get_current_sampler(j, texture_units[i].stages[j]);
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

static void startDraw(bool compute) {
	if ((constantBufferIndex + 1) >= constantBufferMultiply || waitAfterNextDraw) {
		memcpy(current_state.vertex_constant_data, vertexConstantBuffer.data, constantBufferSize);
		memcpy(current_state.fragment_constant_data, fragmentConstantBuffer.data, constantBufferSize);
		memcpy(current_state.compute_constant_data, computeConstantBuffer.data, constantBufferSize);
	}
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);
	kinc_g5_constant_buffer_unlock(&computeConstantBuffer);

	kinc_g4_on_g5_internal_set_samplers(current_state.texture_count, current_state.texture_units);
	kinc_g4_on_g5_internal_set_samplers(current_state.render_target_count, current_state.render_target_units);
	kinc_g4_on_g5_internal_set_samplers(current_state.depth_render_target_count, current_state.depth_render_target_units);

	if (compute) {
		kinc_g5_command_list_set_compute_constant_buffer(&commandList, &computeConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
	}
	else {
		kinc_g5_command_list_set_vertex_constant_buffer(&commandList, &vertexConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
		kinc_g5_command_list_set_fragment_constant_buffer(&commandList, &fragmentConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
	}
}

static void endDraw(bool compute) {
	++constantBufferIndex;
	if (constantBufferIndex >= constantBufferMultiply || waitAfterNextDraw) {
		kinc_g5_command_list_end(&commandList);
		kinc_g5_command_list_execute(&commandList);
		kinc_g5_command_list_wait_for_execution_to_finish(&commandList);
		kinc_g5_command_list_begin(&commandList);
		if (windows[current_window].current_render_targets[0] == NULL) {
			kinc_g4_on_g5_internal_restore_render_target();
		}
		else {
			const int count = windows[current_window].current_render_target_count;
			kinc_g5_render_target_t *render_targets[16];
			for (int i = 0; i < count; ++i) {
				render_targets[i] = &windows[current_window].current_render_targets[i]->impl._renderTarget;
			}
			kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
		}

		if (current_state.pipeline != NULL) {
			kinc_g5_command_list_set_pipeline(&commandList, current_state.pipeline);
		}
		if (current_state.compute_shader != NULL) {
#ifndef KINC_METAL
			// Metal still has some trouble switching between graphics and compute encoders
			kinc_g5_command_list_set_compute_shader(&commandList, current_state.compute_shader);
#endif
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

		kinc_g5_constant_buffer_lock(&vertexConstantBuffer, 0, constantBufferSize);
		kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, 0, constantBufferSize);
		kinc_g5_constant_buffer_lock(&computeConstantBuffer, 0, constantBufferSize);

		memcpy(vertexConstantBuffer.data, current_state.vertex_constant_data, constantBufferSize);
		memcpy(fragmentConstantBuffer.data, current_state.fragment_constant_data, constantBufferSize);
		memcpy(computeConstantBuffer.data, current_state.compute_constant_data, constantBufferSize);
	}
	else {
		kinc_g5_constant_buffer_lock(&vertexConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
		kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
		kinc_g5_constant_buffer_lock(&computeConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
	}
}

void kinc_g4_draw_indexed_vertices(void) {
	startDraw(false);
	kinc_g5_command_list_draw_indexed_vertices(&commandList);
	endDraw(false);
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	startDraw(false);
	kinc_g5_command_list_draw_indexed_vertices_from_to(&commandList, start, count);
	endDraw(false);
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth) {
	if (windows[current_window].current_render_target_count > 0) {
		if (windows[current_window].current_render_targets[0] == NULL) {
			kinc_g5_command_list_clear(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer], flags, color, depth);
		}
		else {
			if (windows[current_window].current_render_targets[0]->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
				kinc_g5_command_list_texture_to_render_target_barrier(&commandList, &windows[current_window].current_render_targets[0]->impl._renderTarget);
				windows[current_window].current_render_targets[0]->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
			}
			kinc_g5_command_list_clear(&commandList, &windows[current_window].current_render_targets[0]->impl._renderTarget, flags, color, depth);
		}
	}
}

bool first_run = true;

void kinc_g4_begin(int window) {
	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	current_window = window;

	windows[current_window].currentBuffer = (windows[current_window].currentBuffer + 1) % bufferCount;

	bool resized = windows[window].resized;
	if (resized) {
		for (int i = 0; i < bufferCount; ++i) {
			kinc_g5_render_target_destroy(&windows[current_window].framebuffers[i]);
		}
		windows[current_window].currentBuffer = 0;
	}

	kinc_g5_begin(&windows[current_window].framebuffers[windows[current_window].currentBuffer], window);

	if (resized) {
		for (int i = 0; i < bufferCount; ++i) {
			kinc_g5_render_target_init_framebuffer(&windows[current_window].framebuffers[i], kinc_window_width(window), kinc_window_height(window),
			                                       KINC_G5_RENDER_TARGET_FORMAT_32BIT, 16);
		}
		windows[window].resized = false;
	}

	windows[current_window].current_render_targets[0] = NULL;
	windows[current_window].current_render_target_count = 1;

	current_state.pipeline = NULL;
	current_state.compute_shader = NULL;
	current_state.index_buffer = NULL;
	current_state.vertex_buffer = NULL;
	current_state.viewport_set = false;
	current_state.scissor_set = false;
	current_state.texture_count = 0;
	current_state.render_target_count = 0;
	current_state.depth_render_target_count = 0;

	samplers_reset();

	// commandList = new Graphics5::CommandList;
	kinc_g5_command_list_begin(&commandList);

	// Currently we do not necessarily wait at the end of a frame so for now it's endDraw
	// constantBufferIndex = 0;
	// kinc_g5_constant_buffer_lock(&vertexConstantBuffer, 0, constantBufferSize);
	// kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, 0, constantBufferSize);
	endDraw(false);

	kinc_g5_command_list_framebuffer_to_render_target_barrier(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer]);
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

void kinc_g4_end(int window) {
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);
	kinc_g5_constant_buffer_unlock(&computeConstantBuffer);

	kinc_g5_command_list_render_target_to_framebuffer_barrier(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer]);
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	// delete commandList;
	// commandList = nullptr;
	kinc_g5_end(window);

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

bool kinc_g4_swap_buffers(void) {
	return kinc_g5_swap_buffers();
}

void kinc_g4_flush(void) {
	kinc_g5_flush();
}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int(&computeConstantBuffer, location.impl._location.impl.computeOffset, value);
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int2(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2);
}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int3(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2, value3);
}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3, value4);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3, value4);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_int4(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2, value3, value4);
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, values, count);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, values, count);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_ints(&computeConstantBuffer, location.impl._location.impl.computeOffset, values, count);
}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float(&computeConstantBuffer, location.impl._location.impl.computeOffset, value);
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2);
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2, value3);
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3, value4);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3, value4);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&computeConstantBuffer, location.impl._location.impl.computeOffset, value1, value2, value3, value4);
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, values, count);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, values, count);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&computeConstantBuffer, location.impl._location.impl.computeOffset, values, count);
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&computeConstantBuffer, location.impl._location.impl.computeOffset, value);
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&computeConstantBuffer, location.impl._location.impl.computeOffset, value);
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
	if (location.impl._location.impl.computeOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&computeConstantBuffer, location.impl._location.impl.computeOffset, value);
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
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

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].magnification_filter = (kinc_g5_texture_filter_t)filter;
		}
	}
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].minification_filter = (kinc_g5_texture_filter_t)filter;
		}
	}
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].mipmap_filter = (kinc_g5_mipmap_filter_t)filter;
		}
	}
}

void kinc_g4_restore_render_target(void) {
	kinc_g4_on_g5_internal_restore_render_target();
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void kinc_g4_set_render_targets(kinc_g4_render_target_t **targets, int count) {
	for (int i = 0; i < count; ++i) {
		windows[current_window].current_render_targets[i] = targets[i];
		if (windows[current_window].current_render_targets[i]->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
			kinc_g5_command_list_texture_to_render_target_barrier(&commandList, &windows[current_window].current_render_targets[i]->impl._renderTarget);
			windows[current_window].current_render_targets[i]->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
		}
	}
	windows[current_window].current_render_target_count = count;
	kinc_g5_render_target_t *render_targets[16];
	assert(count <= 16);
	for (int i = 0; i < count; ++i) {
		render_targets[i] = &targets[i]->impl._renderTarget;
	}
	kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_t *g5_vertex_buffer = &buffer->impl._buffer;
	current_state.vertex_buffer = g5_vertex_buffer;
	kinc_g5_command_list_set_vertex_buffer(&commandList, g5_vertex_buffer);
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g5_index_buffer_t *g5_index_buffer = &buffer->impl._buffer;
	current_state.index_buffer = g5_index_buffer;
	kinc_g5_command_list_set_index_buffer(&commandList, g5_index_buffer);
}

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	if (!texture->impl._uploaded) {
		kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
		texture->impl._uploaded = true;
	}

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.texture_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.texture_units[i], &g5_unit)) {
			current_state.textures[i] = &texture->impl._texture;
			current_state.texture_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.texture_count < MAX_TEXTURES);
		current_state.textures[current_state.texture_count] = &texture->impl._texture;
		current_state.texture_units[current_state.texture_count] = g5_unit;
		current_state.texture_count += 1;
	}

	kinc_g5_command_list_set_texture(&commandList, g5_unit, &texture->impl._texture);
}

int kinc_g4_max_bound_textures(void) {
	return kinc_g5_max_bound_textures();
}

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g5_pipeline_t *g5_pipeline = &pipeline->impl._pipeline;
	current_state.pipeline = g5_pipeline;
	kinc_g5_command_list_set_pipeline(&commandList, g5_pipeline);
}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *render_target, kinc_g4_texture_unit_t unit) {
	if (render_target->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		kinc_g5_command_list_render_target_to_texture_barrier(&commandList, &render_target->impl._renderTarget);
		render_target->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.render_target_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.render_target_units[i], &g5_unit)) {
			current_state.render_targets[i] = &render_target->impl._renderTarget;
			current_state.render_target_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.render_target_count < MAX_TEXTURES);
		current_state.render_targets[current_state.render_target_count] = &render_target->impl._renderTarget;
		current_state.render_target_units[current_state.render_target_count] = g5_unit;
		current_state.render_target_count += 1;
	}

	kinc_g5_command_list_set_texture_from_render_target(&commandList, g5_unit, &render_target->impl._renderTarget);
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *render_target, kinc_g4_texture_unit_t unit) {
	if (render_target->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		kinc_g5_command_list_render_target_to_texture_barrier(&commandList, &render_target->impl._renderTarget);
		render_target->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	bool found = false;
	for (int i = 0; i < current_state.depth_render_target_count; ++i) {
		if (kinc_g5_texture_unit_equals(&current_state.depth_render_target_units[i], &g5_unit)) {
			current_state.depth_render_targets[i] = &render_target->impl._renderTarget;
			current_state.depth_render_target_units[i] = g5_unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.depth_render_target_count < MAX_TEXTURES);
		current_state.depth_render_targets[current_state.depth_render_target_count] = &render_target->impl._renderTarget;
		current_state.depth_render_target_units[current_state.depth_render_target_count] = g5_unit;
		current_state.depth_render_target_count += 1;
	}

	kinc_g5_command_list_set_texture_from_render_target_depth(&commandList, g5_unit, &render_target->impl._renderTarget);
}

void kinc_g4_set_compute_shader(kinc_g4_compute_shader *shader) {
	kinc_g5_compute_shader *g5_shader = &shader->impl.shader;
	current_state.compute_shader = g5_shader;
	kinc_g5_command_list_set_compute_shader(&commandList, g5_shader);
}

void kinc_g4_compute(int x, int y, int z) {
	startDraw(true);
	kinc_g5_command_list_compute(&commandList, x, y, z);
	endDraw(true);
}
