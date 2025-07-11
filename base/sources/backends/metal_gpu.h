#pragma once

struct gpu_shader;

typedef struct {
	struct gpu_shader *vertex_shader;
	struct gpu_shader *fragment_shader;
	void *_pipeline;
	void *_depth;
} gpu_pipeline_impl_t;

typedef struct {
	char name[1024];
	void *mtl_function;
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct {
	void *_tex;
	void *data;
	void *_readback;
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
