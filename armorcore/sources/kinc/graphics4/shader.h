#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/shader.h>

/*! \file shader.h
    \brief Provides functions for creating and destroying shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g4_shader_type {
	KINC_G4_SHADER_TYPE_FRAGMENT,
	KINC_G4_SHADER_TYPE_VERTEX,
	KINC_G4_SHADER_TYPE_COMPUTE,
	KINC_G4_SHADER_TYPE_GEOMETRY,
	KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL,
	KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION,

	KINC_G4_SHADER_TYPE_COUNT
} kinc_g4_shader_type_t;

typedef struct kinc_g4_shader {
	kinc_g4_shader_impl_t impl;
} kinc_g4_shader_t;

/// <summary>
/// Initializes a shader based on system-specific shader-data. The system-specific shader-data is usually created per system by the krafix-shader-compiler which
/// is automatically called by kincmake.
/// </summary>
/// <param name="shader">The shader to initialize</param>
/// <param name="data">The system-specific shader-data</param>
/// <param name="length">The length of the system-specific shader-data in bytes</param>
/// <param name="type">The type of the shader</param>
void kinc_g4_shader_init(kinc_g4_shader_t *shader, const void *data, size_t length, kinc_g4_shader_type_t type);

/// <summary>
/// Initializes a shader from GLSL-source-code. This only works on some platforms and only if KRAFIX_LIBRARY define has been set and the krafix-shader-compiler
/// was compiled in library-mode and linked into the application.
/// </summary>
/// <param name="shader">The shader to initialize</param>
/// <param name="source">The GLSL-shader-source-code</param>
/// <param name="type">The type of the shader</param>
/// <returns>The number of errors the compiler encountered - hopefully it's zero.</returns>
int kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type);

/// <summary>
/// Destroys a shader.
/// </summary>
/// <param name="shader">The shader to destroy</param>
void kinc_g4_shader_destroy(kinc_g4_shader_t *shader);

#ifdef __cplusplus
}
#endif
