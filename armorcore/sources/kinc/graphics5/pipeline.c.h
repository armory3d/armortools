#include "pipeline.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SAMPLERS_PER_STAGE 16

static kinc_g5_sampler_options_t sampler_options[KINC_G5_SHADER_TYPE_COUNT][MAX_SAMPLERS_PER_STAGE];

struct sampler_cache_entry {
	kinc_g5_sampler_options_t options;
	kinc_g5_sampler_t sampler;
};

#define MAX_SAMPLER_CACHE_SIZE 256
static struct sampler_cache_entry sampler_cache[MAX_SAMPLER_CACHE_SIZE];
static int sampler_cache_size = 0;

void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipe) {
	pipe->inputLayout = NULL;
	pipe->vertexShader = NULL;
	pipe->fragmentShader = NULL;

	pipe->cullMode = KINC_G5_CULL_MODE_NEVER;
	pipe->depthWrite = false;
	pipe->depthMode = KINC_G5_COMPARE_MODE_ALWAYS;

	pipe->blend_source = KINC_G5_BLEND_ONE;
	pipe->blend_destination = KINC_G5_BLEND_ZERO;
	pipe->blend_operation = KINC_G5_BLENDOP_ADD;
	pipe->alpha_blend_source = KINC_G5_BLEND_ONE;
	pipe->alpha_blend_destination = KINC_G5_BLEND_ZERO;
	pipe->alpha_blend_operation = KINC_G5_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		pipe->colorWriteMaskRed[i] = true;
		pipe->colorWriteMaskGreen[i] = true;
		pipe->colorWriteMaskBlue[i] = true;
		pipe->colorWriteMaskAlpha[i] = true;
		pipe->colorAttachment[i] = KINC_G5_RENDER_TARGET_FORMAT_32BIT;
	}

	pipe->colorAttachmentCount = 1;
	pipe->depthAttachmentBits = 0;
}

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

void kinc_g4_internal_pipeline_set_defaults(kinc_g4_pipeline_t *state) {
	state->input_layout = NULL;
	state->vertex_shader = NULL;
	state->fragment_shader = NULL;

	state->cull_mode = KINC_G4_CULL_NOTHING;
	state->depth_write = false;
	state->depth_mode = KINC_G4_COMPARE_ALWAYS;

	state->blend_source = KINC_G4_BLEND_ONE;
	state->blend_destination = KINC_G4_BLEND_ZERO;
	state->blend_operation = KINC_G4_BLENDOP_ADD;
	state->alpha_blend_source = KINC_G4_BLEND_ONE;
	state->alpha_blend_destination = KINC_G4_BLEND_ZERO;
	state->alpha_blend_operation = KINC_G4_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		state->color_write_mask_red[i] = true;
		state->color_write_mask_green[i] = true;
		state->color_write_mask_blue[i] = true;
		state->color_write_mask_alpha[i] = true;
		state->color_attachment[i] = KINC_G4_RENDER_TARGET_FORMAT_32BIT;
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

void samplers_reset(void) {
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

kinc_g5_sampler_t *get_current_sampler(int stage, int unit) {
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

void kinc_g4_shader_init(kinc_g4_shader_t *shader, const void *_data, size_t length, kinc_g4_shader_type_t type) {
	kinc_g5_shader_init(&shader->impl._shader, _data, length, (kinc_g5_shader_type_t)type);
}

#ifdef KRAFIX_LIBRARY
extern int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

int kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type) {
#ifdef KRAFIX_LIBRARY
	char *output = malloc(1024 * 1024);
	int length;

#ifdef KINC_WINDOWS
	const char *system = "windows";
#elif defined(KINC_MACOS)
	const char *system = "macos";
#elif defined(KINC_LINUX)
	const char *system = "linux";
#elif defined(KINC_ANDROID)
	const char *system = "android";
#elif defined(KINC_IOS)
	const char *system = "ios";
#endif

#ifdef KINC_VULKAN
	const char *target = "spirv";
#elif defined(KINC_METAL)
	const char *target = "metal";
#else
	const char *target = "d3d11";
#endif

	int errors = krafix_compile(source, output, &length, target, system, type == KINC_G4_SHADER_TYPE_FRAGMENT ? "frag" : "vert", -1);
	if (errors > 0) {
		return errors;
	}
	kinc_g4_shader_init(shader, output, length, type);
	return 0;
#else
	return 0;
#endif
}

void kinc_g4_shader_destroy(kinc_g4_shader_t *shader) {
	kinc_g5_shader_destroy(&shader->impl._shader);
}
