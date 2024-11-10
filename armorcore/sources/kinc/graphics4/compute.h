#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/compute.h>
#ifdef KINC_OPENGL
#include <kinc/graphics4/vertexbuffer.h>
#endif
#include <kinc/graphics4/graphics.h>

/*! \file compute.h
    \brief Provides support for running compute-shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_texture;
struct kinc_g4_render_target;

typedef struct kinc_g4_compute_shader {
	kinc_g4_compute_shader_impl impl;
} kinc_g4_compute_shader;

/// <summary>
/// Initialize a compute-shader from system-specific shader-data.
/// </summary>
/// <param name="shader">The shader-object to initialize</param>
/// <param name="source">A pointer to system-specific shader-data</param>
/// <param name="length">Length of the shader-data in bytes</param>
void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *source, int length);

/// <summary>
/// Destroy a shader-object
/// </summary>
/// <param name="shader">The shader-object to destroy</param>
void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader);

/// <summary>
/// Finds the location of a constant/uniform inside of a shader.
/// </summary>
/// <param name="shader">The shader to look into</param>
/// <param name="name">The constant/uniform-name to look for</param>
/// <returns>The found constant-location</returns>
kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name);

/// <summary>
/// Finds a texture-unit inside of a shader.
/// </summary>
/// <param name="shader">The shader to look into</param>
/// <param name="name">The texture-name to look for</param>
/// <returns>The found texture-unit</returns>
kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name);

#ifdef __cplusplus
}
#endif
