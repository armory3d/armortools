#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics5/pipeline.h>

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *pipe) {
	kinc_g4_internal_pipeline_set_defaults(pipe);
	kinc_g5_pipeline_init(&pipe->impl._pipeline);
}

void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *pipe) {
	kinc_g5_pipeline_destroy(&pipe->impl._pipeline);
}

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *pipe, const char *name) {
	kinc_g4_constant_location_t location;
	location.impl._location = kinc_g5_pipeline_get_constant_location(&pipe->impl._pipeline, name);
	return location;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t g5_unit = kinc_g5_pipeline_get_texture_unit(&pipe->impl._pipeline, name);

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g4_texture_unit_t g4_unit;
	memcpy(&g4_unit.stages[0], &g5_unit.stages[0], KINC_G4_SHADER_TYPE_COUNT * sizeof(int));

	return g4_unit;
}

void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *pipe) {
	pipe->impl._pipeline.inputLayout = pipe->input_layout;
	pipe->impl._pipeline.vertexShader = &pipe->vertex_shader->impl._shader;
	pipe->impl._pipeline.fragmentShader = &pipe->fragment_shader->impl._shader;
	pipe->impl._pipeline.blend_source = (kinc_g5_blending_factor_t)pipe->blend_source;
	pipe->impl._pipeline.blend_destination = (kinc_g5_blending_factor_t)pipe->blend_destination;
	pipe->impl._pipeline.blend_operation = (kinc_g5_blending_operation_t)pipe->blend_operation;
	pipe->impl._pipeline.alpha_blend_source = (kinc_g5_blending_factor_t)pipe->alpha_blend_source;
	pipe->impl._pipeline.alpha_blend_destination = (kinc_g5_blending_factor_t)pipe->alpha_blend_destination;
	pipe->impl._pipeline.alpha_blend_operation = (kinc_g5_blending_operation_t)pipe->alpha_blend_operation;
	pipe->impl._pipeline.cullMode = (kinc_g5_cull_mode_t)pipe->cull_mode;
	pipe->impl._pipeline.depthMode = (kinc_g5_compare_mode_t)pipe->depth_mode;
	pipe->impl._pipeline.depthWrite = pipe->depth_write;
	pipe->impl._pipeline.colorAttachmentCount = pipe->color_attachment_count;
	for (int i = 0; i < 8; ++i) {
		pipe->impl._pipeline.colorWriteMaskRed[i] = pipe->color_write_mask_red[i];
		pipe->impl._pipeline.colorWriteMaskGreen[i] = pipe->color_write_mask_green[i];
		pipe->impl._pipeline.colorWriteMaskBlue[i] = pipe->color_write_mask_blue[i];
		pipe->impl._pipeline.colorWriteMaskAlpha[i] = pipe->color_write_mask_alpha[i];
		pipe->impl._pipeline.colorAttachment[i] = (kinc_g5_render_target_format_t)pipe->color_attachment[i];
	}
	pipe->impl._pipeline.depthAttachmentBits = pipe->depth_attachment_bits;
	kinc_g5_pipeline_compile(&pipe->impl._pipeline);
}
