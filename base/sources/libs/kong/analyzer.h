#ifndef KONG_ANALYZER_HEADER
#define KONG_ANALYZER_HEADER

#include "array.h"
#include "functions.h"
#include "globals.h"
#include "sets.h"

#include <stdint.h>

typedef struct render_pipeline {
	function *vertex_shader;
	function *amplification_shader;
	function *mesh_shader;
	function *fragment_shader;
} render_pipeline;

static_array(render_pipeline, render_pipelines, 256);

static_array(uint32_t, render_pipeline_indices, 256);

typedef render_pipeline_indices render_pipeline_group;

static_array(render_pipeline_group, render_pipeline_groups, 64);

static_array(function *, compute_shaders, 256);

static_array(uint32_t, compute_shader_indices, 256);

typedef struct raytracing_pipeline {
	function *gen_shader;
	function *miss_shader;
	function *closest_shader;
	function *intersection_shader;
	function *any_shader;
} raytracing_pipeline;

static_array(raytracing_pipeline, raytracing_pipelines, 256);

static_array(uint32_t, raytracing_pipeline_indices, 256);

typedef raytracing_pipeline_indices raytracing_pipeline_group;

static_array(raytracing_pipeline_group, raytracing_pipeline_groups, 64);

static_array(descriptor_set *, descriptor_sets, 256);

typedef descriptor_sets descriptor_set_group;

static_array(descriptor_set_group, descriptor_set_groups, 256);

void find_referenced_functions(function *f, function **functions, size_t *functions_size);
void find_referenced_types(function *f, type_id *types, size_t *types_size);
void find_referenced_globals(function *f, global_array *globals);

descriptor_set_group *get_descriptor_set_group(uint32_t descriptor_set_group_index);

descriptor_set_group *find_descriptor_set_group_for_pipe_type(type *t);
descriptor_set_group *find_descriptor_set_group_for_function(function *f);

void analyze(void);

#endif
