#pragma once

#include <kinc/global.h>

#include <kinc/math/matrix.h>

#include "constantlocation.h"
#include "pipeline.h"
#include "textureunit.h"

/*! \file graphics.h
    \brief Contains the base G4-functionality.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_compute_shader;
struct kinc_g4_pipeline;
struct kinc_g4_render_target;
struct kinc_g4_texture;

typedef enum {
	KINC_G4_TEXTURE_ADDRESSING_REPEAT,
	KINC_G4_TEXTURE_ADDRESSING_MIRROR,
	KINC_G4_TEXTURE_ADDRESSING_CLAMP,
	KINC_G4_TEXTURE_ADDRESSING_BORDER
} kinc_g4_texture_addressing_t;

typedef enum {
	KINC_G4_TEXTURE_DIRECTION_U,
	KINC_G4_TEXTURE_DIRECTION_V,
	KINC_G4_TEXTURE_DIRECTION_W
} kinc_g4_texture_direction_t;

typedef enum {
	KINC_G4_TEXTURE_FILTER_POINT,
	KINC_G4_TEXTURE_FILTER_LINEAR,
	KINC_G4_TEXTURE_FILTER_ANISOTROPIC
} kinc_g4_texture_filter_t;

typedef enum {
	KINC_G4_MIPMAP_FILTER_NONE,
	KINC_G4_MIPMAP_FILTER_POINT,
	KINC_G4_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} kinc_g4_mipmap_filter_t;

/// <summary>
/// Returns whether instanced rendering (kinc_g4_draw_indexed_vertices_instanced and pals) is supported.
/// </summary>
/// <returns>Whether instanced rendering is supported</returns>
bool kinc_g4_supports_instanced_rendering(void);

/// <summary>
/// Returns whether GPU-compute (the functions in kinc/compute/compute.h) is supported.
/// </summary>
/// <returns>Whether GPU-compute is supported</returns>
bool kinc_g4_supports_compute_shaders(void);

/// <summary>
/// Returns whether blend-constants (see kinc_g4_set_blend_constant and the blending-properties for pipelines) are supported.
/// </summary>
/// <returns>Whether blend-constants are supported</returns>
bool kinc_g4_supports_blend_constants(void);

/// <summary>
/// Returns whether textures are supported which have widths/heights which are not powers of two.
/// </summary>
/// <returns>Whether non power of two texture-sizes are supported</returns>
bool kinc_g4_supports_non_pow2_textures(void);

/// <summary>
/// Returns whether render-targets are upside down. This happens in OpenGL and there is currently no automatic mitigation.
/// </summary>
/// <returns>Whether render-targets are upside down</returns>
bool kinc_g4_render_targets_inverted_y(void);

/// <summary>
/// Returns how many textures can be used at the same time in a fragment-shader.
/// </summary>
/// <returns>The number of textures</returns>
int kinc_g4_max_bound_textures(void);

/// <summary>
/// Kicks off lingering work - may or may not actually do anything depending on the underlying graphics-API.
/// </summary>
void kinc_g4_flush(void);

/// <summary>
/// Needs to be called before rendering to a window. Typically called at the start of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
void kinc_g4_begin(int window);

/// <summary>
/// Needs to be called after rendering to a window. Typically called at the end of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
/// <returns></returns>
void kinc_g4_end(int window);

/// <summary>
/// Needs to be called to make the rendered frame visible. Typically called at the very end of each frame.
/// </summary>
bool kinc_g4_swap_buffers(void);

#define KINC_G4_CLEAR_COLOR 1
#define KINC_G4_CLEAR_DEPTH 2
#define KINC_G4_CLEAR_STENCIL 4

/// <summary>
/// Clears the color, depth and/or stencil-components of the current framebuffer or render-target.
/// </summary>
/// <param name="flags">Defines what components to clear</param>
/// <param name="color">The color-value to clear to in 0xAARRGGBB</param>
/// <param name="depth">The depth-value to clear to</param>
/// <param name="stencil">The stencil-value to clear to</param>
void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil);

/// <summary>
/// Sets the viewport which defines the portion of the framebuffer or render-target things are rendered into. By default the viewport is equivalent to the full
/// size of the current render-target or framebuffer.
/// </summary>
/// <param name="x">The x-offset of the viewport from the left of the screen in pixels</param>
/// <param name="y">The y-offset of the viewport from the top of the screen in pixels</param>
/// <param name="width">The width of the viewport in pixels</param>
/// <param name="height">The height of the viewport in pixels</param>
void kinc_g4_viewport(int x, int y, int width, int height);

/// <summary>
/// Enables and defines the scissor-rect. When the scissor-rect is enabled, anything that's rendered outside of the scissor-rect will be ignored.
/// </summary>
/// <param name="x">The x-offset of the scissor-rect from the left of the screen in pixels</param>
/// <param name="y">The y-offset of the scissor-rect from the top of the screen in pixels</param>
/// <param name="width">The width of the scissor-rect in pixels</param>
/// <param name="height">The height of the scissor-rect in pixels</param>
void kinc_g4_scissor(int x, int y, int width, int height);

/// <summary>
/// Disables the scissor-rect.
/// </summary>
void kinc_g4_disable_scissor(void);

/// <summary>
/// Draws the entire content of the currently set index-buffer and vertex-buffer. G4 can only draw triangle-lists using vertex-indices as this is what GPUs tend
/// to be optimized for.
/// </summary>
void kinc_g4_draw_indexed_vertices(void);

/// <summary>
/// Draws a part of the content of the currently set index-buffer and vertex-buffer. G4 can only draw triangle-lists using vertex-indices as this is what GPUs
/// tend to be optimized for.
/// </summary>
/// <param name="start">The offset into the index-buffer</param>
/// <param name="count">The number of indices to use</param>
void kinc_g4_draw_indexed_vertices_from_to(int start, int count);

/// <summary>
/// Draws a part of the content of the currently set index-buffer and vertex-buffer and additionally applies a general offset into the vertex-buffer. G4 can
/// only draw triangle-lists using vertex-indices as this is what GPUs tend to be optimized for.
/// </summary>
/// <param name="start">The offset into the index-buffer</param>
/// <param name="count">The number of indices to use</param>
/// <param name="vertex_offset">The offset into the vertex-buffer which is added to each index read from the index-buffer</param>
void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_offset);

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount);

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count);

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing);

/// <summary>
/// Sets the pipeline for the next draw-call. The pipeline defines most rendering-state including the shaders to be used.
/// </summary>
/// <param name="pipeline">The pipeline to set</param>
void kinc_g4_set_pipeline(struct kinc_g4_pipeline *pipeline);

void kinc_g4_set_stencil_reference_value(int value);

/// <summary>
/// Sets the blend constant used for `KINC_G4_BLEND_CONSTANT` or `KINC_G4_INV_BLEND_CONSTANT`
/// </summary>
void kinc_g4_set_blend_constant(float r, float g, float b, float a);


/// <summary>
/// Assigns an integer to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the value to</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g4_set_int(kinc_g4_constant_location_t location, int value);

/// <summary>
/// Assigns two integers to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first component of the constant/uniform</param>
/// <param name="value2">The value to assign to the second component of the constant/uniform</param>
void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2);

/// <summary>
/// Assigns three integers to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first component of the constant/uniform</param>
/// <param name="value2">The value to assign to the second component of the constant/uniform</param>
/// <param name="value3">The value to assign to the third component of the constant/uniform</param>
void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3);

/// <summary>
/// Assigns four integers to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first component of the constant/uniform</param>
/// <param name="value2">The value to assign to the second component of the constant/uniform</param>
/// <param name="value3">The value to assign to the third component of the constant/uniform</param>
/// <param name="value4">The value to assign to the fourth component of the constant/uniform</param>
void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4);

/// <summary>
/// Assigns a bunch of integers to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value">The values to assign to the constant/uniform</param>
/// <param name="value">The number of values to assign to the constant/uniform</param>
void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count);

/// <summary>
/// Assigns a float to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the value to</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g4_set_float(kinc_g4_constant_location_t location, float value);

/// <summary>
/// Assigns two floats to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first constant/uniform</param>
/// <param name="value2">The value to assign to the second constant/uniform</param>
void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2);

/// <summary>
/// Assigns three floats to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first constant/uniform</param>
/// <param name="value2">The value to assign to the second constant/uniform</param>
/// <param name="value3">The value to assign to the third constant/uniform</param>
void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3);

/// <summary>
/// Assigns four floats to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value1">The value to assign to the first constant/uniform</param>
/// <param name="value2">The value to assign to the second constant/uniform</param>
/// <param name="value3">The value to assign to the third constant/uniform</param>
/// <param name="value4">The value to assign to the fourth constant/uniform</param>
void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4);

/// <summary>
/// Assigns a bunch of floats to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value">The values to assign to the constant/uniform</param>
/// <param name="value">The number of values to assign to the constant/uniform</param>
void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count);

/// <summary>
/// Assigns a bool to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the value to</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value);

/// <summary>
/// Assigns a 3x3-matrix to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the value to</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value);

/// <summary>
/// Assigns a 4x4-matrix to a constant/uniform in the currently set pipeline.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the value to</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value);

/// <summary>
/// Set the texture-sampling-mode for upscaled textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Set the texture-sampling-mode for upscaled 3D-textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Set the texture-sampling-mode for downscaled textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Set the texture-sampling-mode for downscaled 3D-textures.
/// </summary>
/// <param name="unit">The texture-unit to set the texture-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter);

/// <summary>
/// Sets the mipmap-sampling-mode which defines whether mipmaps are used at all and if so whether the two neighbouring mipmaps are linearly interpolated.
/// </summary>
/// <param name="unit">The texture-unit to set the mipmap-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, kinc_g4_mipmap_filter_t filter);

/// <summary>
/// Sets the mipmap-sampling-mode for a 3D-texture which defines whether mipmaps are used at all and if so whether the two neighbouring mipmaps are linearly
/// interpolated.
/// </summary>
/// <param name="unit">The texture-unit to set the mipmap-sampling-mode for</param>
/// <param name="filter">The mode to set</param>
void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter);

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

void kinc_g4_set_texture_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode);

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

void kinc_g4_set_cubemap_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode);

void kinc_g4_set_texture_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy);

void kinc_g4_set_cubemap_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy);

void kinc_g4_set_texture_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp);

void kinc_g4_set_cubemap_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp);

/// <summary>
/// Sets the framebuffer (aka the actual contents of the current window) to be the target of any future draw-calls.
/// </summary>
void kinc_g4_restore_render_target(void);

/// <summary>
/// Sets the passed render-targets to be the target of any future draw-calls.
/// </summary>
/// <param name="targets">An array of render-targets</param>
/// <param name="count">The number of render-targets in the render-target-array</param>
void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count);

void kinc_g4_set_render_target_face(struct kinc_g4_render_target *texture, int face);

/// <summary>
/// Assigns a texture to a texture-unit for sampled access via GLSL's texture.
/// </summary>
/// <param name="unit">The unit to assign this texture to</param>
/// <param name="texture">The texture to assign to the unit</param>
void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

/// <summary>
/// Assigns a texture to a texture-unit for direct access via GLSL's texelFetch (as
/// opposed to GLSL's texture). The name of this functions is unfortunately based
/// on OpenGL's confusing terminology.
/// </summary>
/// <param name="unit">The unit to assign this texture to</param>
/// <param name="texture">The texture to assign to the unit</param>
void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

/// <summary>
/// Returns the currently used number of samples for hardware-antialiasing.
/// </summary>
/// <returns>The number of samples</returns>
int kinc_g4_antialiasing_samples(void);

/// <summary>
/// Sets the number of samples used for hardware-antialiasing. This typically uses multisampling and typically only works with a few specific numbers of
/// sample-counts - 2 and 4 are pretty safe bets. It also might do nothing at all.
/// </summary>
/// <param name="samples">The number of samples</param>
void kinc_g4_set_antialiasing_samples(int samples);

/// <summary>
/// Sets a shader for the next compute-run.
/// </summary>
/// <param name="shader">The shader to use</param>
void kinc_g4_set_compute_shader(struct kinc_g4_compute_shader *shader);

/// <summary>
/// Fire off a compute-run on x * y * z elements.
/// </summary>
/// <param name="x">The x-size for the compute-run</param>
/// <param name="y">The y-size for the compute-run</param>
/// <param name="z">The z-size for the compute-run</param>
void kinc_g4_compute(int x, int y, int z);

void kinc_g4_internal_init(void);
void kinc_g4_internal_init_window(int window, int depth_buffer_bits, int stencil_buffer_bits, bool vsync);
void kinc_g4_internal_destroy_window(int window);
void kinc_g4_internal_destroy(void);

#ifdef __cplusplus
}
#endif
