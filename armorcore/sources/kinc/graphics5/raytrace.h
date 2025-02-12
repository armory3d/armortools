#pragma once

/*! \file raytrace.h
    \brief Preliminary API, requires some actual D3D12/Vulkan code to fill in
    the acceleration-structure and pipeline-details. Also requires manually
    compiled shaders. Use with caution.
*/

#include <kinc/global.h>
#include <kinc/backend/graphics5/raytrace.h>

struct kinc_g5_command_list;
struct kinc_g5_constant_buffer;
struct kinc_g5_index_buffer;
struct kinc_g5_render_target;
struct kinc_g5_texture;
struct kinc_g5_vertex_buffer;

typedef struct kinc_raytrace_pipeline {
	struct kinc_g5_constant_buffer *_constant_buffer;
	kinc_raytrace_pipeline_impl_t impl;
} kinc_raytrace_pipeline_t;

bool kinc_raytrace_supported(void);
void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, struct kinc_g5_command_list *command_list,
    void *ray_shader, int ray_shader_size, struct kinc_g5_constant_buffer *constant_buffer);
void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline);

typedef struct kinc_raytrace_acceleration_structure {
	kinc_raytrace_acceleration_structure_impl_t impl;
} kinc_raytrace_acceleration_structure_t;

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel);
void kinc_raytrace_acceleration_structure_add(kinc_raytrace_acceleration_structure_t *accel, struct kinc_g5_vertex_buffer *vb, struct kinc_g5_index_buffer *ib, kinc_matrix4x4_t transform);
void kinc_raytrace_acceleration_structure_build(kinc_raytrace_acceleration_structure_t *accel, struct kinc_g5_command_list *command_list,
    struct kinc_g5_vertex_buffer *_vb_full, struct kinc_g5_index_buffer *_ib_full);
void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel);

void kinc_raytrace_set_textures(struct kinc_g5_render_target *texpaint0, struct kinc_g5_render_target *texpaint1, struct kinc_g5_render_target *texpaint2,
    struct kinc_g5_texture *texenv, struct kinc_g5_texture *texsobol, struct kinc_g5_texture *texscramble, struct kinc_g5_texture *texrank);
void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *accel);
void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *pipeline);
void kinc_raytrace_set_target(struct kinc_g5_render_target *output);
void kinc_raytrace_dispatch_rays(struct kinc_g5_command_list *command_list);
