#include "iron_gpu.h"
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include "iron_system.h"
#include "iron_math.h"
#include "iron_file.h"

#define CONSTANT_BUFFER_SIZE 4096
#define CONSTANT_BUFFER_MULTIPLY 100
#define FRAMEBUFFER_COUNT 2
#define MAX_TEXTURES 16
#define MAX_SAMPLERS_PER_STAGE 16
#define MAX_SAMPLER_CACHE_SIZE 256

static iron_gpu_sampler_options_t sampler_options[IRON_GPU_SHADER_TYPE_COUNT][MAX_SAMPLERS_PER_STAGE];

struct sampler_cache_entry {
	iron_gpu_sampler_options_t options;
	iron_gpu_sampler_t sampler;
};

static struct sampler_cache_entry sampler_cache[MAX_SAMPLER_CACHE_SIZE];
static int sampler_cache_size = 0;

void iron_internal_samplers_reset(void);
iron_gpu_sampler_t *iron_internal_get_current_sampler(int stage, int unit);

iron_gpu_command_list_t commandList;
bool waitAfterNextDraw = false;

static iron_gpu_buffer_t vertexConstantBuffer;
static iron_gpu_buffer_t fragmentConstantBuffer;
static int constantBufferIndex = 0;

static struct {
	int currentBuffer;
	iron_gpu_texture_t framebuffers[FRAMEBUFFER_COUNT];
	iron_gpu_texture_t *current_render_targets[8];
	int current_render_target_count;
	bool resized;
} windows[1] = {0};

typedef struct render_state {
	iron_gpu_pipeline_t *pipeline;

	iron_gpu_buffer_t *index_buffer;
	iron_gpu_buffer_t *vertex_buffer;

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

	iron_gpu_texture_t *textures[MAX_TEXTURES];
	iron_gpu_texture_unit_t texture_units[MAX_TEXTURES];
	int texture_count;

	iron_gpu_texture_t *depth_render_targets[MAX_TEXTURES];
	iron_gpu_texture_unit_t depth_render_target_units[MAX_TEXTURES];
	int depth_render_target_count;

	uint8_t vertex_constant_data[CONSTANT_BUFFER_SIZE];
	uint8_t fragment_constant_data[CONSTANT_BUFFER_SIZE];
} render_state;

static render_state current_state;

bool iron_gpu_texture_unit_equals(iron_gpu_texture_unit_t *unit1, iron_gpu_texture_unit_t *unit2) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		if (unit1->stages[i] != unit2->stages[i]) {
			return false;
		}
	}
	return true;
}

void iron_gpu_internal_resize(int width, int height) {
	windows[0].resized = true;
}

void iron_gpu_internal_restore_render_target(void) {
	windows[0].current_render_targets[0] = NULL;
	iron_gpu_texture_t *render_target = &windows[0].framebuffers[windows[0].currentBuffer];
	iron_gpu_command_list_set_render_targets(&commandList, &render_target, 1);
	windows[0].current_render_target_count = 1;
}

void gpu_internal_init_window(int depthBufferBits, bool vsync) {
	iron_gpu_internal_init_window(depthBufferBits, vsync);

	iron_gpu_command_list_init(&commandList);
	windows[0].currentBuffer = -1;
	for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
		iron_gpu_render_target_init_framebuffer(&windows[0].framebuffers[i], iron_window_width(), iron_window_height(),
		                                       IRON_IMAGE_FORMAT_RGBA32, depthBufferBits);
	}
	iron_gpu_constant_buffer_init(&vertexConstantBuffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLY);
	iron_gpu_constant_buffer_init(&fragmentConstantBuffer, CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MULTIPLY);

	// to support doing work after gpu_end and before gpu_begin
	iron_gpu_command_list_begin(&commandList);
}

void iron_gpu_internal_set_samplers(int count, iron_gpu_texture_unit_t *texture_units) {
	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < IRON_GPU_SHADER_TYPE_COUNT; ++j) {
			if (texture_units[i].stages[j] >= 0) {
				iron_gpu_sampler_t *sampler = iron_internal_get_current_sampler(j, texture_units[i].stages[j]);
				iron_gpu_texture_unit_t unit;
				for (int k = 0; k < IRON_GPU_SHADER_TYPE_COUNT; ++k) {
					unit.stages[k] = -1;
				}
				unit.stages[j] = texture_units[i].stages[j];
				iron_gpu_command_list_set_sampler(&commandList, unit, sampler);
			}
		}
	}
}

static void iron_internal_start_draw(bool compute) {
	if ((constantBufferIndex + 1) >= CONSTANT_BUFFER_MULTIPLY || waitAfterNextDraw) {
		memcpy(current_state.vertex_constant_data, vertexConstantBuffer.data, CONSTANT_BUFFER_SIZE);
		memcpy(current_state.fragment_constant_data, fragmentConstantBuffer.data, CONSTANT_BUFFER_SIZE);
	}
	iron_gpu_constant_buffer_unlock(&vertexConstantBuffer);
	iron_gpu_constant_buffer_unlock(&fragmentConstantBuffer);

	iron_gpu_internal_set_samplers(current_state.texture_count, current_state.texture_units);
	iron_gpu_internal_set_samplers(current_state.depth_render_target_count, current_state.depth_render_target_units);

	iron_gpu_command_list_set_vertex_constant_buffer(&commandList, &vertexConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	iron_gpu_command_list_set_fragment_constant_buffer(&commandList, &fragmentConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
}

static void iron_internal_end_draw(bool compute) {
	++constantBufferIndex;
	if (constantBufferIndex >= CONSTANT_BUFFER_MULTIPLY || waitAfterNextDraw) {
		iron_gpu_command_list_end(&commandList);
		iron_gpu_command_list_execute(&commandList);
		iron_gpu_command_list_wait_for_execution_to_finish(&commandList);
		iron_gpu_command_list_begin(&commandList);
		if (windows[0].current_render_targets[0] == NULL) {
			iron_gpu_internal_restore_render_target();
		}
		else {
			const int count = windows[0].current_render_target_count;
			iron_gpu_texture_t *render_targets[16];
			for (int i = 0; i < count; ++i) {
				render_targets[i] = windows[0].current_render_targets[i];
			}
			iron_gpu_command_list_set_render_targets(&commandList, render_targets, count);
		}

		if (current_state.pipeline != NULL) {
			iron_gpu_command_list_set_pipeline(&commandList, current_state.pipeline);
		}
		if (current_state.index_buffer != NULL) {
			iron_gpu_command_list_set_index_buffer(&commandList, current_state.index_buffer);
		}
		if (current_state.vertex_buffer != NULL) {
			iron_gpu_command_list_set_vertex_buffer(&commandList, current_state.vertex_buffer);
		}
		if (current_state.viewport_set) {
			iron_gpu_command_list_viewport(&commandList, current_state.viewport_x, current_state.viewport_y, current_state.viewport_width,
			                              current_state.viewport_height);
		}
		if (current_state.scissor_set) {
			iron_gpu_command_list_scissor(&commandList, current_state.scissor_x, current_state.scissor_y, current_state.scissor_width,
			                             current_state.scissor_height);
		}
		for (int i = 0; i < current_state.texture_count; ++i) {
			iron_gpu_command_list_set_texture(&commandList, current_state.texture_units[i], current_state.textures[i]);
		}
		for (int i = 0; i < current_state.depth_render_target_count; ++i) {
			iron_gpu_command_list_set_texture_from_render_target_depth(&commandList, current_state.depth_render_target_units[i],
			                                                          current_state.depth_render_targets[i]);
		}
		constantBufferIndex = 0;
		waitAfterNextDraw = false;

		iron_gpu_constant_buffer_lock(&vertexConstantBuffer, 0, CONSTANT_BUFFER_SIZE);
		iron_gpu_constant_buffer_lock(&fragmentConstantBuffer, 0, CONSTANT_BUFFER_SIZE);

		memcpy(vertexConstantBuffer.data, current_state.vertex_constant_data, CONSTANT_BUFFER_SIZE);
		memcpy(fragmentConstantBuffer.data, current_state.fragment_constant_data, CONSTANT_BUFFER_SIZE);
	}
	else {
		iron_gpu_constant_buffer_lock(&vertexConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
		iron_gpu_constant_buffer_lock(&fragmentConstantBuffer, constantBufferIndex * CONSTANT_BUFFER_SIZE, CONSTANT_BUFFER_SIZE);
	}
}

void iron_gpu_draw_indexed_vertices(void) {
	iron_internal_start_draw(false);
	iron_gpu_command_list_draw_indexed_vertices(&commandList);
	iron_internal_end_draw(false);
}

void iron_gpu_draw_indexed_vertices_from_to(int start, int count) {
	iron_internal_start_draw(false);
	iron_gpu_command_list_draw_indexed_vertices_from_to(&commandList, start, count);
	iron_internal_end_draw(false);
}

void iron_gpu_clear(unsigned color, float depth, unsigned flags) {
	if (windows[0].current_render_target_count > 0) {
		if (windows[0].current_render_targets[0] == NULL) {
			iron_gpu_command_list_clear(&commandList, &windows[0].framebuffers[windows[0].currentBuffer], flags, color, depth);
		}
		else {
			if (windows[0].current_render_targets[0]->state != IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
				iron_gpu_command_list_texture_to_render_target_barrier(&commandList, windows[0].current_render_targets[0]);
				windows[0].current_render_targets[0]->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
			}
			iron_gpu_command_list_clear(&commandList, windows[0].current_render_targets[0], flags, color, depth);
		}
	}
}

void gpu_begin() {
	// to support doing work after gpu_end and before gpu_begin
	iron_gpu_command_list_end(&commandList);
	iron_gpu_command_list_execute(&commandList);

	windows[0].currentBuffer = (windows[0].currentBuffer + 1) % FRAMEBUFFER_COUNT;

	bool resized = windows[0].resized;
	if (resized) {
		for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
			iron_gpu_texture_destroy(&windows[0].framebuffers[i]);
		}
		windows[0].currentBuffer = 0;
	}

	iron_gpu_begin(&windows[0].framebuffers[windows[0].currentBuffer]);

	if (resized) {
		for (int i = 0; i < FRAMEBUFFER_COUNT; ++i) {
			iron_gpu_render_target_init_framebuffer(&windows[0].framebuffers[i], iron_window_width(), iron_window_height(),
			                                       IRON_IMAGE_FORMAT_RGBA32, 16);
		}
		windows[0].resized = false;
	}

	windows[0].current_render_targets[0] = NULL;
	windows[0].current_render_target_count = 1;

	current_state.pipeline = NULL;
	current_state.index_buffer = NULL;
	current_state.vertex_buffer = NULL;
	current_state.viewport_set = false;
	current_state.scissor_set = false;
	current_state.texture_count = 0;
	current_state.depth_render_target_count = 0;

	iron_internal_samplers_reset();

	iron_gpu_command_list_begin(&commandList);

	// Currently we do not necessarily wait at the end of a frame so for now it's iron_internal_end_draw
	iron_internal_end_draw(false);

	iron_gpu_command_list_framebuffer_to_render_target_barrier(&commandList, &windows[0].framebuffers[windows[0].currentBuffer]);
	gpu_restore_render_target();
}

void gpu_viewport(int x, int y, int width, int height) {
	current_state.viewport_x = x;
	current_state.viewport_y = y;
	current_state.viewport_width = width;
	current_state.viewport_height = height;
	current_state.viewport_set = true;
	iron_gpu_command_list_viewport(&commandList, x, y, width, height);
}

void gpu_scissor(int x, int y, int width, int height) {
	current_state.scissor_x = x;
	current_state.scissor_y = y;
	current_state.scissor_width = width;
	current_state.scissor_height = height;
	current_state.scissor_set = true;
	iron_gpu_command_list_scissor(&commandList, x, y, width, height);
}

void gpu_disable_scissor(void) {
	current_state.scissor_set = false;
	iron_gpu_command_list_disable_scissor(&commandList);
}

void gpu_end() {
	iron_gpu_constant_buffer_unlock(&vertexConstantBuffer);
	iron_gpu_constant_buffer_unlock(&fragmentConstantBuffer);

	iron_gpu_command_list_render_target_to_framebuffer_barrier(&commandList, &windows[0].framebuffers[windows[0].currentBuffer]);
	iron_gpu_command_list_end(&commandList);
	iron_gpu_command_list_execute(&commandList);

	iron_gpu_end();

	// to support doing work after gpu_end and before gpu_begin
	iron_gpu_command_list_begin(&commandList);
}

void gpu_set_int(iron_gpu_constant_location_t *location, int value) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_int(&vertexConstantBuffer, location->impl.vertexOffset, value);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_int(&fragmentConstantBuffer, location->impl.fragmentOffset, value);
}

void gpu_set_int2(iron_gpu_constant_location_t *location, int value1, int value2) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_int2(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_int2(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2);
}

void gpu_set_int3(iron_gpu_constant_location_t *location, int value1, int value2, int value3) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_int3(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2, value3);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_int3(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2, value3);
}

void gpu_set_int4(iron_gpu_constant_location_t *location, int value1, int value2, int value3, int value4) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_int4(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2, value3, value4);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_int4(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2, value3, value4);
}

void gpu_set_ints(iron_gpu_constant_location_t *location, int *values, int count) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_ints(&vertexConstantBuffer, location->impl.vertexOffset, values, count);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_ints(&fragmentConstantBuffer, location->impl.fragmentOffset, values, count);
}

void gpu_set_float(iron_gpu_constant_location_t *location, float value) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_float(&vertexConstantBuffer, location->impl.vertexOffset, value);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_float(&fragmentConstantBuffer, location->impl.fragmentOffset, value);
}

void gpu_set_float2(iron_gpu_constant_location_t *location, float value1, float value2) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_float2(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_float2(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2);
}

void gpu_set_float3(iron_gpu_constant_location_t *location, float value1, float value2, float value3) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_float3(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2, value3);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_float3(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2, value3);
}

void gpu_set_float4(iron_gpu_constant_location_t *location, float value1, float value2, float value3, float value4) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_float4(&vertexConstantBuffer, location->impl.vertexOffset, value1, value2, value3, value4);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_float4(&fragmentConstantBuffer, location->impl.fragmentOffset, value1, value2, value3, value4);
}

void gpu_set_floats(iron_gpu_constant_location_t *location, f32_array_t *values) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_floats(&vertexConstantBuffer, location->impl.vertexOffset, values->buffer, values->length);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_floats(&fragmentConstantBuffer, location->impl.fragmentOffset, values->buffer, values->length);
}

void gpu_set_bool(iron_gpu_constant_location_t *location, bool value) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_bool(&vertexConstantBuffer, location->impl.vertexOffset, value);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_bool(&fragmentConstantBuffer, location->impl.fragmentOffset, value);
}

void gpu_set_matrix4(iron_gpu_constant_location_t *location, iron_matrix4x4_t value) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_matrix4(&vertexConstantBuffer, location->impl.vertexOffset, &value);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_matrix4(&fragmentConstantBuffer, location->impl.fragmentOffset, &value);
}

void gpu_set_matrix3(iron_gpu_constant_location_t *location, iron_matrix3x3_t value) {
	if (location->impl.vertexOffset >= 0)
		iron_gpu_constant_buffer_set_matrix3(&vertexConstantBuffer, location->impl.vertexOffset, &value);
	if (location->impl.fragmentOffset >= 0)
		iron_gpu_constant_buffer_set_matrix3(&fragmentConstantBuffer, location->impl.fragmentOffset, &value);
}

void gpu_set_texture_addressing(iron_gpu_texture_unit_t unit, gpu_texture_direction_t dir, gpu_texture_addressing_t addressing) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		if (unit.stages[i] >= 0) {
			if (dir == GPU_TEXTURE_DIRECTION_U) {
				sampler_options[i][unit.stages[i]].u_addressing = (iron_gpu_texture_addressing_t)addressing;
			}
			if (dir == GPU_TEXTURE_DIRECTION_V) {
				sampler_options[i][unit.stages[i]].v_addressing = (iron_gpu_texture_addressing_t)addressing;
			}
		}
	}
}

void gpu_set_texture_magnification_filter(iron_gpu_texture_unit_t texunit, gpu_texture_filter_t filter) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].magnification_filter = (iron_gpu_texture_filter_t)filter;
		}
	}
}

void gpu_set_texture_minification_filter(iron_gpu_texture_unit_t texunit, gpu_texture_filter_t filter) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].minification_filter = (iron_gpu_texture_filter_t)filter;
		}
	}
}

void gpu_set_texture_mipmap_filter(iron_gpu_texture_unit_t texunit, gpu_mipmap_filter_t filter) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		if (texunit.stages[i] >= 0) {
			sampler_options[i][texunit.stages[i]].mipmap_filter = (iron_gpu_mipmap_filter_t)filter;
		}
	}
}

void gpu_restore_render_target(void) {
	iron_gpu_internal_restore_render_target();
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void gpu_set_render_targets(iron_gpu_texture_t **targets, int count) {
	for (int i = 0; i < count; ++i) {
		windows[0].current_render_targets[i] = targets[i];
		if (windows[0].current_render_targets[i]->state != IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
			iron_gpu_command_list_texture_to_render_target_barrier(&commandList, windows[0].current_render_targets[i]);
			windows[0].current_render_targets[i]->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
		}
	}
	windows[0].current_render_target_count = count;
	iron_gpu_texture_t *render_targets[16];
	for (int i = 0; i < count; ++i) {
		render_targets[i] = targets[i];
	}
	iron_gpu_command_list_set_render_targets(&commandList, render_targets, count);
	current_state.viewport_set = false;
	current_state.scissor_set = false;
}

void gpu_set_vertex_buffer(iron_gpu_buffer_t *buffer) {
	current_state.vertex_buffer = buffer;
	iron_gpu_command_list_set_vertex_buffer(&commandList, buffer);
}

void gpu_set_index_buffer(iron_gpu_buffer_t *buffer) {
	current_state.index_buffer = buffer;
	iron_gpu_command_list_set_index_buffer(&commandList, buffer);
}

void iron_gpu_set_pipeline(iron_gpu_pipeline_t *pipeline) {
	current_state.pipeline = pipeline;
	iron_gpu_command_list_set_pipeline(&commandList, pipeline);
}

void gpu_set_texture(iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *render_target) {
	if (!render_target->_uploaded) {
		iron_gpu_command_list_upload_texture(&commandList, render_target);
		render_target->_uploaded = true;
	}

	if (render_target->state != IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		iron_gpu_command_list_render_target_to_texture_barrier(&commandList, render_target);
		render_target->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	bool found = false;
	for (int i = 0; i < current_state.texture_count; ++i) {
		if (iron_gpu_texture_unit_equals(&current_state.texture_units[i], unit)) {
			current_state.textures[i] = render_target;
			current_state.texture_units[i] = *unit;
			found = true;
			break;
		}
	}
	if (!found) {
		current_state.textures[current_state.texture_count] = render_target;
		current_state.texture_units[current_state.texture_count] = *unit;
		current_state.texture_count += 1;
	}

	iron_gpu_command_list_set_texture(&commandList, *unit, render_target);
}

void gpu_set_texture_depth(iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *render_target) {
	if (render_target->state != IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE) {
		iron_gpu_command_list_render_target_to_texture_barrier(&commandList, render_target);
		render_target->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	}

	bool found = false;
	for (int i = 0; i < current_state.depth_render_target_count; ++i) {
		if (iron_gpu_texture_unit_equals(&current_state.depth_render_target_units[i], unit)) {
			current_state.depth_render_targets[i] = render_target;
			current_state.depth_render_target_units[i] = *unit;
			found = true;
			break;
		}
	}
	if (!found) {
		assert(current_state.depth_render_target_count < MAX_TEXTURES);
		current_state.depth_render_targets[current_state.depth_render_target_count] = render_target;
		current_state.depth_render_target_units[current_state.depth_render_target_count] = *unit;
		current_state.depth_render_target_count += 1;
	}

	iron_gpu_command_list_set_texture_from_render_target_depth(&commandList, *unit, render_target);
}

void iron_gpu_render_target_get_pixels(iron_gpu_texture_t *render_target, uint8_t *data) {
	iron_gpu_command_list_get_render_target_pixels(&commandList, render_target, data);
}

void gpu_index_buffer_unlock_all(iron_gpu_buffer_t *buffer) {
	iron_gpu_index_buffer_unlock_all(buffer);
	iron_gpu_command_list_upload_index_buffer(&commandList, buffer);
}

void gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer, int count) {
	iron_gpu_index_buffer_unlock(buffer, count);
	iron_gpu_command_list_upload_index_buffer(&commandList, buffer);
}

void iron_gpu_vertex_structure_init(iron_gpu_vertex_structure_t *structure) {
	structure->size = 0;
}

void iron_gpu_vertex_structure_add(iron_gpu_vertex_structure_t *structure, const char *name, iron_gpu_vertex_data_t data) {
	structure->elements[structure->size].name = name;
	structure->elements[structure->size].data = data;
	structure->size++;
}

void gpu_vertex_buffer_init(iron_gpu_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, gpu_usage_t usage) {
	buffer->myCount = count;
	iron_gpu_vertex_buffer_init(buffer, count, structure, usage == GPU_USAGE_STATIC);
}

float *gpu_vertex_buffer_lock_all(iron_gpu_buffer_t *buffer) {
	waitAfterNextDraw = true;
	return iron_gpu_vertex_buffer_lock_all(buffer);
}

float *gpu_vertex_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count) {
	waitAfterNextDraw = true;
	return iron_gpu_vertex_buffer_lock(buffer, start, count);
}

void iron_gpu_constant_buffer_set_int(iron_gpu_buffer_t *buffer, int offset, int value) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value;
}

void iron_gpu_constant_buffer_set_int2(iron_gpu_buffer_t *buffer, int offset, int value1, int value2) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
}

void iron_gpu_constant_buffer_set_int3(iron_gpu_buffer_t *buffer, int offset, int value1, int value2, int value3) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

void iron_gpu_constant_buffer_set_int4(iron_gpu_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

void iron_gpu_constant_buffer_set_ints(iron_gpu_buffer_t *buffer, int offset, int *values, int count) {
	int *ints = (int *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		ints[i] = values[i];
	}
}

void iron_gpu_constant_buffer_set_float(iron_gpu_buffer_t *buffer, int offset, float value) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value;
}

void iron_gpu_constant_buffer_set_float2(iron_gpu_buffer_t *buffer, int offset, float value1, float value2) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
}

void iron_gpu_constant_buffer_set_float3(iron_gpu_buffer_t *buffer, int offset, float value1, float value2, float value3) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void iron_gpu_constant_buffer_set_float4(iron_gpu_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

void iron_gpu_constant_buffer_set_floats(iron_gpu_buffer_t *buffer, int offset, float *values, int count) {
	float *floats = (float *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		floats[i] = values[i];
	}
}

void iron_gpu_constant_buffer_set_bool(iron_gpu_buffer_t *buffer, int offset, bool value) {
	int *ints = (int *)(&buffer->data[offset]);
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

void iron_gpu_constant_buffer_set_matrix3(iron_gpu_buffer_t *buffer, int offset, iron_matrix3x3_t *value) {
	if (iron_gpu_transpose_mat) {
		iron_matrix3x3_t m = *value;
		iron_matrix3x3_transpose(&m);
		iron_internal_set_matrix3(buffer->data, offset, &m);
	}
	else {
		iron_internal_set_matrix3(buffer->data, offset, value);
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

void iron_gpu_constant_buffer_set_matrix4(iron_gpu_buffer_t *buffer, int offset, iron_matrix4x4_t *value) {
	if (iron_gpu_transpose_mat) {
		iron_matrix4x4_t m = *value;
		iron_matrix4x4_transpose(&m);
		iron_internal_set_matrix4(buffer->data, offset, &m);
	}
	else {
		iron_internal_set_matrix4(buffer->data, offset, value);
	}
}

void iron_gpu_internal_pipeline_init(iron_gpu_pipeline_t *pipe) {
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

void iron_gpu_internal_pipeline_set_defaults(iron_gpu_pipeline_t *state) {
	state->input_layout = NULL;
	state->vertex_shader = NULL;
	state->fragment_shader = NULL;

	state->cull_mode = IRON_GPU_CULL_MODE_NEVER;
	state->depth_write = false;
	state->depth_mode = IRON_GPU_COMPARE_MODE_ALWAYS;

	state->blend_source = IRON_GPU_BLEND_ONE;
	state->blend_destination = IRON_GPU_BLEND_ZERO;
	state->blend_operation = IRON_GPU_BLENDOP_ADD;
	state->alpha_blend_source = IRON_GPU_BLEND_ONE;
	state->alpha_blend_destination = IRON_GPU_BLEND_ZERO;
	state->alpha_blend_operation = IRON_GPU_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		state->color_write_mask_red[i] = true;
		state->color_write_mask_green[i] = true;
		state->color_write_mask_blue[i] = true;
		state->color_write_mask_alpha[i] = true;
		state->color_attachment[i] = IRON_IMAGE_FORMAT_RGBA32;
	}

	state->color_attachment_count = 1;
	state->depth_attachment_bits = 0;

	state->kong = false;
}

void iron_gpu_sampler_options_set_defaults(iron_gpu_sampler_options_t *options) {
	options->u_addressing = IRON_GPU_TEXTURE_ADDRESSING_CLAMP;
	options->v_addressing = IRON_GPU_TEXTURE_ADDRESSING_CLAMP;

	options->magnification_filter = IRON_GPU_TEXTURE_FILTER_POINT;
	options->minification_filter = IRON_GPU_TEXTURE_FILTER_POINT;
	options->mipmap_filter = IRON_GPU_MIPMAP_FILTER_POINT;
}

void iron_internal_samplers_reset(void) {
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		for (int j = 0; j < MAX_SAMPLERS_PER_STAGE; ++j) {
			iron_gpu_sampler_options_set_defaults(&sampler_options[i][j]);
		}
	}
}

bool sampler_options_equals(iron_gpu_sampler_options_t *options1, iron_gpu_sampler_options_t *options2) {
	return options1->u_addressing == options2->u_addressing &&
		   options1->v_addressing == options2->v_addressing &&
	       options1->minification_filter == options2->minification_filter &&
	       options1->magnification_filter == options2->magnification_filter &&
		   options1->mipmap_filter == options2->mipmap_filter;
}

iron_gpu_sampler_t *iron_internal_get_current_sampler(int stage, int unit) {
	// TODO: Please make this much faster
	for (int i = 0; i < sampler_cache_size; ++i) {
		if (sampler_options_equals(&sampler_cache[i].options, &sampler_options[stage][unit])) {
			return &sampler_cache[i].sampler;
		}
	}

	assert(sampler_cache_size < MAX_SAMPLER_CACHE_SIZE);
	iron_gpu_sampler_t *sampler = &sampler_cache[sampler_cache_size].sampler;
	iron_gpu_sampler_init(sampler, &sampler_options[stage][unit]);
	sampler_cache[sampler_cache_size].options = sampler_options[stage][unit];
	sampler_cache_size += 1;

	return sampler;
}
