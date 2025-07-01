#pragma once

struct gpu_shader;

typedef struct {
	struct gpu_shader *vertex_shader;
	struct gpu_shader *fragment_shader;
	void *_pipeline;
	void *_pipelineDepth;
	void *_reflection;
	void *_depthStencil;
	void *_depthStencilNone;
} gpu_pipeline_impl_t;

typedef struct {
	char name[1024];
	void *mtlFunction;
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct {
	void *_tex;
	void *data;
	bool has_mipmaps;
	void *_texReadback;
} gpu_texture_impl_t;

typedef struct {
	int myStride;
	int count;
	void *metal_buffer;
} gpu_buffer_impl_t;

typedef struct {
	void *_raytracingPipeline;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	void *_accelerationStructure;
} gpu_raytrace_acceleration_structure_impl_t;
