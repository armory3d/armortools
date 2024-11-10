#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/shader.h>

#include <stddef.h>

/*! \file shader.h
    \brief Provides functions for creating and destroying shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g5_shader_type {
	KINC_G5_SHADER_TYPE_FRAGMENT,
	KINC_G5_SHADER_TYPE_VERTEX,
	KINC_G5_SHADER_TYPE_COMPUTE,
	KINC_G5_SHADER_TYPE_GEOMETRY,
	KINC_G5_SHADER_TYPE_TESSELLATION_CONTROL,
	KINC_G5_SHADER_TYPE_TESSELLATION_EVALUATION,

	KINC_G5_SHADER_TYPE_COUNT
} kinc_g5_shader_type_t;

typedef struct kinc_g5_shader {
	Shader5Impl impl;
} kinc_g5_shader_t;

/// <summary>
/// Initializes a shader based on system-specific shader-data. The system-specific shader-data is usually created per system by the krafix-shader-compiler which
/// is automatically called by kincmake.
/// </summary>
/// <param name="shader">The shader to initialize</param>
/// <param name="data">The system-specific shader-data</param>
/// <param name="length">The length of the system-specific shader-data in bytes</param>
/// <param name="type">The type of the shader</param>
void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type);

/// <summary>
/// Destroys a shader.
/// </summary>
/// <param name="shader">The shader to destroy</param>
void kinc_g5_shader_destroy(kinc_g5_shader_t *shader);

#ifdef __cplusplus
}
#endif
