#pragma once

#include <kinc/global.h>

#include <kinc/image.h>

#include "rendertarget.h"
#include "shader.h"
#include "vertexstructure.h"

#include <kinc/backend/graphics5/graphics.h>

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file graphics.h
    \brief Contains the base G5-functionality.
*/

#ifdef __cplusplus
extern "C" {
#endif

extern bool kinc_g5_fullscreen;

/// <summary>
/// Returns whether raytracing (see kinc/graphics5/raytrace.h) is supported.
/// </summary>
/// <returns>Whether raytracing is supported</returns>
bool kinc_g5_supports_raytracing(void);

/// <summary>
/// Returns whether instanced rendering (kinc_g5_command_list_draw_indexed_vertices_instanced and pals) is supported.
/// </summary>
/// <returns>Whether instanced rendering is supported</returns>
bool kinc_g5_supports_instanced_rendering(void);

/// <summary>
/// Returns whether GPU-compute (the functions in kinc/compute/compute.h) is supported.
/// </summary>
/// <returns>Whether GPU-compute is supported</returns>
bool kinc_g5_supports_compute_shaders(void);

/// <summary>
/// Returns whether blend-constants (see kinc_g4_set_blend_constant and the blending-properties for pipelines) are supported.
/// </summary>
/// <returns>Whether blend-constants are supported</returns>
bool kinc_g5_supports_blend_constants(void);

/// <summary>
/// Returns whether textures are supported which have widths/heights which are not powers of two.
/// </summary>
/// <returns>Whether non power of two texture-sizes are supported</returns>
bool kinc_g5_supports_non_pow2_textures(void);

/// <summary>
/// Returns whether render-targets are upside down. This happens in OpenGL and there is currently no automatic mitigation.
/// </summary>
/// <returns>Whether render-targets are upside down</returns>
bool kinc_g5_render_targets_inverted_y(void);

/// <summary>
/// Returns how many textures can be used at the same time in a fragment-shader.
/// </summary>
/// <returns>The number of textures</returns>
int kinc_g5_max_bound_textures(void);

/// <summary>
/// I think this does nothing.
/// </summary>
void kinc_g5_flush(void);

/// <summary>
/// Returns the currently used number of samples for hardware-antialiasing.
/// </summary>
/// <returns>The number of samples</returns>
int kinc_g5_antialiasing_samples(void);

/// <summary>
/// Sets the number of samples used for hardware-antialiasing. This typically uses multisampling and typically only works with a few specific numbers of
/// sample-counts - 2 and 4 are pretty safe bets. It also might do nothing at all.
/// </summary>
/// <param name="samples">The number of samples</param>
void kinc_g5_set_antialiasing_samples(int samples);

/// <summary>
/// Needs to be called before rendering to a window. Typically called at the start of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window);

/// <summary>
/// Needs to be called after rendering to a window. Typically called at the end of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
/// <returns></returns>
void kinc_g5_end(int window);

/// <summary>
/// Needs to be called to make the rendered frame visible. Typically called at the very end of each frame.
/// </summary>
bool kinc_g5_swap_buffers(void);

void kinc_g5_internal_init(void);
void kinc_g5_internal_init_window(int window, int depth_buffer_bits, int stencil_buffer_bits, bool vsync);
void kinc_g5_internal_destroy_window(int window);
void kinc_g5_internal_destroy(void);

#ifdef __cplusplus
}
#endif
