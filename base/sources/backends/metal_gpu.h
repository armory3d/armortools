#pragma once

typedef struct {
	void *pipeline;
	void *depth;
} gpu_pipeline_impl_t;

typedef struct {
	char name[256];
	void *mtl_function;
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct {
	void *_tex;
	void *data;
} gpu_texture_impl_t;

typedef struct {
	void *metal_buffer;
} gpu_buffer_impl_t;

typedef struct {
	void *_raytracingPipeline;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	void *_accelerationStructure;
} gpu_raytrace_acceleration_structure_impl_t;
