#include <kinc/graphics5/compute.h>
#include <kinc/math/core.h>

#include <assert.h>

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *source, int length) {
	kinc_g4_compute_shader_init(&shader->impl.g4, source, length);
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {
	kinc_g4_compute_shader_destroy(&shader->impl.g4);
}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location = {0};
	location.impl.location = kinc_g4_compute_shader_get_constant_location(&shader->impl.g4, name);
	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_texture_unit_t g5_unit = {0};
	kinc_g4_texture_unit_t g4_unit = kinc_g4_compute_shader_get_texture_unit(&shader->impl.g4, name);
	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		g5_unit.stages[i] = g4_unit.stages[i];
	}
	return g5_unit;
}
