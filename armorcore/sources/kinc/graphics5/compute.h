#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics5/compute.h>
#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/textureunit.h>

/*! \file compute.h
    \brief Provides support for running compute-shaders.
*/

typedef struct kinc_g5_compute_shader {
	kinc_g5_compute_shader_impl impl;
} kinc_g5_compute_shader;

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *source, int length);
void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader);
kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name);
kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name);
