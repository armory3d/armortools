#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics4/shader.h>

/*! \file shader.h
    \brief Provides functions for creating and destroying shaders.
*/

typedef enum kinc_g4_shader_type {
	KINC_G4_SHADER_TYPE_FRAGMENT,
	KINC_G4_SHADER_TYPE_VERTEX,
	KINC_G4_SHADER_TYPE_COMPUTE,
	KINC_G4_SHADER_TYPE_COUNT
} kinc_g4_shader_type_t;

typedef struct kinc_g4_shader {
	kinc_g4_shader_impl_t impl;
} kinc_g4_shader_t;

void kinc_g4_shader_init(kinc_g4_shader_t *shader, const void *data, size_t length, kinc_g4_shader_type_t type);
int kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type);
void kinc_g4_shader_destroy(kinc_g4_shader_t *shader);
