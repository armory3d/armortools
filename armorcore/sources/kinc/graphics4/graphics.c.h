#include "graphics.h"

static int antialiasing_samples;

int kinc_g4_antialiasing_samples(void) {
	return antialiasing_samples;
}

void kinc_g4_set_antialiasing_samples(int samples) {
	antialiasing_samples = samples;
}
