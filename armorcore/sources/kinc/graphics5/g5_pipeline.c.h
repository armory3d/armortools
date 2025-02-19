#include "g5_pipeline.h"
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
