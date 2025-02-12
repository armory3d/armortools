#include <kinc/graphics5/sampler.h>
#include <assert.h>

#define MAX_SAMPLERS_PER_STAGE 16

static kinc_g5_sampler_options_t sampler_options[KINC_G5_SHADER_TYPE_COUNT][MAX_SAMPLERS_PER_STAGE];

static void samplers_reset(void) {
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

struct sampler_cache_entry {
	kinc_g5_sampler_options_t options;
	kinc_g5_sampler_t sampler;
};

#define MAX_SAMPLER_CACHE_SIZE 256
static struct sampler_cache_entry sampler_cache[MAX_SAMPLER_CACHE_SIZE];
static int sampler_cache_size = 0;

// TODO: Please make this much faster
static kinc_g5_sampler_t *get_current_sampler(int stage, int unit) {
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
