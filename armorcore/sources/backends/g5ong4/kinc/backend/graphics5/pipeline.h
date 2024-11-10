#pragma once

#include <kinc/graphics4/pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g4_pipeline_t pipe;
	// int vertex_location_count;
	// int fragment_location_count;
	// kinc_g4_constant_location_t *vertex_locations;
	// kinc_g4_constant_location_t *fragment_locations;
	// int *vertex_sizes;
	// int *fragment_sizes;
} PipelineState5Impl;

typedef struct {
	int a;
} ComputePipelineState5Impl;

typedef struct {
	kinc_g4_constant_location_t location;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;

#ifdef __cplusplus
}
#endif
