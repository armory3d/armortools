#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics5/shader.h>
#include <stddef.h>

/*! \file shader.h
    \brief Provides functions for creating and destroying shaders.
*/

typedef enum kinc_g5_shader_type {
	KINC_G5_SHADER_TYPE_FRAGMENT,
	KINC_G5_SHADER_TYPE_VERTEX,
	KINC_G5_SHADER_TYPE_COMPUTE,
	KINC_G5_SHADER_TYPE_COUNT
} kinc_g5_shader_type_t;

typedef struct kinc_g5_shader {
	Shader5Impl impl;
} kinc_g5_shader_t;

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type);
void kinc_g5_shader_destroy(kinc_g5_shader_t *shader);
