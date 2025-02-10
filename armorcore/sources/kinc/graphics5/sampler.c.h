#include "sampler.h"

void kinc_g5_sampler_options_set_defaults(kinc_g5_sampler_options_t *options) {
	options->u_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;
	options->v_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;

	options->magnification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	options->minification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	options->mipmap_filter = KINC_G5_MIPMAP_FILTER_POINT;
}
