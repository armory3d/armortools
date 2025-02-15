#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/image.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <stdio.h>
#include <string.h>

extern kinc_g5_command_list_t commandList;

void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *source, int length) {
	kinc_g5_compute_shader_init(&shader->impl.shader, source, length);
}

void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader) {
	kinc_g5_compute_shader_destroy(&shader->impl.shader);
}

kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_constant_location_t location;
	location.impl._location = kinc_g5_compute_shader_get_constant_location(&shader->impl.shader, name);
	return location;
}

kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g5_texture_unit_t g5_unit = kinc_g5_compute_shader_get_texture_unit(&shader->impl.shader, name);
	kinc_g4_texture_unit_t g4_unit;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		g4_unit.stages[i] = g5_unit.stages[i];
	}
	return g4_unit;
}
