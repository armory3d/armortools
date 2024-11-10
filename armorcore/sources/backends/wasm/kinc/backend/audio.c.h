#include <kinc/audio2/audio.h>
#include <stdlib.h>

static kinc_a2_buffer_t a2_buffer;

void kinc_a2_init() {
	kinc_a2_internal_init();
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {}

static uint32_t samples_per_second = 44100;

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}
