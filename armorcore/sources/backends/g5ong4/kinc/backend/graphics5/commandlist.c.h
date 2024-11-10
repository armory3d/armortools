#include <kinc/graphics4/graphics.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>

#include <kinc/log.h>

#include <assert.h>
#include <string.h>

#ifdef KINC_MICROSOFT
#include <malloc.h>
#endif

#define WRITE(type, value)                                                                                                                                     \
	if (list->impl.commandIndex + sizeof(type) > KINC_G5ONG4_COMMANDS_SIZE) {                                                                                  \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Trying to write too many commands to the command list.");                                                              \
		return;                                                                                                                                                \
	}                                                                                                                                                          \
	*(type *)(&list->impl.commands[list->impl.commandIndex]) = value;                                                                                          \
	list->impl.commandIndex += sizeof(type);
#define READ(type, var)                                                                                                                                        \
	if (index + sizeof(type) > KINC_G5ONG4_COMMANDS_SIZE) {                                                                                                    \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Trying to read beyond the end of the command list?");                                                                  \
		return;                                                                                                                                                \
	}                                                                                                                                                          \
	type var = *(type *)(&list->impl.commands[index]);                                                                                                         \
	index += sizeof(type);

typedef enum command {
	Clear,
	Draw,
	SetViewport,
	SetScissor,
	SetPipeline,
	SetVertexBuffers,
	SetIndexBuffer,
	SetRenderTargets,
	SetRenderTargetFace,
	DrawInstanced,
	SetSampler,
	SetTexture,
	SetImageTexture,
	SetTextureFromRenderTarget,
	SetTextureFromRenderTargetDepth,
	SetVertexConstantBuffer,
	SetFragmentConstantBuffer,
	SetBlendConstant,
} command_t;

void kinc_g4_pipeline_get_constant_locations(kinc_g4_pipeline_t *state, kinc_g4_constant_location_t *vertex_locations,
                                             kinc_g4_constant_location_t *fragment_locations, int *vertex_sizes, int *fragment_sizes, int *max_vertex,
                                             int *max_fragment);

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	list->impl.commandIndex = 0;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth,
                                int stencil) {
	WRITE(command_t, Clear);
	WRITE(unsigned, flags);
	WRITE(unsigned, color);
	WRITE(float, depth);
	WRITE(int, stencil);
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {
	WRITE(command_t, Draw);
	WRITE(int, start);
	WRITE(int, count);
}

void kinc_g5_command_list_draw_indexed_vertices_instanced(kinc_g5_command_list_t *list, int instanceCount) {
	kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(list, instanceCount, 0, list->impl._indexCount);
}
void kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(kinc_g5_command_list_t *list, int instanceCount, int start, int count) {
	WRITE(command_t, DrawInstanced);
	WRITE(int, instanceCount);
	WRITE(int, start);
	WRITE(int, count);
}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	WRITE(command_t, SetViewport);
	WRITE(int, x);
	WRITE(int, y);
	WRITE(int, width);
	WRITE(int, height);
}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	WRITE(command_t, SetScissor);
	WRITE(int, x);
	WRITE(int, y);
	WRITE(int, width);
	WRITE(int, height);
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	WRITE(command_t, SetPipeline);
	WRITE(kinc_g5_pipeline_t *, pipeline);
}

void kinc_g5_command_list_set_blend_constant(kinc_g5_command_list_t *list, float r, float g, float b, float a) {
	WRITE(command_t, SetBlendConstant);
	WRITE(float, r);
	WRITE(float, g);
	WRITE(float, b);
	WRITE(float, a);
}

void kinc_g5_command_list_set_vertex_buffers(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer **buffers, int *offsets, int count) {
	WRITE(command_t, SetVertexBuffers);
	WRITE(int, count);
	for (int i = 0; i < count; ++i) {
		WRITE(kinc_g5_vertex_buffer_t *, buffers[i]);
		if (offsets[i] != 0) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_g5_command_list_set_vertex_buffers: offsets not supported");
		}
	}
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	WRITE(command_t, SetIndexBuffer);
	WRITE(kinc_g5_index_buffer_t *, buffer);
	list->impl._indexCount = buffer->impl.myCount;
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	WRITE(command_t, SetRenderTargets);
	WRITE(int, count);
	for (int i = 0; i < count; ++i) {
		WRITE(kinc_g5_render_target_t *, targets[i]);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	WRITE(command_t, SetVertexConstantBuffer);
	WRITE(kinc_g5_constant_buffer_t *, buffer);
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	WRITE(command_t, SetFragmentConstantBuffer);
	WRITE(kinc_g5_constant_buffer_t *, buffer);
}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	kinc_g5_pipeline_t *current_pipeline = NULL;
	int index = 0;
	while (index < list->impl.commandIndex) {
		READ(command_t, command);
		switch (command) {
		case Clear: {
			READ(unsigned, flags);
			READ(unsigned, color);
			READ(float, depth);
			READ(int, stencil);
			kinc_g4_clear(flags, color, depth, stencil);
			break;
		}
		case Draw: {
			READ(int, start);
			READ(int, count);
			kinc_g4_draw_indexed_vertices_from_to(start, count);
			break;
		}
		case SetViewport: {
			READ(int, x);
			READ(int, y);
			READ(int, width);
			READ(int, height);
			kinc_g4_viewport(x, y, width, height);
			break;
		}
		case SetScissor: {
			READ(int, x);
			READ(int, y);
			READ(int, width);
			READ(int, height);
			kinc_g4_scissor(x, y, width, height);
			break;
		}
		case SetPipeline: {
			READ(kinc_g5_pipeline_t *, pipeline);
			current_pipeline = pipeline;
			kinc_g4_set_pipeline(&pipeline->impl.pipe);
			break;
		}
		case SetVertexBuffers: {
			READ(int, count);
#ifdef KINC_MICROSOFT
			kinc_g4_vertex_buffer_t **buffers = (kinc_g4_vertex_buffer_t **)alloca(sizeof(kinc_g4_vertex_buffer_t *) * count);
#else
			kinc_g4_vertex_buffer_t *buffers[count];
#endif
			for (int i = 0; i < count; ++i) {
				READ(kinc_g5_vertex_buffer_t *, buffer);
				buffers[i] = &buffer->impl.buffer;
			}
			kinc_g4_set_vertex_buffers(buffers, count);
			break;
		}
		case SetIndexBuffer: {
			READ(kinc_g5_index_buffer_t *, buffer);
			kinc_g4_set_index_buffer(&buffer->impl.buffer);
			break;
		}
		case SetRenderTargets: {
			READ(int, count);
#ifdef KINC_MICROSOFT
			kinc_g4_render_target_t **buffers = (kinc_g4_render_target_t **)alloca(sizeof(kinc_g4_render_target_t *) * count);
#else
			kinc_g4_render_target_t *buffers[count];
#endif
			int first_framebuffer_index = -1;
			for (int i = 0; i < count; ++i) {
				READ(kinc_g5_render_target_t *, buffer);
				if (i == 0) {
					first_framebuffer_index = buffer->framebuffer_index;
				}
				buffers[i] = &buffer->impl.target;
			}
			if (first_framebuffer_index >= 0) {
				if (count > 1) {
					kinc_log(KINC_LOG_LEVEL_ERROR, "Rendering to backbuffer and render targets at the same time is not supported");
				}
				kinc_g4_restore_render_target();
			}
			else {
				kinc_g4_set_render_targets(buffers, count);
			}
			break;
		}
		case SetRenderTargetFace: {
			READ(kinc_g5_render_target_t *, target);
			READ(int, face);
			kinc_g4_set_render_target_face(&target->impl.target, face);
			break;
		}
		case DrawInstanced: {
			READ(int, instanceCount);
			READ(int, start);
			READ(int, count);
			kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, start, count);
			break;
		}
		case SetSampler: {
			assert(false);
			// TODO
			break;
		}
		case SetTexture: {
			READ(kinc_g5_texture_unit_t, unit);
			READ(kinc_g5_texture_t *, texture);
			assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
			kinc_g4_texture_unit_t g4_unit;
			memcpy(&g4_unit.stages[0], &unit.stages[0], KINC_G4_SHADER_TYPE_COUNT * sizeof(int));
			kinc_g4_set_texture(g4_unit, &texture->impl.texture);
			break;
		}
		case SetImageTexture: {
			READ(kinc_g5_texture_unit_t, unit);
			READ(kinc_g5_texture_t *, texture);
			assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
			kinc_g4_texture_unit_t g4_unit;
			memcpy(&g4_unit.stages[0], &unit.stages[0], KINC_G4_SHADER_TYPE_COUNT * sizeof(int));
			kinc_g4_set_image_texture(g4_unit, &texture->impl.texture);
			break;
		}
		case SetTextureFromRenderTarget: {
			assert(false);
			// TODO
			break;
		}
		case SetTextureFromRenderTargetDepth: {
			assert(false);
			// TODO
			break;
		}
		case SetVertexConstantBuffer: {
			READ(kinc_g5_constant_buffer_t *, buffer);
			(void)buffer;
			(void)current_pipeline;
			kinc_log(KINC_LOG_LEVEL_ERROR, "Constant buffers are not supported on G5onG4 at the moment.");
			// 		if(current_pipeline == NULL) {
			// 			kinc_log(KINC_LOG_LEVEL_ERROR, "Please set the pipeline before setting constant buffers.");
			// 		} else {
			// 			kinc_g4_constant_location_t *constant_locations = current_pipeline->impl.pipe.vertex_locations;
			// 			int *sizes = current_pipeline->impl.pipe.vertex_sizes;
			// 			char *data = buffer->data;
			// 			for(int i = 0; i < current_pipeline->impl.pipe.vertex_count; ++i) {
			// 				// kinc_g4_set
			// 				// kinc_g4_set_vertex_constant_buffer(constant_locations[i], sizes[i], data);
			// 				data += sizes[i];
			// 			}
			// 		}
			break;
		}
		case SetFragmentConstantBuffer: {
			READ(kinc_g5_constant_buffer_t *, buffer);
			(void)buffer;
			kinc_log(KINC_LOG_LEVEL_ERROR, "Constant buffers are not supported on G5onG4 at the moment.");
			break;
		}
		case SetBlendConstant: {
			READ(float, r);
			READ(float, g);
			READ(float, b);
			READ(float, a);
			kinc_g4_set_blend_constant(r, g, b, a);
		}
		default:
			kinc_log(KINC_LOG_LEVEL_ERROR, "Unknown command %i\n", command);
			return;
		}
	}
}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {
	kinc_g4_flush();
}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, struct kinc_g5_render_target *render_target, uint8_t *data) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_g5_command_list_get_render_target_pixels not implemented");
}

void kinc_g5_command_list_set_render_target_face(kinc_g5_command_list_t *list, kinc_g5_render_target_t *texture, int face) {
	WRITE(command_t, SetRenderTargetFace);
	WRITE(kinc_g5_render_target_t *, texture);
	WRITE(int, face);
}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	WRITE(command_t, SetTexture);
	WRITE(kinc_g5_texture_unit_t, unit);
	WRITE(kinc_g5_texture_t *, texture);
}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {
	WRITE(command_t, SetSampler);
}

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *render_target) {
	WRITE(command_t, SetTextureFromRenderTarget);
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit,
                                                               kinc_g5_render_target_t *render_target) {
	WRITE(command_t, SetTextureFromRenderTargetDepth);
}

void kinc_g5_command_list_set_image_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	WRITE(command_t, SetImageTexture);
	WRITE(kinc_g5_texture_unit_t, unit);
	WRITE(kinc_g5_texture_t *, texture);
}

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, struct kinc_g5_compute_shader *shader) {}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {}
